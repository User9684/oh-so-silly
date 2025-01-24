#include <iostream>
#include <string>
#include <windows.h>

#pragma once

// Callback to find the resource by name
struct ResourceSearchContext {
  std::wstring targetName;
  HRSRC resource = nullptr;
  bool found = false;
};

inline std::wstring convert(const std::string &as) {
  if (as.empty())
    return std::wstring();

  size_t reqLength =
      ::MultiByteToWideChar(CP_UTF8, 0, as.c_str(), (int)as.length(), 0, 0);

  std::wstring ret(reqLength, L'\0');

  ::MultiByteToWideChar(CP_UTF8, 0, as.c_str(), (int)as.length(), &ret[0],
                        (int)ret.length());

  return ret;
}

BOOL CALLBACK EnumNamesCallback(HMODULE hModule, LPCSTR lpszType,
                                LPSTR lpszName, LONG_PTR lParam) {
  auto *context = reinterpret_cast<ResourceSearchContext *>(lParam);

  std::wstring resourceName;
  if (IS_INTRESOURCE(lpszName)) {
    resourceName = std::to_wstring(reinterpret_cast<ULONG_PTR>(lpszName));
  } else {
    resourceName = convert(lpszName);
  }

  if (resourceName == context->targetName) {
    HRSRC hResInfo = FindResource(hModule, lpszName, lpszType);
    if (!hResInfo) {
      // Continue enumerating as the resource wasn't found
      return true;
    }

    HANDLE hData = LoadResource(hModule, hResInfo);
    if (hData) {
      context->resource = hResInfo;
      context->found = true;

      FreeResource(hResInfo);
      return false; // Stop enumeration as we've found the resource
    }
  }

  return true; // Continue enumeration
}

// Callback to enumerate all resource types
BOOL CALLBACK EnumTypesCallback(HMODULE hModule, PCHAR lpszType,
                                LONG_PTR lParam) {
  auto *context = reinterpret_cast<ResourceSearchContext *>(lParam);
  if (context->found)
    return false; // Stop if already found

  if (!EnumResourceNames(hModule, lpszType, EnumNamesCallback, lParam)) {
    if (GetLastError() != ERROR_RESOURCE_TYPE_NOT_FOUND) {
    }
  }
  return !context->found; // Continue enumeration if not found
}

// Function to find a resource by name across all types
int FindResourceByNameAcrossAllTypes(const std::wstring &targetName,
                                     HRSRC &resourceOut) {
  HMODULE hModule = GetModuleHandle(NULL);
  if (hModule == NULL) {
    // std::cerr << "Failed to get module handle: " << GetLastError() <<
    // std::endl;
    return -1;
  }

  ResourceSearchContext context{targetName};
  if (!EnumResourceTypes(hModule, EnumTypesCallback,
                         reinterpret_cast<LONG_PTR>(&context))) {
    if (!context.found) {
      std::cerr << "Failed to find resource: " << GetLastError() << std::endl;
      return -1;
    }
  }

  if (context.found) {
    resourceOut = context.resource;
    return 0; // Success
  }

  return -1; // Not found
}
