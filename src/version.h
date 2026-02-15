#ifndef _GOL_WIN32_VERSION_H_
#define _GOL_WIN32_VERSION_H_

//#ifdef __clang__
// #pragma code_page(65001) // UTF-8
//#endif // __clang__

#ifndef __MINGW32__
 #include <WinSDKVer.h> // Doesn't exist in MinGW
#endif // __MINGW32__

#ifndef _WIN32_WINNT
 #define _WIN32_WINNT 0x0500 // Windows 2000
#endif // _WIN32_WINNT
#ifndef _WIN64_WINNT
 #define _WIN64_WINNT 0x0502 // Windows Server 2003
#endif // _WIN64_WINNT
#ifndef _WIN32_IE
 #define _WIN32_IE 0x0501 // Minimum Internet Explorer version for common controls
#endif // _WIN32_IE

#ifndef _ATL_XP_TARGETING
 #define _ATL_XP_TARGETING
#endif // _ATL_XP_TARGETING

#ifndef __MINGW32__
 #include <SDKDDKVer.h> // Doesn't exist in MinGW
#endif // __MINGW32__

// Macro to convert to string
#if !defined(_STRINGIZER_)
 #define _STRINGIZER_
 #define _STRINGIZER(in) #in
 #define STRINGIZE(in) _STRINGIZER(in)
#endif // !defined(_STRINGIZER_)

// Main version constant
#ifndef _VERSION
 // Run stringizer above
 #define _VERSION(major,minor,build) STRINGIZE(major) "." STRINGIZE(minor) "." STRINGIZE(build)
#endif // _VERSION

// Adhere to semver
#define MAJOR_VERSION 0
#define MINOR_VERSION 0
#define BUILD_VERSION 2

#define VERSION_STRING _VERSION(MAJOR_VERSION, MINOR_VERSION, BUILD_VERSION)
#define ABOUT_TITLE L"About Game of Life"
#define ABOUT_CONTENT L"gol_win32 ver. " VERSION_STRING
#define ABOUT_COPYRIGHT L"\251 2026 Alex313031" // \251 is the © symbol
#define LEGAL_COPYRIGHT L"\251 2026 Alex313031"

#endif // _GOL_WIN32_VERSION_H_
