[![Build status](https://ci.appveyor.com/api/projects/status/k197mtn89ylpjyp7?svg=true)](https://ci.appveyor.com/project/tromgy/service-base)

# C++ class and library to implement a Windows service

This project provides a C++ base class with minimal dependencies that you can derive from to create a Microsoft Windows service executable.

It also includes a complete service example that utilizes this class.

## Background

Years ago (circa 2008?) Microsoft published the original C++ class and a sample service project that was once available at

https://code.msdn.microsoft.com/windowsapps/CppWindowsService-cacf4948

This project is an attempt to improve on that original example in the areas of security, usability, and convenience, as well as trying to cross the "t"s and dot the "i"s.

## Improvements

- The `InstallService` function in **ServiceInstaller.cpp** encloses the service executable path in quotes to avoid a common security vulnerability (https://www.commonexploits.com/unquoted-service-paths/)

- The same function also allows to set the command line parameters to the service executable when it runs as a service.

- The service installation includes the service description argument, which the original installer was lacking.

- The `CServiceBase` class now provides support for logging into Windows Event Log the _proper_ way, with event and category ids. You will no longer see "Either the component that raises this event is not installed on your local computer or the installation is corrupted." in the Event Viewer.

- You can also override the logging function to log into your own log file instead of Windows Event Log.

## Creating the service

The project includes a sample service in the **sample-service** subdirectory.

To create the service follow these steps:

1. Set up a Visual C++ project statically linking with **service-base.lib**
2. Derive a class from `CServiceBase` like this
```
    class CSampleService: public CServiceBase
```
Override (implement with your service-specific details) virtual methods:
- `OnStart`
- `OnStop`
- `OnPause` (sample service does not implement this one)

Create a static method where the actual service work will be performed (on a separate thread). In sample service it is done
in `static DWORD __stdcall  ServiceRunner(void* self)`.

This static method takes a pointer to the service instance in the `self` argument.

In addition our sample service provides an option to run it as a regular (non-service) process by providing a command line argument called **run** (while to run as a service it takes the **serve** argument).

## Command line

The sample service demonstrates how to implement a rich command line interface.

Command line parameters are used for installing/uninstalling the service as well as at the service run time. The original Microsoft example did not have an ability to install the service with command line parameters to be used at run time.

In our case the service "uses" a configuration file at run time, so when installing the service we can use the following command:
```
    c:\github\service-base\sample-service\Release>sample-service install -config c:\Users\Me\my-config.cfg
```
This will install the service to run as:

**"c:\github\service-base\sample-service\Release\sample-service.exe" serve -config "c:\Users\Me\my-config.cfg"**

It should be noted that these command line parameters are _not_ getting passed to the service in the `OnStart` method, contrary to the method signature. Instead, we have to store number of arguments and their array (`m_argc` and `m_argv`) with the class instance, and use them from the same `OnStart` method.

These class members are set by calling `service.SetCommandLine(argc, argv)` method from the `wmain` function.

## Logging

Sample service also demonstrates correct way of logging information into the Windows Event Log. The `CServiceBase::WriteLogEntry` method takes the following parameters:

- `pszMessage`: the message(s) to be displayed on the **Details** page of the logged event when viewed with Windows Event Viewer. If you wish to display multi-line message, just separate lines with `\n` in this string.
- `wType`: type of the message (information, warning, error, etc.) -- these are defined in system SDK's **winnt.h** file.
- `dwEventId`: numerical event id matching the string to be displayed on the **General** page in Event Viewer. These strings should come from resources in your service executable or a separate resource DLL (more details below).
- `wCategory`: numerical category id matching the string to be displayed in the **Task Category** column in Event Viewer.

The last two arguments are optional, but lacking event id the logged event would look ugly in the Event Viewer with the following text appearing on the **General** page:

    The description for Event ID 0 from source <your-service> cannot be found. Either the component that raises this event is not installed on your local computer or the installation is corrupted. You can install or repair the component on the local computer.

To allow the same logging options from _within_ the `CServiceBase` class, the constructor was extended to take two optional arguments: `dwErrorEventId` and `wErrorCategoryId`. It is expected (if these arguments are specified when creating the service instance) that strings matching these ids can be found in the resources. These strings will be used only for error reporting (the base class only writes logging messages if something goes wrong).

To provide strings for event and category ids we need to have a two-step process for building the string resources:

1. Run _message compiler_ (**mc.exe**) on the message strings (.mc) file. That generates resources (.rc) file, corresponding header file to be included in your code, and the binary (.bin) file containing the actual strings for the events and categories.

2. Run resource compiler to generate .res file.

The resulting .res file is then linked with your executable as any other resource file.

Each message string entry in the .mc file has the following format (this is not a complete description):

```
MessageId=10
SymbolicName=MSG_STARTUP
Severity=Informational
Facility=Application
Language=English
Startup
.
```
The `MessageId` then becomes event or category id to be passed as its `SymbolicName` to the `WriteEventLog` method. It will be defined as
```
#define MSG_STARTUP                      ((DWORD)0x4000000AL)
```
in the generated header file.

The `Language` value comes from the language defintion in the same message file:

```
LanguageNames=(EnglishUS=0x409:MSG00409)
```

Language ids can be found at https://docs.microsoft.com/en-us/windows-hardware/manufacture/desktop/default-input-locales-for-windows-language-packs

so for instance to implement message strings also in Faroese you could use:
```
LanguageNames=(
EnglishUS=0x409:MSG00409
Faroese=0438:MSG00406
)
```
and then refer to that language in your message entry.

The actual text that will be displayed on the **General** page goes after the `Language` line and before the line containing the period (`.`), which is in the example above would be **Startup**.

Category names can be created similarly:

```
MessageId=1
SymbolicName=CATEGORY_SERVICE
Language=EnglishUS
Service
.
```

Our sample service contains only one category, called **Service**.

Once we have the resources in the main executable or a separate resource DLL, these need to be registered when the service is installed. The `InstallService` function takes care of that. The last three (optional) arguments are:

- `bRegisterWithEventLog`: pass `TRUE` to create Registry entries so that Event Viewer can display your log messages properly.
- `dwNumMessageCategories`: number of message categores (which were described above).
- `pszMessageResourceFilePath`: full path to the file containing message string resources. If this parameter is `NULL`, and
`bRegisterWithEventLog` is `TRUE`, the service executable path is used. So you only need to supply this parameter if you have you message strings in a separate resource DLL.

The registration information is stored in the following key:

**HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\EventLog\Application\sample-service**

Finally, if you do not wish to utilize Windows Event Log system, you can completely override `CServiceBase::WriteLogEntry` method to log into your own file for example.

## Building

The **master** branch now contains the solutions and projects for MS Visual Studio 2022. The **VS2019** and **VS2017** branches contain the same for MS Visual Studio 2019 and MS Visual Studio 2017 accordingly.
