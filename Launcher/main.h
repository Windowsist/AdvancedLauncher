#pragma once
#include "pch.h"

inline void launch(LPWSTR lpCmdLine);

std::wstring expandEnvString(const winrt::param::hstring &_raw);