#pragma once
#include "pch.h"

inline void launch(LPWSTR lpCmdLine);

std::wstring expandEnvString(winrt::hstring raw);