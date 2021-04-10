#include "pch.h"
#include "main.h"

int
	WINAPI
	wWinMain(
		_In_ HINSTANCE hInstance,
		_In_opt_ HINSTANCE hPrevInstance,
		_In_ LPWSTR lpCmdLine,
		_In_ int nShowCmd)
{
	try
	{
		winrt::init_apartment(/*winrt::apartment_type::single_threaded*/);
		STARTUPINFOW startupinfo{};
		PROCESS_INFORMATION procinfo;
		if (__argc > 1)
		{
			wchar_t **argv = __wargv;
			auto pathdir = winrt::Windows::Storage::StorageFile::GetFileFromPathAsync(argv[1]).get().GetParentAsync().get().Path();
			if (!CreateProcessW(argv[1], lpCmdLine, nullptr, nullptr, FALSE, 0UL, nullptr, pathdir.c_str(), &startupinfo, &procinfo))
			{
				throw GetLastError();
			}
			return 0;
		}
		auto filePathJson = std::wstring();
		{
			wchar_t buffer[260];
			DWORD rst1 = GetModuleFileNameW(nullptr, buffer, 260);
			if (!rst1)
			{
				throw GetLastError();
			}
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				throw ERROR_INSUFFICIENT_BUFFER;
			}
			filePathJson.assign(buffer);
			auto end = filePathJson.end();
			auto beg = end - 3;
			filePathJson.erase(end - 3, end);
			filePathJson.append(L"json");
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
				auto rst = expandEnvString(env.GetNamedString(L"Value").c_str());
				SetEnvironmentVariableW(env.GetNamedString(L"Variable").c_str(), rst.c_str());
			}
		}
		SetEnvironmentVariableW(L"LauncherDir", nullptr);
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
					auto rst = expandEnvString(env.GetNamedString(L"Value").c_str());
					SetEnvironmentVariableW(env.GetNamedString(L"Variable").c_str(), rst.c_str());
				}
				auto appPath = expandEnvString(launch.GetNamedString(L"AppPath").c_str());
				auto workingDirectory = expandEnvString(launch.GetNamedString(L"WorkingDirectory").c_str());
				auto commandLine = expandEnvString(launch.GetNamedString(L"CommandLine").c_str());
				SetEnvironmentVariableW(L"LauncherDir", nullptr);
				{
					BOOL crst = CreateProcessW(appPath.c_str(), commandLine.data(), nullptr, nullptr, FALSE, 0UL, nullptr, workingDirectory.c_str(), &startupinfo, &procinfo);
					if (!crst)
					{
						throw GetLastError();
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
						throw GetLastError();
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
					auto curhid = entry.AppUserModelId();
					if (curhid == id)
					{
						entry.LaunchAsync().get();
						found = true;
						break;
					}
				}
				if (!found)
				{
					throw nullptr;
				}
			}
		}
	}
	catch (DWORD &error)
	{
		return error;
	}
	return 0;
}

std::wstring expandEnvString(LPCWSTR raw)
{
	DWORD rst1 = ExpandEnvironmentStringsW(raw, nullptr, 0UL);
	if (!rst1)
	{
		throw GetLastError();
	}
	LPWSTR buffer = new wchar_t[rst1];
	if (!ExpandEnvironmentStringsW(raw, buffer, rst1))
	{
		delete[] buffer;
		throw GetLastError();
	}
	auto value = std::wstring(buffer);
	delete[] buffer;
	return value;
}