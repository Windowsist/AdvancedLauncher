#include "pch.h"
#include "main.h"

int
	WINAPI
	wWinMain(
		_In_ HINSTANCE /*hInstance*/,
		_In_opt_ HINSTANCE /*hPrevInstance*/,
		_In_ LPWSTR lpCmdLine,
		_In_ int /*nShowCmd*/)
try
{
	winrt::init_apartment(winrt::apartment_type::single_threaded);
	auto filePathJson = winrt::hstring();
	{
		wchar_t buffer[MAX_PATH];
		DWORD rst1 = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
		if ((!rst1) || GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			winrt::throw_last_error();
		}
		filePathJson = winrt::hstring(buffer, (uint32_t)wcslen(buffer) - 3) + L"json";
	}
	auto jsonFile = winrt::Windows::Storage::StorageFile::GetFileFromPathAsync(filePathJson).get();
	auto launcherDir = jsonFile.GetParentAsync().get().Path();
	auto jsonObj = winrt::Windows::Data::Json::JsonObject::Parse(winrt::Windows::Storage::FileIO::ReadTextAsync(jsonFile).get());
	SetEnvironmentVariableW(L"LauncherDir", launcherDir.c_str());
	{
		auto envs = jsonObj.GetNamedArray(L"EnvironmentVariables");
		for (uint32_t i = 0U, count = envs.Size(); i < count; i++)
		{
			auto env = envs.GetObjectAt(i);
			auto rst = expandEnvString(env.GetNamedString(L"Value"));
			SetEnvironmentVariableW(env.GetNamedString(L"Variable").c_str(), rst.c_str());
		}
	}
	SetEnvironmentVariableW(L"LauncherDir", nullptr);
	STARTUPINFOW startupinfo{};
	PROCESS_INFORMATION procinfo;
	if (__argc > 1)
	{
		wchar_t **argv = __wargv;
		auto pathdir = winrt::Windows::Storage::StorageFile::GetFileFromPathAsync(argv[1]).get().GetParentAsync().get().Path();
		if (!CreateProcessW(argv[1], lpCmdLine, nullptr, nullptr, FALSE, 0UL, nullptr, pathdir.c_str(), &startupinfo, &procinfo))
		{
			winrt::throw_last_error();
		}
		return 0;
	}
	auto launches = jsonObj.GetNamedArray(L"LaunchApps");
	for (uint32_t i = 0U, count = launches.Size(); i < count; i++)
	{
		auto launch = launches.GetObjectAt(i);
		auto type = launch.GetNamedString(L"Type");
		if (type == L"process")
		{
			SetEnvironmentVariableW(L"LauncherDir", launcherDir.c_str());
			auto envs = launch.GetNamedArray(L"EnvironmentVariables");
			for (uint32_t i2 = 0U, count2 = envs.Size(); i2 < count2; i2++)
			{
				auto env = envs.GetObjectAt(i2);
				auto rst = expandEnvString(env.GetNamedString(L"Value"));
				SetEnvironmentVariableW(env.GetNamedString(L"Variable").c_str(), rst.c_str());
			}
			auto appPath = expandEnvString(launch.GetNamedString(L"AppPath"));
			auto workingDirectory = expandEnvString(launch.GetNamedString(L"WorkingDirectory"));
			auto commandLine = expandEnvString(launch.GetNamedString(L"CommandLine"));
			SetEnvironmentVariableW(L"LauncherDir", nullptr);
			{
				BOOL crst = CreateProcessW(appPath.c_str(), std::wstring(commandLine).data(), nullptr, nullptr, FALSE, 0UL, nullptr, workingDirectory.c_str(), &startupinfo, &procinfo);
				if (!crst)
				{
					winrt::throw_last_error();
				}
			}
			for (uint32_t i2 = 0U, count2 = envs.Size(); i2 < count2; i2++)
			{
				SetEnvironmentVariableW(envs.GetObjectAt(i2).GetNamedString(L"Variable").c_str(), nullptr);
			}
			if (launch.GetNamedBoolean(L"Wait"))
			{
				if (WaitForSingleObject(procinfo.hProcess, INFINITE) == WAIT_FAILED)
				{
					winrt::throw_last_error();
				}
			}
		}
		else if (type == L"AppListEntry")
		{
			auto id = launch.GetNamedString(L"Id");
			auto entries = winrt::Windows::ApplicationModel::Package::Current().GetAppListEntries();
			bool found = false;
			for (uint32_t i2 = 0U, size = entries.Size(); i2 < size; i2++)
			{
				auto entry = entries.GetAt(i2);
				if (entry.AppUserModelId() == id)
				{
					entry.LaunchAsync().get();
					found = true;
					break;
				}
			}
			if (!found)
			{
				winrt::throw_hresult(ERROR_NOT_FOUND);
			}
		}
		else
		{
			winrt::throw_hresult(ERROR_UNKNOWN_PROPERTY);
		}
	}
	return 0;
}
catch (winrt::hresult_error &result_error)
{
	FatalAppExitW(0, result_error.message().c_str());
	return 0;
}

winrt::hstring expandEnvString(const winrt::hstring &raw)
{
	DWORD rst1 = ExpandEnvironmentStringsW(raw.c_str(), nullptr, 0UL);
	if (!rst1)
	{
		winrt::throw_last_error();
	}
	LPWSTR buffer = new wchar_t[rst1];
	if (!ExpandEnvironmentStringsW(raw.c_str(), buffer, rst1))
	{
		delete[] buffer;
		winrt::throw_last_error();
	}
	auto value = winrt::hstring(buffer);
	delete[] buffer;
	return value;
}