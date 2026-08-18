#ifndef TRAY_INDICATOR_HPP
#define TRAY_INDICATOR_HPP

#include <atomic>
#include <mutex>
#include <string>
#include <thread>

typedef struct _GDBusConnection GDBusConnection;
typedef struct _GMainContext GMainContext;
typedef struct _GMainLoop GMainLoop;
typedef struct _GDBusNodeInfo GDBusNodeInfo;

// Publishes Spark as a StatusNotifierItem (the freedesktop tray icon
// protocol) directly over D-Bus, with no GTK/appindicator dependency.
class TrayIndicator {
   public:
    TrayIndicator();
    ~TrayIndicator();
    TrayIndicator(const TrayIndicator&) = delete;
    TrayIndicator& operator=(const TrayIndicator&) = delete;

    void SetLabel(const std::string& label);
    std::string GetLabel();

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
    unsigned int registration_id_ = 0;

    std::mutex label_mutex_;
    std::string label_;
};

#endif  // TRAY_INDICATOR_HPP
