#pragma once 

#pragma comment(linker,"/NODEFAULTLIB:msvcrt.lib")
#pragma comment(lib,"store/msvcrt.lib")
#pragma comment(lib,"WindowsApp.lib")

// #define WINAPI_FAMILY_PC_APP        /* Windows Store Applications */
// #define WINAPI_FAMILY_PHONE_APP     /* Windows Phone Applications */
// #define WINAPI_FAMILY_SYSTEM        /* Windows Drivers and Tools */
// #define WINAPI_FAMILY_SERVER        /* Windows Server Applications */
// #define WINAPI_FAMILY_GAMES         /* Windows Games and Applications */
// #define WINAPI_FAMILY_DESKTOP_APP   /* Windows Desktop Applications */

#define WINAPI_FAMILY WINAPI_FAMILY_SYSTEM

#define _CONSOLE
#define WIN32_LEAN_AND_MEAN
#define WINRT_LEAN_AND_MEAN

#include <windows.h>
#include <corecrt_startup.h>
#include <winrt/base.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
