#include "cats.cpp"
#include "find.cpp"
#include "keyboardhook.cpp"
#include "pinkhue.cpp"
#include "playresource.cpp"
#include "protect.cpp"
#include "screenarea.cpp"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {

  InitKeyboardHook(hInstance);
  std::thread listenerThread(InitProcessKilling);
  ApplyHueOverlay(hInstance);
  SpawnCat(hInstance);

  // Message loop to keep the application running
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // Unhook the keyboard hook on exit
  UnhookWindowsHookEx(keyboardHook);
  return 0;
}
