// clang-format off
#include <iostream>
#include <algorithm>
#include <string>
#include <cctype>
#include <windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <processthreadsapi.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <vector>
// clang-format on

#pragma comment(lib, "wbemuuid.lib")

const std::vector<std::string> appNames = {"taskmgr.exe", "cmd.exe",
                                           "powershell.exe"};
const int exitCode = 69420;

bool appInVector(std::string &processName) {
  std::transform(processName.begin(), processName.end(), processName.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  return (std::find(appNames.begin(), appNames.end(), processName) !=
          appNames.end());
}

std::string bstrToString(BSTR bstr) {
  if (!bstr) {
    return "";
  }
  std::wstring wstr(bstr, SysStringLen(bstr));

  std::string str;
  std::transform(wstr.begin(), wstr.end(), std::back_inserter(str),
                 [](wchar_t c) { return (char)c; });
  return str;
}

int closeProcesses() {
  PROCESSENTRY32 pe32;
  pe32.dwSize = sizeof(PROCESSENTRY32);

  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnapshot == INVALID_HANDLE_VALUE) {
    std::cerr << "failed to create snapshot" << std::endl;
    return 1;
  }

  if (Process32First(hSnapshot, &pe32)) {
    do {
      std::string exeFileStr(pe32.szExeFile);

      if (!appInVector(exeFileStr)) {
        continue;
      }

      HANDLE hProcess =
          OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
      if (hProcess == NULL) {
        std::cerr << "Failed to open process handle" << std::endl;
        continue;
      }

      TerminateProcess(hProcess, exitCode);
      CloseHandle(hProcess);

    } while (Process32Next(hSnapshot, &pe32));
  }

  CloseHandle(hSnapshot);

  return 0;
}

int InitProcessKilling() {
  closeProcesses();
  HRESULT hres;

  hres = CoInitializeEx(0, COINIT_MULTITHREADED);
  if (FAILED(hres)) {
    std::cerr << "Failed to initialize COM library. Error code: " << hres
              << std::endl;
    return 1;
  }

  hres =
      CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
                           RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

  if (FAILED(hres)) {
    std::cerr << "Failed to initialize security. Error code: " << hres
              << std::endl;
    CoUninitialize();
    return 1;
  }

  IWbemLocator *pLoc = NULL;
  hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator, (LPVOID *)&pLoc);

  if (FAILED(hres)) {
    std::cerr << "Failed to create IWbemLocator object. Error code: " << hres
              << std::endl;
    CoUninitialize();
    return 1;
  }

  IWbemServices *pSvc = NULL;
  hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0,
                             &pSvc);

  if (FAILED(hres)) {
    std::cerr << "Could not connect to WMI. Error code: " << hres << std::endl;
    pLoc->Release();
    CoUninitialize();
    return 1;
  }

  IEnumWbemClassObject *pEnumerator = NULL;
  hres = pSvc->ExecNotificationQuery(
      _bstr_t("WQL"),
      _bstr_t("SELECT * FROM __InstanceCreationEvent WITHIN 1 WHERE "
              "TargetInstance ISA 'Win32_Process'"),
      WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL,
      &pEnumerator);

  if (FAILED(hres)) {
    std::cerr << "Failed to set up event query. Error code: " << hres
              << std::endl;
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
    return 1;
  }

  std::cout << "Listening for process creation events..." << std::endl;

  while (pEnumerator) {
    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

    if (0 == uReturn) {
      break;
    }

    VARIANT vtProp;
    hres = pclsObj->Get(L"TargetInstance", 0, &vtProp, 0, 0);

    if (SUCCEEDED(hres)) {
      IUnknown *str = vtProp.punkVal;
      IWbemClassObject *pObj2 = NULL;
      str->QueryInterface(IID_IWbemClassObject, (void **)&pObj2);

      VARIANT name, pid;
      pObj2->Get(L"Name", 0, &name, 0, 0);
      pObj2->Get(L"ProcessId", 0, &pid, 0, 0);

      std::string processName = bstrToString(name.bstrVal);
      DWORD processID = pid.uintVal;

      if (appInVector(processName)) {
        std::cout << "Tracked process started: " << processName << std::endl;
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processID);
        if (hProcess == NULL) {
          std::cerr << "Failed to open process handle" << std::endl;
          continue;
        }
        TerminateProcess(hProcess, exitCode);
      }

      pObj2->Release();
      VariantClear(&name);
    }

    pclsObj->Release();
  }

  pSvc->Release();
  pLoc->Release();
  CoUninitialize();

  return 0;
}