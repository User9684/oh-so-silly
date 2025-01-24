#include "cats.h"
#include <gdiplus.h>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>
#include <windows.h>

#pragma comment(lib, "gdiplus.lib")

struct CatWindowData {
  Gdiplus::Bitmap *image;
  float scaleRatio;
  float x;
  float y;
  float dx;
  float dy;
};

const std::map<std::string, float> catRatios = {
    {"CAT1", 3.0f}, {"CAT2", 2.5f},  {"CAT3", 3.0f}, {"CAT4", 3.5f},
    {"CAT5", 1.0f}, {"CAT6", 0.3f},  {"CAT7", 4.0f}, {"CAT8", 0.5f},
    {"CAT9", 0.7f}, {"CAT10", 7.0f},
};

const std::vector<std::string> cats = {
    "CAT1", "CAT2", "CAT3", "CAT4", "CAT5",
    "CAT6", "CAT7", "CAT8", "CAT9", "CAT10",
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

void SpawnCat(HINSTANCE hInstance) {
  std::string catIdentifier = "CatWindow_" + gen_random(8);

  const LPCSTR CLASS_NAME = catIdentifier.c_str();
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

  // Use the ratio from the map (defaults to 1 if not found)
  float scaleRatio = 1;
  auto it = catRatios.find(catName);
  if (it != catRatios.end()) {
    scaleRatio = it->second;
  }

  float imageWidth = static_cast<float>(catImage->GetWidth()) / scaleRatio;
  float imageHeight = static_cast<float>(catImage->GetHeight()) / scaleRatio;

  CatWindowData *catData = new CatWindowData;
  catData->image = catImage;
  catData->scaleRatio = scaleRatio;

  // Register a window class for the cat
  WNDCLASS wc = {};
  wc.lpfnWndProc = [](HWND hwnd, UINT uMsg, WPARAM wParam,
                      LPARAM lParam) -> LRESULT {
    static CatWindowData *data = nullptr;

    switch (uMsg) {
    case WM_CREATE: {
      CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
      CatWindowData *data = (CatWindowData *)cs->lpCreateParams;

      SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data);

      POINT randomPosition = GetRandomPositionOnScreen();

      data->x = randomPosition.x;
      data->y = randomPosition.y;
      ;
      data->dx = randomBetween(5, 10);
      data->dy = randomBetween(5, 10);

      SetTimer(hwnd, 1, 16, NULL);
      return 0;
    }
    case WM_TIMER: {
      CatWindowData *data =
          (CatWindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
      if (!data)
        return 0;

      float imageWidth =
          static_cast<float>(data->image->GetWidth()) / data->scaleRatio;
      float imageHeight =
          static_cast<float>(data->image->GetHeight()) / data->scaleRatio;

      RECT totalScreen = GetTotalScreenArea();

      // Update the cat's position
      data->x += data->dx;
      data->y += data->dy;

      // Reverse direction on hitting screen boundaries
      if (data->x < totalScreen.left ||
          data->x + imageWidth > totalScreen.right) {
        data->dx = -data->dx;
      }
      if (data->y < totalScreen.top ||
          data->y + imageWidth > totalScreen.bottom) {
        data->dy = -data->dy;
      }

      // Move the window to the new position
      SetWindowPos(hwnd, NULL, data->x, data->y, 0, 0,
                   SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

      return 0;
    }

    case WM_PAINT: {
      CatWindowData *data =
          (CatWindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
      if (!data)
        return 0;

      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);

      RECT rect;
      GetClientRect(hwnd, &rect);
      float windowWidth = static_cast<float>(rect.right - rect.left);
      float windowHeight = static_cast<float>(rect.bottom - rect.top);

      // Get the dimensions of the image
      float scaledWidth =
          static_cast<float>(data->image->GetWidth()) / data->scaleRatio;
      float scaledHeight =
          static_cast<float>(data->image->GetHeight()) / data->scaleRatio;

      // Calculate the top-left position to center the image
      float xPos = (windowWidth - scaledWidth) / 2;
      float yPos = (windowHeight - scaledHeight) / 2;

      Gdiplus::Graphics graphics(hdc);
      graphics.DrawImage(data->image, xPos, yPos, scaledWidth, scaledHeight);

      EndPaint(hwnd, &ps);
      return 0;
    }
    case WM_DESTROY: {
      CatWindowData *data =
          (CatWindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
      if (data) {
        KillTimer(hwnd, 1);
        delete data->image;
        delete data;
      }
      PostQuitMessage(0);
      return 0;
    }
    default:
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
  };
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;

  if (!RegisterClass(&wc)) {
    std::cerr << "Failed to register cat window class!" << std::endl;
    delete catImage;
    delete catData;
    GlobalFree(hMem);
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return;
  }

  // Create the window with the scaled size
  HWND hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW |
                                 WS_EX_TRANSPARENT,
                             CLASS_NAME, "", WS_POPUP, 0, 0, imageWidth,
                             imageHeight, NULL, NULL, hInstance, catData);

  SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);

  if (!hwnd) {
    std::cerr << "Failed to create cat window!" << std::endl;
    delete catImage;
    delete catData;
    GlobalFree(hMem);
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return;
  }

  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);
}
