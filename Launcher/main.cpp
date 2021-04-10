#include "pch.h"
#include "main.h"


int wmain(int argc, wchar_t** argv, wchar_t** /*envp*/)
{
	winrt::init_apartment(/*winrt::apartment_type::single_threaded*/);
	STARTUPINFOW startupinfo{};
	PROCESS_INFORMATION procinfo;
	if (argc > 1)
	{
		wchar_t pathdir[260];
		{
			errno_t err = _wsplitpath_s(argv[1], pathdir, 3, pathdir + 2, 258, nullptr, 0, nullptr, 0);
			if (err)
			{
				return err;
			}
		}
		if (!CreateProcessW(argv[1], _get_wide_winmain_command_line(), nullptr, nullptr, FALSE, 0UL, nullptr, pathdir, &startupinfo, &procinfo))
		{
			return GetLastError();
		}
		return 0;
	}
	auto jsonFile = getJsonFile();
	auto hstlauncherDir = jsonFile.GetParentAsync().get().Path();
	LPCWSTR launcherDir = hstlauncherDir.c_str();
	auto doc = winrt::Windows::Data::Json::JsonObject::Parse(winrt::Windows::Storage::FileIO::ReadTextAsync(jsonFile).get());
	SetEnvironmentVariableW(L"LauncherDir", launcherDir);
	{
		auto envs = doc.GetNamedArray(L"EnvironmentVariables");
		for (uint32_t i = 0, count = envs.Size(); i < count; i++)
		{
			auto env = envs.GetObjectAt(i);
			LPWSTR rst = expandEnvString(env.GetNamedString(L"Value").c_str());
			if (!rst)
			{
				return GetLastError();
			}
			SetEnvironmentVariableW(env.GetNamedString(L"Variable").c_str(), rst);
			delete[] rst;
		}
	}
	SetEnvironmentVariableW(L"LauncherDir", nullptr);
	auto launches = doc.GetNamedArray(L"LaunchApps");
	for (uint32_t i = 0, count = launches.Size(); i < count; i++)
	{
		auto launch = launches.GetObjectAt(i);
		auto type = launch.GetNamedString(L"Type");
		if (type == L"process")
		{
			SetEnvironmentVariableW(L"LauncherDir", launcherDir);
			auto envs = launch.GetNamedArray(L"EnvironmentVariables");
			for (uint32_t i2 = 0, count2 = envs.Size(); i2 < count2; i2++)
			{
				auto env = envs.GetObjectAt(i2);
				LPWSTR rst = expandEnvString(env.GetNamedString(L"Value").c_str());
				if (!rst)
				{
					return GetLastError();
				}
				SetEnvironmentVariableW(env.GetNamedString(L"Variable").c_str(), rst);
				delete[] rst;
			}
			LPWSTR appPath = expandEnvString(launch.GetNamedString(L"AppPath").c_str());
			if (!appPath)
			{
				return GetLastError();
			}
			LPWSTR workingDirectory = expandEnvString(launch.GetNamedString(L"WorkingDirectory").c_str());
			if (!workingDirectory)
			{
				return GetLastError();
			}
			LPWSTR commandLine = expandEnvString(launch.GetNamedString(L"CommandLine").c_str());
			if (!commandLine)
			{
				return GetLastError();
			}
			SetEnvironmentVariableW(L"LauncherDir", nullptr);
			{
				BOOL crst = CreateProcessW(appPath, commandLine, nullptr, nullptr, FALSE, 0UL, nullptr, workingDirectory, &startupinfo, &procinfo);
				delete[] appPath;
				delete[] workingDirectory;
				delete[] commandLine;
				if (!crst)
				{
					return GetLastError();
				}
			}
			for (uint32_t i2 = 0, count2 = envs.Size(); i2 < count2; i2++)
			{
				SetEnvironmentVariableW(envs.GetObjectAt(i2).GetNamedString(L"Variable").c_str(), nullptr);
			}
			if (launch.GetNamedBoolean(L"Wait"))
			{
				if (WaitForSingleObject(procinfo.hProcess, INFINITE) == WAIT_FAILED)
				{
					return GetLastError();
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
				return 1;
			}
		}
	}
	return 0;
}

winrt::Windows::Storage::StorageFile getJsonFile()
{
	wchar_t filePath[260];
	GetModuleFileNameW(nullptr, filePath, 260);
	auto filePathJson = std::wstring(filePath);
	auto end = filePathJson.end();
	filePathJson.erase(end - 3, end);
	filePathJson.append(L"json");
	return winrt::Windows::Storage::StorageFile::GetFileFromPathAsync(filePathJson).get();
}

LPWSTR expandEnvString(LPCWSTR raw)
{
	DWORD rst1 = ExpandEnvironmentStringsW(raw, nullptr, 0);
	if (!rst1)
	{
		return 0;
	}
	LPWSTR value = new wchar_t[rst1];
	if (!ExpandEnvironmentStringsW(raw, value, rst1))
	{
		return 0;
	}
	return value;
}