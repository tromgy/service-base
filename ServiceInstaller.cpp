/****************************** Module Header ******************************\
* Module Name:  ServiceInstaller.cpp
* Project:      service-base
* Copyright (c) Microsoft Corporation.
* Copyright (c) Tromgy (tromgy@yahoo.com)
*
* The file implements functions that install and uninstall the service.
*
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL.
* All other rights reserved.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma region "Includes"
#include "ServiceInstaller.h"
#pragma endregion

//
//   FUNCTION: InstallService
//
//   PURPOSE: Install the current application as a service to the local
//   service control manager database.
//
//   PARAMETERS:
//   * pszServiceName - the name of the service to be installed
//   * pszDisplayName - the display name of the service
//   * dwStartType - the service start option. This parameter can be one of
//     the following values: SERVICE_AUTO_START, SERVICE_BOOT_START,
//     SERVICE_DEMAND_START, SERVICE_DISABLED, SERVICE_SYSTEM_START.
//   * pszDependencies - a pointer to a double null-terminated array of null-
//     separated names of services or load ordering groups that the system
//     must start before this service.
//   * pszAccount - the name of the account under which the service runs.
//   * pszPassword - the password to the account name.
//
//   NOTE: If the function fails to install the service, it prints the error
//   in the standard output stream for users to diagnose the problem.
//
void InstallService(PCWSTR pszServiceName,
                    PCWSTR pszDisplayName,
                    PCWSTR pszDescription,
                    PCWSTR pszParams,
                    DWORD dwStartType,
                    PCWSTR pszDependencies,
                    PCWSTR pszAccount,
                    PCWSTR pszPassword,
                    BOOL bRegisterWithEventLog,
                    DWORD dwNumMessageCategories,
                    PCWSTR pszMessageResourceFilePath
                   )
{
    wchar_t wszPath[MAX_PATH];
    wchar_t wszFullCommand[2 * MAX_PATH];
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;

    if (GetModuleFileName(NULL, wszPath, _countof(wszPath)) == 0)
    {
        wprintf(L"GetModuleFileName failed w/err 0x%08lx\n", GetLastError());
        goto Cleanup;
    }
    else
    {
        // Assemble full service command
        _snwprintf_s(wszFullCommand, _countof(wszFullCommand), _countof(wszFullCommand) - 1, L"\"%s\" %s", wszPath, pszParams);
    }

    // Open the local default service control manager database
    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT |
                                 SC_MANAGER_CREATE_SERVICE);
    if (schSCManager == NULL)
    {
        wprintf(L"OpenSCManager failed w/err 0x%08lx\n", GetLastError());
        goto Cleanup;
    }

    // Install the service into SCM by calling CreateService
    schService = CreateService(
                     schSCManager,                   // SCManager database
                     pszServiceName,                 // Name of service
                     pszDisplayName,                 // Name to display
                     SERVICE_ALL_ACCESS,             // Desired access
                     SERVICE_WIN32_OWN_PROCESS,      // Service type
                     dwStartType,                    // Service start type
                     SERVICE_ERROR_NORMAL,           // Error control type
                     wszFullCommand,                 // Service's command
                     NULL,                           // No load ordering group
                     NULL,                           // No tag identifier
                     pszDependencies,                // Dependencies
                     pszAccount,                     // Service running account
                     pszPassword                     // Password of the account
                 );
    if (schService == NULL)
    {
        wprintf(L"CreateService failed with error code: 0x%08lx\n", GetLastError());
        goto Cleanup;
    }
    else
    {
        // Add service description
        SERVICE_DESCRIPTION sd;
        wchar_t wszDesc[1024];

        wcsncpy_s(wszDesc, _countof(wszDesc), pszDescription, _TRUNCATE);

        sd.lpDescription = wszDesc;

        if (!ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &sd))
        {
            wprintf(L"Couldn't set service description with error code: 0x%08lx\n", GetLastError());
            goto Cleanup;
        }

        // Register service with event logger (optional)
        if (bRegisterWithEventLog)
        {
            // This assumes that the service executable contains
            // event message strings in its resources
            WCHAR wszKey[256];
            HKEY hResult;
            DWORD dwDisposition;
            LSTATUS status;
            DWORD dwFlags = 7;   // error | warning | information
            PCWSTR pszResourcePath = pszMessageResourceFilePath == NULL ? wszPath : pszMessageResourceFilePath;

            if (-1 == _snwprintf_s(wszKey, _countof(wszKey), _TRUNCATE, L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\%s", pszServiceName))
            {
                wprintf(L"Service name is too long. Cannot register service with Windows Event Log.\n");
                goto Cleanup;
            }

            status = RegCreateKeyEx(HKEY_LOCAL_MACHINE,  // Root key
                                    wszKey,                                   // Name of the key to create
                                    0,                                        // Reserved
                                    const_cast<WCHAR*>(L""),                  // No class
                                    REG_OPTION_NON_VOLATILE,                  // Persistent key
                                    KEY_ALL_ACCESS,                           // Can do what we need with it
                                    NULL,                                     // No security attributes
                                    &hResult,                                 // Handle to the created/opened key
                                    &dwDisposition);                          // Created new or opened existing?

            if (status != ERROR_SUCCESS)
            {
                wprintf(L"Cannot register service with Windows Event Log. Error creating key: %d\n", status);
                goto Cleanup;
            }

            // Now add values to this key

            // Full path to the executable that should contain
            // message strings
            status = RegSetValueEx(hResult,
                                   L"EventMessageFile",
                                   0,
                                   REG_SZ,
                                   (BYTE*)pszResourcePath,
                                   (wcslen(pszResourcePath) + 1) * sizeof(WCHAR));

            if (status != ERROR_SUCCESS)
            {
                wprintf(L"Cannot register service with Windows Event Log. Error creating value \"EventMessageFile\": %d\n", status);

                RegCloseKey(hResult);

                goto Cleanup;
            }

            // Event types supported
            status = RegSetValueEx(hResult,
                                   L"TypesSupported",
                                   0,
                                   REG_DWORD,
                                   (BYTE *)&dwFlags,
                                   sizeof(DWORD));

            if (status != ERROR_SUCCESS)
            {
                wprintf(L"Cannot register service with Windows Event Log. Error creating value \"TypesSupported\": %d\n", status);

                RegCloseKey(hResult);

                goto Cleanup;
            }

            // Message categories (optional)
            if (dwNumMessageCategories > 0)
            {
                status = RegSetValueEx(hResult,
                                       L"CategoryMessageFile",
                                       0,
                                       REG_SZ,
                                       (BYTE*)pszResourcePath,
                                       (wcslen(pszResourcePath) + 1) * sizeof(WCHAR));

                if (status != ERROR_SUCCESS)
                {
                    wprintf(L"Cannot register service with Windows Event Log. Error creating value \"CategoryMessageFile\": %d\n", status);

                    RegCloseKey(hResult);

                    goto Cleanup;
                }

                status = RegSetValueEx(hResult,
                                       L"CategoryCount",
                                       0,
                                       REG_DWORD,
                                       (BYTE *)&dwNumMessageCategories,
                                       sizeof(DWORD));

                if (status != ERROR_SUCCESS)
                {
                    wprintf(L"Cannot register service with Windows Event Log. Error creating value \"CategoryCount\": %d\n", status);

                    RegCloseKey(hResult);

                    goto Cleanup;
                }
            }

            RegCloseKey(hResult);
        }

        wprintf(L"%s is installed.\n", pszServiceName);

    Cleanup:
        // Centralized cleanup for all allocated resources.
        if (schSCManager)
        {
            CloseServiceHandle(schSCManager);
            schSCManager = NULL;
        }
        if (schService)
        {
            CloseServiceHandle(schService);
            schService = NULL;
        }
    }
}

//
//   FUNCTION: UninstallService
//
//   PURPOSE: Stop and remove the service from the local service control
//   manager database.
//
//   PARAMETERS:
//   * pszServiceName - the name of the service to be removed.
//
//   NOTE: If the function fails to uninstall the service, it prints the
//   error in the standard output stream for users to diagnose the problem.
//
void UninstallService(PCWSTR pszServiceName)
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    SERVICE_STATUS ssSvcStatus = {};

    // Open the local default service control manager database
    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (schSCManager == NULL)
    {
        wprintf(L"OpenSCManager failed w/err 0x%08lx\n", GetLastError());
        goto Cleanup;
    }

    // Open the service with delete, stop, and query status permissions
    schService = OpenService(schSCManager, pszServiceName, SERVICE_STOP |
                             SERVICE_QUERY_STATUS | DELETE);
    if (schService == NULL)
    {
        wprintf(L"OpenService failed w/err 0x%08lx\n", GetLastError());
        goto Cleanup;
    }

    // Try to stop the service
    if (ControlService(schService, SERVICE_CONTROL_STOP, &ssSvcStatus))
    {
        wprintf(L"Stopping %s.", pszServiceName);
        Sleep(1000);

        while (QueryServiceStatus(schService, &ssSvcStatus))
        {
            if (ssSvcStatus.dwCurrentState == SERVICE_STOP_PENDING)
            {
                wprintf(L".");
                Sleep(1000);
            }
            else break;
        }

        if (ssSvcStatus.dwCurrentState == SERVICE_STOPPED)
        {
            wprintf(L"\n%s is stopped.\n", pszServiceName);
        }
        else
        {
            wprintf(L"\n%s failed to stop.\n", pszServiceName);
        }
    }

    // Now remove the service by calling DeleteService.
    if (!DeleteService(schService))
    {
        wprintf(L"DeleteService failed w/err 0x%08lx\n", GetLastError());
        goto Cleanup;
    }

    // Clean the Windows Event Log registration (if any)
    WCHAR wszKey[256];
    LSTATUS status;

    if (-1 == _snwprintf_s(wszKey, _countof(wszKey), _TRUNCATE, L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\%s", pszServiceName))
    {
        wprintf(L"Service name is too long. Cannot register service with Windows Event Log.\n");
        goto Cleanup;
    }

    status = RegDeleteKey(HKEY_LOCAL_MACHINE,    // Root key
                          wszKey                 // Name of the key to delete
                         );

    if (status != ERROR_SUCCESS)
    {
        wprintf(L"Service cannot be unregistered with Windows Event Log. Error deleting key: %d", status);
    }

    wprintf(L"%s is removed.\n", pszServiceName);

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (schSCManager)
    {
        CloseServiceHandle(schSCManager);
        schSCManager = NULL;
    }
    if (schService)
    {
        CloseServiceHandle(schService);
        schService = NULL;
    }
}