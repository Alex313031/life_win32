#ifndef _GOL_WIN32_H_
#define _GOL_WIN32_H_

#include "version.h"

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>

#include <cwchar>
#include <stdio.h>
#include <strsafe.h>
#include <string>

#include "resource.h"

#define RGB_BLACK RGB(0, 0, 0)
#define RGB_WHITE RGB(255, 255, 255)
#define RGB_RED RGB(255, 0, 0)
#define RGB_GREEN RGB(0, 255, 0)
#define RGB_BLUE RGB(0, 0, 255)
#define RGB_YELLOW RGB(255, 255, 0)

static const LPCWSTR szWinClass = L"GOLWIN32";

#endif // _GOL_WIN32_H_
