// Copyright 2010 Google Inc. All Rights Reserved


#include "talk/base/win32windowpicker.h"

#include <string>

#include "talk/base/common.h"
#include "talk/base/logging.h"

namespace talk_base {

// Window class names that we want to filter out.
static const std::string kProgramManagerClass = "Progman";
static const std::string kButtonClass = "Button";

BOOL CALLBACK Win32WindowPicker::EnumProc(HWND hwnd, LPARAM l_param) {
  WindowDescriptionList* descriptions =
      reinterpret_cast<WindowDescriptionList*>(l_param);

  // Skip windows that are invisible, minimized, have no title, or are owned,
  // unless they have the app window style set. Except for minimized windows,
  // this is what Alt-Tab does.
  // TODO: Figure out how to grab a thumbnail of a minimized window and
  // include them in the list.
  int len = GetWindowTextLength(hwnd);
  HWND owner = GetWindow(hwnd, GW_OWNER);
  LONG exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
  if (len == 0 || IsIconic(hwnd) || !IsWindowVisible(hwnd) ||
      (owner && !(exstyle & WS_EX_APPWINDOW))) {
    // TODO: Investigate if windows without title still could be
    // interesting to share. We could use the name of the process as title:
    //
    // GetWindowThreadProcessId()
    // OpenProcess()
    // QueryFullProcessImageName()
    return TRUE;
  }

  // Skip the Program Manager window and the Start button.
  TCHAR class_name_w[500];
  ::GetClassName(hwnd, class_name_w, 500);
  std::string class_name = ToUtf8(class_name_w, wcslen(class_name_w));
  if (class_name == kProgramManagerClass || class_name == kButtonClass) {
    // We don't want the Program Manager window nor the Start button.
    return TRUE;
  }

  TCHAR window_title[500];
  GetWindowText(hwnd, window_title, ARRAY_SIZE(window_title));
  std::string title = ToUtf8(window_title, wcslen(window_title));
  WindowDescription desc(reinterpret_cast<int>(hwnd), title);
  descriptions->push_back(desc);
  return TRUE;
}

Win32WindowPicker::Win32WindowPicker() {
}

bool Win32WindowPicker::Init() {
  return true;
}

bool Win32WindowPicker::GetWindowList(WindowDescriptionList* descriptions) {
  return EnumWindows(Win32WindowPicker::EnumProc,
                     (LPARAM)descriptions) == TRUE ? true : false;
}

bool Win32WindowPicker::IsVisible(WindowId id) {
  HWND hwnd = reinterpret_cast<HWND>(id);
  return (::IsWindow(hwnd) != FALSE && ::IsWindowVisible(hwnd) != FALSE);
}

bool Win32WindowPicker::MoveToFront(WindowId id) {
  return SetForegroundWindow(reinterpret_cast<HWND>(id)) != FALSE;
}

}  // namespace talk_base
