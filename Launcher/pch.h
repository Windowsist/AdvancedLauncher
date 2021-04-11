
// #define WINAPI_FAMILY WINAPI_FAMILY_PC_APP
// #define WINAPI_FAMILY WINAPI_PARTITION_SYSTEM
#define WINAPI_FAMILY WINAPI_FAMILY_GAMES
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef WINRT_LEAN_AND_MEAN
#define WINRT_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <shellapi.h>
#include <corecrt_startup.h>
#include <winrt/base.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
