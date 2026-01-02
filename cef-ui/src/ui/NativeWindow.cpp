#include "../../inc/ui/NativeWindow.h"
#include <stdexcept>
#include <sstream>
#include <map>

namespace cef_ui {
namespace ui {

    bool NativeWindow::window_created_ = false;

namespace {
  // Static window class name
  const wchar_t* WINDOW_CLASS_NAME = L"CefUIWindowClass";

  // Helper to convert std::string to std::wstring
  std::wstring StringToWide(const std::string& str) {
    if (str.empty()) {
      return std::wstring();
    }
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size_needed);
    return wstr;
  }

  // Global map to retrieve NativeWindow instance from HWND
  // Windows passes HWND to WindowProc, but we need access to the instance
  static std::map<HWND, NativeWindow*> g_window_map;

  // Register window class (one-time operation)
  static bool g_class_registered = false;

  void RegisterWindowClass() {
    if (g_class_registered) {
      return;
    }

    WNDCLASSW wc = {};
    wc.lpfnWndProc = NativeWindow::WindowProc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassW(&wc)) {
      throw std::runtime_error("Failed to register window class");
    }

    g_class_registered = true;
  }
}

NativeWindow::NativeWindow(const std::string& title)
    : hwnd_(nullptr),
      title_(title) {

    if (window_created_) {
        throw std::runtime_error("Only one NativeWindow instance is allowed per process");
    }

  try {
    InitializeWindow();
    window_created_ = true;
  } catch (...) {
    DestroyWindowHandle();
    throw;
  }
}

NativeWindow::~NativeWindow() noexcept {
  DestroyWindowHandle();
}

HWND NativeWindow::GetHandle() const {
  return hwnd_;
}

void NativeWindow::InitializeWindow() {
  // Register window class if not already done
  RegisterWindowClass();

  // Convert title to wide string
  std::wstring wide_title = StringToWide(title_);

  // Create window in hidden state
  hwnd_ = ::CreateWindowExW(
      0,                          // Extended style
      WINDOW_CLASS_NAME,          // Class name
      wide_title.c_str(),         // Window title
      WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,        // Style (standard window)
      CW_USEDEFAULT,              // X position
      CW_USEDEFAULT,              // Y position
      800,                         // Width
      600,                         // Height
      NULL,                        // Parent window
      NULL,                        // Menu
      GetModuleHandleW(NULL),      // Instance
      this                         // User data
  );

  if (!hwnd_) {
    std::ostringstream oss;
    oss << "Failed to create window: " << GetLastError();
    throw std::runtime_error(oss.str());
  }

  // Initially hide the window
  ShowWindow(hwnd_, SW_HIDE);
  UpdateWindow(hwnd_);
}

void NativeWindow::DestroyWindowHandle() noexcept {
  if (hwnd_) {
    // Remove from global map
    g_window_map.erase(hwnd_);
    ::DestroyWindow(hwnd_);
    hwnd_ = nullptr;
    window_created_ = false;

    // Destroy window
    if (!::DestroyWindow(hwnd_)) {
      // Log error but don't throw (noexcept)
    }

    hwnd_ = nullptr;
  }
}

LRESULT CALLBACK NativeWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  // Find the NativeWindow instance from the global map
  auto it = g_window_map.find(hwnd);
  if (it != g_window_map.end()) {
    return it->second->OnMessage(msg, wparam, lparam);
  }
  if (msg == WM_NCCREATE) {
      CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lparam);
      NativeWindow* window = static_cast<NativeWindow*>(cs->lpCreateParams);
      g_window_map[hwnd] = window;
  }

  // Fallback to default handling
  return DefWindowProcW(hwnd, msg, wparam, lparam);
}

LRESULT NativeWindow::OnMessage(UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
    case WM_CLOSE:
      // Handle close gracefully - allow default processing
      // This will trigger WM_DESTROY and clean application exit
      return DefWindowProcW(hwnd_, msg, wparam, lparam);

    case WM_DESTROY:
      // Window is being destroyed - post quit message to exit message loop
      return 0;

    default:
      // Pass to default handler
      return DefWindowProcW(hwnd_, msg, wparam, lparam);
  }
}

}  // namespace ui
}  // namespace cef_ui
