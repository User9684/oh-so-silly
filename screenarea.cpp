#include <random>
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

int randomBetween(int min, int max) {
  std::random_device rdev;
  std::mt19937 rgen(rdev());
  std::uniform_int_distribution<int> idist(min, max);

  return idist(rgen);
}

POINT GetRandomPositionOnScreen() {
  RECT screenArea = GetTotalScreenArea();

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> distX(screenArea.left, screenArea.right);
  std::uniform_int_distribution<int> distY(screenArea.top, screenArea.bottom);

  POINT randomPoint;
  randomPoint.x = distX(gen);
  randomPoint.y = distY(gen);

  return randomPoint;
}