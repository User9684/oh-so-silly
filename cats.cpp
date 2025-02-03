#include "cats.h"
#include <cstddef>
#include <gdiplus.h>
#include <iostream>
#include <map>
#include <minwindef.h>
#include <random>
#include <string>
#include <vector>
#include <windows.h>
#include <winuser.h>

#pragma comment(lib, "gdiplus.lib")

struct CatWindowData {
  std::string imageName; // Image name to reference loadedImages

  float mxsr; // Maximum scale ratio
  float mnsr; // Maximum scale ratio

  float scaleRatio; // Current scale ratio
  float dsr;        // Amount to increase/decrease each frame

  float x;  // Current X position
  float y;  // Current Y position
  float dx; // Amount to move X each frame
  float dy; // Amount to move Y each frame
};

std::vector<CatWindowData> catImages;
HWND mainWindow;
HDC hdcMem = NULL;
HBITMAP hbmMem = NULL;
RECT clientRect;
Gdiplus::Graphics *graphics = nullptr;
ULONG_PTR gdiplusToken;

const int maxSize = 350;
const int minSize = 200;

std::vector<std::string> cats = {};

std::map<std::string, Gdiplus::Bitmap *> loadedImages = {

};

std::string gen_random(const int len) {
  static const char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz";
  std::string tmp_s;
  tmp_s.reserve(len);

  for (int i = 0; i < len; ++i) {
    tmp_s += characters[rand() % (sizeof(characters) - 1)];
  }

  return tmp_s;
}

float randomBetween(float min, float max) {
  std::random_device rdev;
  std::mt19937 rgen(rdev());
  std::uniform_real_distribution<float> fdist(min, max);

  return fdist(rgen);
}

LRESULT CALLBACK CatWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                               LPARAM lParam) {

  switch (uMsg) {
  case WM_TIMER: {
    RECT windowRect;
    GetWindowRect(hwnd, &windowRect);
    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    for (size_t i = 0; i < catImages.size(); ++i) {
      CatWindowData &cat = catImages[i];

      auto it = loadedImages.find(cat.imageName);
      if (it == loadedImages.end()) {
        continue;
      }

      Gdiplus::Bitmap *image = it->second;

      cat.scaleRatio += cat.dsr;
      if (cat.scaleRatio > cat.mxsr || cat.scaleRatio < cat.mnsr) {
        cat.dsr = -cat.dsr;
      }

      float width = image->GetWidth() / cat.scaleRatio;
      float height = image->GetHeight() / cat.scaleRatio;

      cat.x += cat.dx;
      cat.y += cat.dy;

      if (cat.x - (width / 2) < 0 || cat.x + (width / 2) > windowWidth) {
        cat.dx = -cat.dx;
      }
      if (cat.y - (height / 2) < 0 || cat.y + (height / 2) > windowHeight)
        cat.dy = -cat.dy;
    }
    RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
    UpdateWindow(mainWindow);
    return 0;
  }
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    // Ensure the buffer is valid
    if (!graphics) {
      EndPaint(hwnd, &ps);
      return 0;
    }

    // Clear the buffer
    graphics->Clear(Gdiplus::Color(0, 0, 0, 0));

    // Draw cats onto the buffer
    for (const auto &cat : catImages) {
      auto it = loadedImages.find(cat.imageName);
      if (it == loadedImages.end()) {
        continue;
      }

      Gdiplus::Bitmap *image = it->second;

      float w = image->GetWidth() / cat.scaleRatio;
      float h = image->GetHeight() / cat.scaleRatio;
      graphics->DrawImage(image, cat.x - (w / 2), cat.y - (h / 2), w, h);
    }

    // Copy the buffer to the window
    BitBlt(hdc, 0, 0, clientRect.right - clientRect.left,
           clientRect.bottom - clientRect.top, hdcMem, 0, 0, SRCCOPY);

    EndPaint(hwnd, &ps);
    return 0;
  }
  case WM_ERASEBKGND:
    return 1;
  case WM_DESTROY:
    if (graphics) {
      delete graphics;
      graphics = nullptr;
    }
    if (hbmMem) {
      DeleteObject(hbmMem);
      hbmMem = NULL;
    }
    if (hdcMem) {
      DeleteDC(hdcMem);
      hdcMem = NULL;
    }
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int InitCats(HINSTANCE hInstance) {
  char type[] = "CAT";
  cats = ListResourcesOfType(type);

  Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) !=
      Gdiplus::Ok) {
    std::cerr << "Failed to initialize GDI+!" << std::endl;
    return 1;
  }

  WNDCLASS wc = {};
  wc.lpfnWndProc = CatWindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = "CatWindow";
  if (!RegisterClass(&wc)) {
    std::cerr << "Failed to register cat window class!" << std::endl;
    return 1;
  }

  RECT totalArea = GetTotalScreenArea();
  mainWindow = CreateWindowEx(
      WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT,
      "CatWindow", "", WS_POPUP, totalArea.left, totalArea.top,
      totalArea.right - totalArea.left, totalArea.bottom - totalArea.top, NULL,
      NULL, hInstance, NULL);

  SetLayeredWindowAttributes(mainWindow, RGB(0, 0, 0), 0, LWA_COLORKEY);

  ShowWindow(mainWindow, SW_SHOW);
  SetTimer(mainWindow, 1, 16, NULL);

  HDC hdc = GetDC(mainWindow);
  hdcMem = CreateCompatibleDC(hdc);
  GetClientRect(mainWindow, &clientRect);
  hbmMem = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
  SelectObject(hdcMem, hbmMem);
  graphics = new Gdiplus::Graphics(hdcMem);
  ReleaseDC(mainWindow, hdc);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}

void SpawnCat() {
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

  // Create an IStream from the resource data
  HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, imageSize);
  memcpy(GlobalLock(hMem), pImageData, imageSize);
  GlobalUnlock(hMem);

  IStream *pStream = NULL;
  if (CreateStreamOnHGlobal(hMem, true, &pStream) != S_OK) {
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

  float maxDimenstion = max(catImage->GetWidth(), catImage->GetHeight());

  float maxScaleRatio = maxDimenstion / minSize;
  float minScaleRatio = maxDimenstion / maxSize;

  std::cout << "min sr: " << minScaleRatio << " max sr: " << maxScaleRatio
            << std::endl;

  // Use the ratio from the map (defaults to 1 if not found)
  float scaleRatio = randomBetween(minScaleRatio, maxScaleRatio);

  float imageWidth = static_cast<float>(catImage->GetWidth()) / scaleRatio;
  float imageHeight = static_cast<float>(catImage->GetHeight()) / scaleRatio;

  float scaleRatioDist = randomBetween((1.0f / 100.0f) * maxScaleRatio,
                                       (5.0f / 100.0f) * minScaleRatio);
  float xOffset = (catImage->GetWidth() / 2.0f) / scaleRatio;
  float yOffset = (catImage->GetHeight() / 2.0f) / scaleRatio;

  POINT randomPosition = GetRandomPositionOnScreen();

  if (loadedImages.find(catName) == loadedImages.end()) {
    loadedImages.insert({catName, catImage});
  }

  catImages.push_back({
      catName, // imageName

      maxScaleRatio, // mxsr
      minScaleRatio, // mnsr

      scaleRatio,     // scaleRatio
      scaleRatioDist, // dsr

      static_cast<float>(randomPosition.x) + imageWidth / 2,  // x
      static_cast<float>(randomPosition.y) + imageHeight / 2, // y
      static_cast<float>(randomBetween(5, 10)),               // dx
      static_cast<float>(randomBetween(5, 10)),               // dy
  });
}
