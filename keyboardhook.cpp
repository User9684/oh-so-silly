#include "keyboardhook.h"
#include <codecvt>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <windows.h>

HHOOK keyboardHook;
HRSRC resource;

// Keys to block and force input ":3" instead
const std::vector<UINT> uwus = {
    0xBF, VK_DIVIDE, 0x2B, VK_OEM_5, VK_MENU, VK_LMENU, VK_RMENU,
};
// Keys to play meow sfx when pressed
const std::vector<UINT> meows = {
    VK_ACCEPT,
    VK_RETURN,
};

// uwus and meows combined
const std::vector<UINT> kitties = [] {
  std::vector<UINT> combined = uwus;
  combined.insert(combined.end(), meows.begin(), meows.end());
  return combined;
}();

std::string WStringToString(const std::wstring &wstr) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  return converter.to_bytes(wstr);
}

void Meow() {
  // Play the sound asynchronously
  std::thread soundThread(PlayResource, resource);

  // Detach the thread so it runs independently
  soundThread.detach();
  return;
}

void SimulateKeyPress(const std::string &text) {
  for (char c : text) {
    // Create a keyboard input for each character
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = 0;   // Virtual key code not needed for ASCII characters
    input.ki.wScan = c; // Scan code for the character
    input.ki.dwFlags = KEYEVENTF_UNICODE;

    // Send the key down event
    SendInput(1, &input, sizeof(INPUT));

    // Send the key up event
    input.ki.dwFlags |= KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
  }
}

LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION) {
    KBDLLHOOKSTRUCT *keyInfo = (KBDLLHOOKSTRUCT *)lParam;

    if ((wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) &&
        std::find(kitties.begin(), kitties.end(), keyInfo->vkCode) !=
            kitties.end()) {
      std::thread soundThread(Meow);
      soundThread.detach();
      SpawnCat();
    }

    if ((wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) &&
        (std::find(uwus.begin(), uwus.end(), keyInfo->vkCode) != uwus.end())) {
      SimulateKeyPress(":3");
      return 1;
    }
  }

  // Pass the event to the next hook
  int nextHook = CallNextHookEx(NULL, nCode, wParam, lParam);
  if (nextHook == 0) {
    return nextHook;
  }
  return 1;
}

int InitKeyboardHook(HINSTANCE hInstance) {
  const std::wstring resourceName = L"IDR_SOUND1";
  if (FindResourceByNameAcrossAllTypes(resourceName, resource) != 0) {
    std::cerr << "Resource not found: " << WStringToString(resourceName)
              << std::endl;
    return 1;
  }

  HHOOK keyboardHook =
      SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, NULL, 0);
  if (!keyboardHook) {
    std::cerr << "Failed to set up keyboard hook!" << std::endl;
    return 1;
  }

  std::cout << "Keyboard hook installed. Listening for keypresses..."
            << std::endl;

  return 0;
}