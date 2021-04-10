#pragma once
#include "pch.h"

winrt::Windows::Storage::StorageFile getJsonFile();

LPWSTR expandEnvString(LPCWSTR raw);