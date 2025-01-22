#include <windows.h>

RECT GetTotalScreenArea() {
  RECT totalArea = {0, 0, 0, 0};

  // EnumDisplayMonitors iterates over all connected monitors
  EnumDisplayMonitors(
      NULL, NULL,
      [](HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor,
         LPARAM dwData) -> BOOL {
        RECT *totalArea = reinterpret_cast<RECT *>(dwData);
        totalArea->left = min(totalArea->left, lprcMonitor->left);
        totalArea->top = min(totalArea->top, lprcMonitor->top);
        totalArea->right = max(totalArea->right, lprcMonitor->right);
        totalArea->bottom = max(totalArea->bottom, lprcMonitor->bottom);
        return TRUE;
      },
      reinterpret_cast<LPARAM>(&totalArea));

  return totalArea;
}