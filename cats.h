#include <string>
#include <windows.h>

int FindResourceByNameAcrossAllTypes(const std::wstring &targetName,
                                     HRSRC &resourceOut);
RECT GetTotalScreenArea();