#include <string>
#include <vector>
#include <windows.h>

int FindResourceByNameAcrossAllTypes(const std::wstring &targetName,
                                     HRSRC &resourceOut);
std::vector<std::string> ListResourcesOfType(PCHAR lpszType);
RECT GetTotalScreenArea();
POINT GetRandomPositionOnScreen();
int randomBetween(int min, int max);