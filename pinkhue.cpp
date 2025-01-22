#include "pinkhue.h"
#include <iostream>
#include <windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void ApplyHueOverlay(HINSTANCE hInstance) {
  RECT totalArea = GetTotalScreenArea();
  // Register window class for the overlay
  const LPCSTR CLASS_NAME = "PinkHueOverlay";

  WNDCLASS wc = {};
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;

  if (!RegisterClass(&wc)) {
    std::cerr << "Failed to register window class!" << std::endl;
    return;
  }

  // Create a layered window for the overlay, covering the entire screen area
  HWND hwnd = CreateWindowEx(
      WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT,
      CLASS_NAME, "", WS_POPUP, totalArea.left, totalArea.top,
      totalArea.right - totalArea.left, totalArea.bottom - totalArea.top, NULL,
      NULL, hInstance, NULL);

  if (!hwnd) {
    std::cerr << "Failed to create overlay window!" << std::endl;
    return;
  }

  // Set up a pink color overlay
  HDC hdcScreen = GetDC(NULL);
  HDC hdcMem = CreateCompatibleDC(hdcScreen);
  HBITMAP hbm =
      CreateCompatibleBitmap(hdcScreen, totalArea.right - totalArea.left,
                             totalArea.bottom - totalArea.top);
  SelectObject(hdcMem, hbm);

  HBRUSH hBrush = CreateSolidBrush(RGB(255, 182, 193)); // Pink color
  RECT rect = {0, 0, totalArea.right - totalArea.left,
               totalArea.bottom - totalArea.top};
  FillRect(hdcMem, &rect, hBrush);

  POINT ptZero = {0, 0};
  SIZE size = {totalArea.right - totalArea.left,
               totalArea.bottom - totalArea.top};
  BLENDFUNCTION blend = {AC_SRC_OVER, 0, 128, 0}; // 128 = 50% transparency

  UpdateLayeredWindow(hwnd, hdcScreen, NULL, &size, hdcMem, &ptZero,
                      RGB(0, 0, 0), &blend, ULW_ALPHA);

  DeleteObject(hBrush);
  DeleteDC(hdcMem);
  ReleaseDC(NULL, hdcScreen);

  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);
}

// Window procedure for handling messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                            LPARAM lParam) {
  switch (uMsg) {
  case WM_PAINT:
    ValidateRect(hwnd, NULL);
    return 0;

  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;

  default:
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
}