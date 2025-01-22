#include "cats.h"
#include <gdiplus.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <windows.h>


#pragma comment(lib, "gdiplus.lib")

const std::map<std::string, int> catRatios = {
    {"CAT1", 2},
};

const std::vector<std::string> cats = {
    "CAT1",
};

void SpawnCat(HINSTANCE hInstance) {
  const LPCSTR CLASS_NAME = "CatWindow";
  const std::string catName = cats[rand() % cats.size()];
  const std::wstring resourceName(catName.begin(), catName.end());
  HRSRC hRes = nullptr;

  if (FindResourceByNameAcrossAllTypes(resourceName, hRes) != 0 || !hRes) {
    std::cerr << "Failed to find cat resource: " << catName << std::endl;
    return;
  }

  // Load the resource
  HGLOBAL hGlobal = LoadResource(NULL, hRes);
  DWORD imageSize = SizeofResource(NULL, hRes);
  if (!hGlobal || imageSize == 0) {
    std::cerr << "Failed to load resource: " << catName << std::endl;
    return;
  }

  void *pImageData = LockResource(hGlobal);
  if (!pImageData) {
    std::cerr << "Failed to lock resource: " << catName << std::endl;
    return;
  }

  // Initialize GDI+
  Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

  // Create an IStream from the resource data
  HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, imageSize);
  memcpy(GlobalLock(hMem), pImageData, imageSize);
  GlobalUnlock(hMem);

  IStream *pStream = NULL;
  if (CreateStreamOnHGlobal(hMem, TRUE, &pStream) != S_OK) {
    std::cerr << "Failed to create stream for image!" << std::endl;
    GlobalFree(hMem);
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return;
  }

  // Load the image
  Gdiplus::Bitmap *catImage = Gdiplus::Bitmap::FromStream(pStream);
  if (!catImage || catImage->GetLastStatus() != Gdiplus::Ok) {
    std::cerr << "Failed to load image from stream!" << std::endl;
    pStream->Release();
    GlobalFree(hMem);
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return;
  }

  pStream->Release();

  // Use the ratio from the map (defaults to 1 if not found)
  int scaleRatio = 1;
  auto it = catRatios.find(catName);
  if (it != catRatios.end()) {
    scaleRatio = it->second;
  }

  int imageWidth = catImage->GetWidth() / scaleRatio;
  int imageHeight = catImage->GetHeight() / scaleRatio;

  // Register a window class for the cat
  WNDCLASS wc = {};
  wc.lpfnWndProc = [](HWND hwnd, UINT uMsg, WPARAM wParam,
                      LPARAM lParam) -> LRESULT {
    static Gdiplus::Bitmap *image = nullptr;
    static int x = 100, y = 100, dx = 5, dy = 5;

    switch (uMsg) {
    case WM_CREATE: {
      image = (Gdiplus::Bitmap *)((LPCREATESTRUCT)lParam)->lpCreateParams;
      SetTimer(hwnd, 1, 16, NULL);
      return 0;
    }
    case WM_TIMER: {
      RECT totalScreen = GetTotalScreenArea();

      // Update the cat's position
      x += dx;
      y += dy;

      // Reverse direction on hitting screen boundaries
      if (x < totalScreen.left || x + image->GetWidth() > totalScreen.right) {
        dx = -dx;
      }
      if (y < totalScreen.top || y + image->GetHeight() > totalScreen.bottom) {
        dy = -dy;
      }

      // Move the window to the new position
      SetWindowPos(hwnd, NULL, x, y, 0, 0,
                   SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

      return 0;
    }

    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);

      RECT rect;
      GetClientRect(hwnd, &rect);
      int windowWidth = rect.right - rect.left;
      int windowHeight = rect.bottom - rect.top;

      // Get the dimensions of the image
      int imageWidth = image->GetWidth();
      int imageHeight = image->GetHeight();

      // Calculate the top-left position to center the image
      int xPos = (windowWidth - imageWidth) / 2;
      int yPos = (windowHeight - imageHeight) / 2;

      Gdiplus::Graphics graphics(hdc);
      graphics.DrawImage(image, xPos, yPos, imageWidth, imageHeight);

      EndPaint(hwnd, &ps);
      return 0;
    }
    case WM_DESTROY:
      KillTimer(hwnd, 1);
      delete image;
      PostQuitMessage(0);
      return 0;
    default:
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
  };
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;

  if (!RegisterClass(&wc)) {
    std::cerr << "Failed to register cat window class!" << std::endl;
    delete catImage;
    GlobalFree(hMem);
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return;
  }

  // Create the window with the scaled size
  HWND hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW |
                                 WS_EX_TRANSPARENT,
                             CLASS_NAME, "", WS_POPUP, 0, 0, imageWidth,
                             imageHeight, NULL, NULL, hInstance, catImage);
  SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);

  if (!hwnd) {
    std::cerr << "Failed to create cat window!" << std::endl;
    delete catImage;
    GlobalFree(hMem);
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return;
  }

  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);
}