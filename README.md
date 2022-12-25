# AdvancedLauncher

start process or AppListEntry(for MSIX Package) according to configuration file(json file)

this is an example configfile:

``` json
{
    "EnvironmentVariables": [
        {
            "Variable": "_zz1",
            "Value": "%SystemRoot%;;;xxx1"
        },
        {
            "Variable": "_zz2",
            "Value": "xxx2"
        }
    ],
    "LaunchApps": [
        {
            "Type": "process",
            "AppPath": "%SystemRoot%\\system32\\cmd.exe",
            "WorkingDirectory": "%SystemRoot%",
            "CommandLine": "\"%SystemRoot%\\system32\\cmd.exe\" /k set",
            "EnvironmentVariables": [
                {
                    "Variable": "__zzz1",
                    "Value": "%LauncherDir%;zzz1"
                },
                {
                    "Variable": "__zzz2",
                    "Value": "%LauncherDir%;zzz2"
                }
            ],
            "Wait": true
        },
        {
            "Type": "process",
            "AppPath": "%SystemRoot%\\system32\\taskmgr.exe",
            "WorkingDirectory": "%SystemRoot%",
            "CommandLine": "\"%SystemRoot%\\system32\\taskmgr.exe\" /4",
            "EnvironmentVariables": [
                {
                    "Variable": "__COMPAT_LAYER",
                    "Value": "RUNASINVOKER"
                }
            ],
            "Wait": true
        },
        {
            "Type": "AppListEntry",
            "Id": "Windowsist.0Template_xns9gpnbns7qp!App"
        }
    ]
}
```
