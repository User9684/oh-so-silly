#include <string>
#include <windows.h>

int FindResourceByNameAcrossAllTypes(const std::wstring &targetName,
                                     HRSRC &resourceOut);
void PlayResource(const HRSRC resource);
void SpawnCat();