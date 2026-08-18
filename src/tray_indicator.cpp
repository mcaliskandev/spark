#include "tray_indicator.hpp"

#include <gio/gio.h>

#include <chrono>
#include <cstdio>

namespace {

constexpr const char* kObjectPath = "/StatusNotifierItem";
constexpr const char* kInterfaceName = "org.kde.StatusNotifierItem";
constexpr const char* kWatcherName = "org.kde.StatusNotifierWatcher";
constexpr const char* kWatcherPath = "/StatusNotifierWatcher";

// The XAyatanaLabel/XAyatanaLabelGuide properties and the XAyatanaNewLabel
// signal are a de-facto extension (originating from Ubuntu's indicator
// stack) to the KDE StatusNotifierItem spec that GNOME's AppIndicator
// extension also honors, used to show text next to (or instead of) an icon.
constexpr const char* kIntrospectionXml = R"xml(
<node>
  <interface name='org.kde.StatusNotifierItem'>
    <property name='Category' type='s' access='read'/>
    <property name='Id' type='s' access='read'/>
    <property name='Title' type='s' access='read'/>
    <property name='Status' type='s' access='read'/>
    <property name='WindowId' type='i' access='read'/>
    <property name='IconName' type='s' access='read'/>
    <property name='IconPixmap' type='a(iiay)' access='read'/>
    <property name='OverlayIconName' type='s' access='read'/>
    <property name='OverlayIconPixmap' type='a(iiay)' access='read'/>
    <property name='AttentionIconName' type='s' access='read'/>
    <property name='AttentionIconPixmap' type='a(iiay)' access='read'/>
    <property name='AttentionMovieName' type='s' access='read'/>
    <property name='ToolTip' type='(sa(iiay)ss)' access='read'/>
    <property name='ItemIsMenu' type='b' access='read'/>
    <property name='Menu' type='o' access='read'/>
    <property name='XAyatanaLabel' type='s' access='read'/>
    <property name='XAyatanaLabelGuide' type='s' access='read'/>
    <method name='ContextMenu'>
      <arg type='i' direction='in'/>
      <arg type='i' direction='in'/>
    </method>
    <method name='Activate'>
      <arg type='i' direction='in'/>
      <arg type='i' direction='in'/>
    </method>
    <method name='SecondaryActivate'>
      <arg type='i' direction='in'/>
      <arg type='i' direction='in'/>
    </method>
    <method name='Scroll'>
      <arg type='i' direction='in'/>
      <arg type='s' direction='in'/>
    </method>
    <signal name='NewTitle'/>
    <signal name='NewIcon'/>
    <signal name='NewAttentionIcon'/>
    <signal name='NewOverlayIcon'/>
    <signal name='NewMenu'/>
    <signal name='NewToolTip'/>
    <signal name='NewStatus'>
      <arg type='s'/>
    </signal>
    <signal name='XAyatanaNewLabel'>
      <arg type='s'/>
      <arg type='s'/>
    </signal>
  </interface>
</node>
)xml";

GVariant* EmptyPixmapArray() {
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("a(iiay)"));
    return g_variant_builder_end(&builder);
}

void HandleMethodCall(GDBusConnection* /*connection*/, const gchar* /*sender*/,
                      const gchar* /*object_path*/,
                      const gchar* /*interface_name*/,
                      const gchar* /*method_name*/, GVariant* /*parameters*/,
                      GDBusMethodInvocation* invocation,
                      gpointer /*user_data*/) {
    // No context menu, activation or scroll behavior; just acknowledge.
    g_dbus_method_invocation_return_value(invocation, nullptr);
}

constexpr const char* kLabelGuide = "CPU : %100   RAM : %100";

GVariant* HandleGetProperty(GDBusConnection* /*connection*/,
                            const gchar* /*sender*/,
                            const gchar* /*object_path*/,
                            const gchar* /*interface_name*/,
                            const gchar* property_name, GError** error,
                            gpointer user_data) {
    auto* self = static_cast<TrayIndicator*>(user_data);
    const std::string name(property_name);

    if (name == "Category") return g_variant_new_string("ApplicationStatus");
    if (name == "Id") return g_variant_new_string("spark-system-monitor");
    if (name == "Title") return g_variant_new_string("Spark");
    if (name == "Status") return g_variant_new_string("Active");
    if (name == "WindowId") return g_variant_new_int32(0);
    if (name == "IconName") return g_variant_new_string("");
    if (name == "IconPixmap") return EmptyPixmapArray();
    if (name == "OverlayIconName") return g_variant_new_string("");
    if (name == "OverlayIconPixmap") return EmptyPixmapArray();
    if (name == "AttentionIconName") return g_variant_new_string("");
    if (name == "AttentionIconPixmap") return EmptyPixmapArray();
    if (name == "AttentionMovieName") return g_variant_new_string("");
    if (name == "ToolTip") {
        GVariantBuilder tooltip;
        g_variant_builder_init(&tooltip, G_VARIANT_TYPE("(sa(iiay)ss)"));
        g_variant_builder_add(&tooltip, "s", "");
        g_variant_builder_add_value(&tooltip, EmptyPixmapArray());
        g_variant_builder_add(&tooltip, "s", "Spark");
        g_variant_builder_add(&tooltip, "s", "");
        return g_variant_builder_end(&tooltip);
    }
    if (name == "ItemIsMenu") return g_variant_new_boolean(FALSE);
    // No menu is offered; the property still must exist and resolve to a
    // valid object path or some hosts fail to set up the item at all.
    if (name == "Menu") return g_variant_new_object_path("/");
    if (name == "XAyatanaLabel") {
        return g_variant_new_string(self->GetLabel().c_str());
    }
    if (name == "XAyatanaLabelGuide") return g_variant_new_string(kLabelGuide);

    g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_PROPERTY,
                "No such property \"%s\"", property_name);
    return nullptr;
}

const GDBusInterfaceVTable kInterfaceVTable = {
    HandleMethodCall, HandleGetProperty, nullptr, {nullptr}};

}  // namespace

TrayIndicator::TrayIndicator() {
    thread_ = std::thread(&TrayIndicator::Run, this);
    while (!ready_.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}

TrayIndicator::~TrayIndicator() {
    if (thread_.joinable()) {
        if (loop_ != nullptr) {
            g_main_loop_quit(loop_);
        }
        thread_.join();
    }
    if (registration_id_ != 0 && connection_ != nullptr) {
        g_dbus_connection_unregister_object(connection_, registration_id_);
    }
    if (introspection_data_ != nullptr) {
        g_dbus_node_info_unref(introspection_data_);
    }
    if (connection_ != nullptr) {
        g_object_unref(connection_);
    }
    if (loop_ != nullptr) {
        g_main_loop_unref(loop_);
    }
    if (context_ != nullptr) {
        g_main_context_unref(context_);
    }
}

void TrayIndicator::Run() {
    context_ = g_main_context_new();
    g_main_context_push_thread_default(context_);

    GError* error = nullptr;
    connection_ = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
    if (connection_ == nullptr) {
        std::fprintf(stderr,
                     "TrayIndicator: failed to connect to session bus: %s\n",
                     error != nullptr ? error->message : "unknown error");
        if (error != nullptr) {
            g_error_free(error);
        }
        g_main_context_pop_thread_default(context_);
        ready_.store(true, std::memory_order_release);
        return;
    }

    introspection_data_ =
        g_dbus_node_info_new_for_xml(kIntrospectionXml, &error);
    if (introspection_data_ == nullptr) {
        std::fprintf(stderr, "TrayIndicator: bad introspection XML: %s\n",
                     error != nullptr ? error->message : "unknown error");
        if (error != nullptr) {
            g_error_free(error);
        }
        g_main_context_pop_thread_default(context_);
        ready_.store(true, std::memory_order_release);
        return;
    }

    registration_id_ = g_dbus_connection_register_object(
        connection_, kObjectPath, introspection_data_->interfaces[0],
        &kInterfaceVTable, this, nullptr, &error);
    if (registration_id_ == 0) {
        std::fprintf(stderr, "TrayIndicator: failed to register object: %s\n",
                     error != nullptr ? error->message : "unknown error");
        if (error != nullptr) {
            g_error_free(error);
        }
    } else {
        RegisterWithWatcher();
    }

    loop_ = g_main_loop_new(context_, FALSE);
    ready_.store(true, std::memory_order_release);
    g_main_loop_run(loop_);

    g_main_context_pop_thread_default(context_);
}

void TrayIndicator::RegisterWithWatcher() {
    GError* error = nullptr;
    GVariant* result = g_dbus_connection_call_sync(
        connection_, kWatcherName, kWatcherPath, kWatcherName,
        "RegisterStatusNotifierItem",
        g_variant_new("(s)", g_dbus_connection_get_unique_name(connection_)),
        nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
    if (result != nullptr) {
        g_variant_unref(result);
        std::fprintf(stderr,
                     "TrayIndicator: registered with StatusNotifierWatcher\n");
    } else {
        std::fprintf(stderr,
                     "TrayIndicator: RegisterStatusNotifierItem failed: %s\n",
                     error != nullptr ? error->message : "unknown error");
        if (error != nullptr) {
            g_error_free(error);
        }
    }
}

std::string TrayIndicator::GetLabel() {
    std::lock_guard<std::mutex> lock(label_mutex_);
    return label_;
}

void TrayIndicator::EmitLabelChanged() {
    if (connection_ == nullptr) {
        return;
    }
    GError* error = nullptr;
    g_dbus_connection_emit_signal(
        connection_, nullptr, kObjectPath, kInterfaceName, "XAyatanaNewLabel",
        g_variant_new("(ss)", GetLabel().c_str(), kLabelGuide), &error);
    if (error != nullptr) {
        g_error_free(error);
    }
}

void TrayIndicator::SetLabel(const std::string& label) {
    {
        std::lock_guard<std::mutex> lock(label_mutex_);
        label_ = label;
    }
    if (context_ == nullptr) {
        return;
    }
    // Marshal the signal emission onto the thread running our GMainContext.
    GSource* source = g_idle_source_new();
    g_source_set_callback(
        source,
        [](gpointer user_data) -> gboolean {
            static_cast<TrayIndicator*>(user_data)->EmitLabelChanged();
            return G_SOURCE_REMOVE;
        },
        this, nullptr);
    g_source_attach(source, context_);
    g_source_unref(source);
}
