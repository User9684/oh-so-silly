#include <windows.h>
#include <iostream>

void PlayResource(const HRSRC resource) {
  // Load resource
  HGLOBAL hRes = LoadResource(NULL, resource);
  if (!hRes) {
    return;
  }
  std::cout << "Loaded resource" << std::endl;

  // Lock resource
  LPVOID lpRes = LockResource(hRes);
  if (lpRes == NULL) {
    std::cerr << "Failed to lock resource :C" << std::endl;
    return;
  }
  std::cout << "Locked resource" << std::endl;

  LPCSTR sound = static_cast<LPCSTR>(lpRes);
  std::cout << "Cast resource to LCPSTR" << std::endl;

  BOOL played = sndPlaySound(sound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);

  if (!played) {
    std::cerr << "Failed to play sound" << std::endl;
  } else {
    std::cout << "Sound playback started successfully!" << std::endl;
    UnlockResource(hRes);
  }

  FreeResource(hRes);
}