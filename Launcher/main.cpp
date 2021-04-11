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
		launch(lpCmdLine);
	}
	catch (DWORD &error)
	{
		LPWSTR message;
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
						   FORMAT_MESSAGE_FROM_SYSTEM |
						   FORMAT_MESSAGE_IGNORE_INSERTS,
					   nullptr,
					   error,
					   0UL,
					   (LPWSTR)&message,
					   0UL,
					   nullptr);
		MessageBoxW(GetDesktopWindow(), message, nullptr, MB_ICONERROR);
		LocalFree(message);
		return 0;
	}
	catch (winrt::hresult_error &result_error)
	{
		MessageBoxW(GetDesktopWindow(), result_error.message().c_str(), L"WinRT originate error", MB_ICONERROR);
		return 0;
	}
	return 0;
}

inline void launch(LPWSTR lpCmdLine)
{
	winrt::init_apartment(/*winrt::apartment_type::single_threaded*/);
	STARTUPINFOW startupinfo{};
	PROCESS_INFORMATION procinfo;
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
			auto rst = expandEnvString(env.GetNamedString(L"Value"));
			SetEnvironmentVariableW(env.GetNamedString(L"Variable").c_str(), rst.c_str());
		}
	}
	SetEnvironmentVariableW(L"LauncherDir", nullptr);
	if (__argc > 1)
	{
		wchar_t **argv = __wargv;
		auto pathdir = winrt::Windows::Storage::StorageFile::GetFileFromPathAsync(argv[1]).get().GetParentAsync().get().Path();
		if (!CreateProcessW(argv[1], lpCmdLine, nullptr, nullptr, FALSE, 0UL, nullptr, pathdir.c_str(), &startupinfo, &procinfo))
		{
			throw GetLastError();
		}
		return;
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
		else if (type == L"shell")
		{
			SetEnvironmentVariableW(L"LauncherDir", launcherDir.c_str());
			auto envs = launch.GetNamedArray(L"EnvironmentVariables");
			for (uint32_t i2 = 0U, count2 = envs.Size(); i2 < count2; i2++)
			{
				auto env = envs.GetObjectAt(i2);
				auto rst = expandEnvString(env.GetNamedString(L"Value"));
				SetEnvironmentVariableW(env.GetNamedString(L"Variable").c_str(), rst.c_str());
			}
			auto operation = expandEnvString(launch.GetNamedString(L"Operation"));
			auto filePath = expandEnvString(launch.GetNamedString(L"FilePath"));
			auto workingDirectoryValue = launch.GetNamedValue(L"WorkingDirectory");
			auto ParametersValue = launch.GetNamedValue(L"WorkingDirectory");
			SetEnvironmentVariableW(L"LauncherDir", nullptr);
			{
				int srst = (int)(LONG_PTR)ShellExecuteW(
					nullptr,
					operation.c_str(),
					filePath.c_str(),
					ParametersValue.ValueType() == winrt::Windows::Data::Json::JsonValueType::Null ? nullptr : expandEnvString(launch.GetNamedString(L"Parameters")).c_str(),
					workingDirectoryValue.ValueType() == winrt::Windows::Data::Json::JsonValueType::Null ? nullptr : expandEnvString(launch.GetNamedString(L"WorkingDirectory")).c_str(),
					SW_SHOWDEFAULT);
				if (srst < 33)
				{
					switch (srst)
					{
					case 0:
						MessageBoxW(GetDesktopWindow(), L"The operating system is out of memory or resources.", L"ShellExecute error.", MB_ICONERROR);
						break;
					case ERROR_BAD_FORMAT:
						MessageBoxW(GetDesktopWindow(), L"The .exe file is invalid (non-Win32 .exe or error in .exe image).", L"ShellExecute error.", MB_ICONERROR);
						break;
					case SE_ERR_ACCESSDENIED:
						MessageBoxW(GetDesktopWindow(), L"The operating system denied access to the specified file.", L"ShellExecute error.", MB_ICONERROR);
						break;
					case SE_ERR_ASSOCINCOMPLETE:
						MessageBoxW(GetDesktopWindow(), L"The file name association is incomplete or invalid.", L"ShellExecute error.", MB_ICONERROR);
						break;
					case SE_ERR_DDEBUSY:
						MessageBoxW(GetDesktopWindow(), L"The DDE transaction could not be completed because other DDE transactions were being processed.", L"ShellExecute error.", MB_ICONERROR);
						break;
					case SE_ERR_DDEFAIL:
						MessageBoxW(GetDesktopWindow(), L"The DDE transaction failed.", L"ShellExecute error.", MB_ICONERROR);
						break;
					case SE_ERR_DDETIMEOUT:
						MessageBoxW(GetDesktopWindow(), L"The DDE transaction could not be completed because the request timed out.", L"ShellExecute error.", MB_ICONERROR);
						break;
					case SE_ERR_DLLNOTFOUND:
						MessageBoxW(GetDesktopWindow(), L"The specified DLL was not found.", L"ShellExecute error.", MB_ICONERROR);
						break;
					case SE_ERR_FNF:
						MessageBoxW(GetDesktopWindow(), L"The specified file was not found.", L"ShellExecute error.", MB_ICONERROR);
						break;
					case SE_ERR_NOASSOC:
						MessageBoxW(GetDesktopWindow(), L"There is no application associated with the given file name extension. This error will also be returned if you attempt to print a file that is not printable.", L"ShellExecute error.", MB_ICONERROR);
						break;
					case SE_ERR_OOM:
						MessageBoxW(GetDesktopWindow(), L"There was not enough memory to complete the operation.", L"ShellExecute error.", MB_ICONERROR);
						break;
					case SE_ERR_PNF:
						MessageBoxW(GetDesktopWindow(), L"The specified path was not found.", L"ShellExecute error.", MB_ICONERROR);
						break;
					case SE_ERR_SHARE:
						MessageBoxW(GetDesktopWindow(), L"A sharing violation occurred.", L"ShellExecute error.", MB_ICONERROR);
						break;
					default:
						MessageBoxW(GetDesktopWindow(), L"Unknown error.", L"ShellExecute error.", MB_ICONERROR);
						break;
					}
					return;
				}
			}
			for (uint32_t i2 = 0U, count2 = envs.Size(); i2 < count2; i2++)
			{
				SetEnvironmentVariableW(envs.GetObjectAt(i2).GetNamedString(L"Variable").c_str(), nullptr);
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
				throw(DWORD) ERROR_NOT_FOUND;
			}
		}
		else
		{
			throw(DWORD) ERROR_UNKNOWN_PROPERTY;
		}
	}
}

std::wstring expandEnvString(winrt::hstring raw)
{
	DWORD rst1 = ExpandEnvironmentStringsW(raw.c_str(), nullptr, 0UL);
	if (!rst1)
	{
		throw GetLastError();
	}
	LPWSTR buffer = new wchar_t[rst1];
	if (!ExpandEnvironmentStringsW(raw.c_str(), buffer, rst1))
	{
		delete[] buffer;
		throw GetLastError();
	}
	auto value = std::wstring(buffer);
	delete[] buffer;
	return value;
}