#ifndef TRAY_INDICATOR_HPP
#define TRAY_INDICATOR_HPP

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

typedef struct _GDBusConnection GDBusConnection;
typedef struct _GMainContext GMainContext;
typedef struct _GMainLoop GMainLoop;
typedef struct _GDBusNodeInfo GDBusNodeInfo;

// Publishes Spark as a StatusNotifierItem (the freedesktop tray icon
// protocol) directly over D-Bus, with no GTK/appindicator dependency. Also
// publishes a minimal com.canonical.dbusmenu object so the tray host can
// show a dropdown with a single "Quit" entry.
class TrayIndicator {
   public:
    TrayIndicator();
    ~TrayIndicator();
    TrayIndicator(const TrayIndicator&) = delete;
    TrayIndicator& operator=(const TrayIndicator&) = delete;

    void SetLabel(const std::string& label);
    std::string GetLabel();

    // Invoked (on the tray's internal D-Bus thread) when the user clicks
    // "Quit" in the tray dropdown.
    void SetQuitCallback(std::function<void()> callback);

    // Called by the D-Bus menu handler when "Quit" is clicked. Not intended
    // to be called directly.
    void HandleQuitClicked();

   private:
    void Run();
    void RegisterWithWatcher();
    void EmitLabelChanged();

    std::thread thread_;
    std::atomic<bool> ready_{false};

    GMainContext* context_ = nullptr;
    GMainLoop* loop_ = nullptr;
    GDBusConnection* connection_ = nullptr;
    GDBusNodeInfo* introspection_data_ = nullptr;
    GDBusNodeInfo* menu_introspection_data_ = nullptr;
    unsigned int registration_id_ = 0;
    unsigned int menu_registration_id_ = 0;

    std::mutex label_mutex_;
    std::string label_;

    std::mutex quit_callback_mutex_;
    std::function<void()> quit_callback_;
};

#endif  // TRAY_INDICATOR_HPP
