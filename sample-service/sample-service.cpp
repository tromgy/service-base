/****************************** Module Header ******************************\
* Module Name:  sample-service.cpp
* Project:      sample-service
* Copyright (c) Microsoft Corporation.
* Copyright (c) Tromgy (tromgy@yahoo.com)
*
* Provides a sample service class that derives from the service base class -
* CServiceBase. The sample service logs the service start and stop
* information to the Application event log, and shows how to run the main
* function of the service in a thread pool worker thread.
*
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL.
* All other rights reserved.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/


// sample-service.cpp : Defines the entry point for the console application.
//

#pragma region "Includes"
#include "stdafx.h"
#include <regex>
#include "SampleService.h"
#include <ServiceInstaller.h>
#pragma endregion

using namespace std;

int wmain(int argc, wchar_t *argv[])
{
   // Service parameters
   DWORD dwSvcStartType = SERVICE_START_TYPE;
   PCWSTR wsSvcAccount = SERVICE_ACCOUNT;
   PCWSTR wsSvcPwd = SERVICE_PASSWORD;
   PCWSTR wsConfigFullPath = SERVICE_CONFIG_FILE;
   WCHAR wsServiceParams[MAX_PATH] = SERVICE_CMD;

   if (argc > 1)
   {
      if (_wcsicmp(L"install", argv[1]) == 0)
      {
         try
         {
            for (int i = 2; i < argc; i++)
            {
               PWSTR arg = argv[i];

               if (arg[0] == '-')
               {
                  if (_wcsicmp(L"-start-type", arg) == 0)
                  {
                     if (argc > i)
                     {
                        PCWSTR wsStartType = argv[++i];

                        if (regex_match(wsStartType, wregex(L"[2-4]")))
                        {
                           dwSvcStartType = _wtol(wsStartType);
                        }
                        else
                        {
                           throw exception("service start type must be a number between 2 and 4");
                        }
                     }
                     else
                     {
                        throw exception("no startup type specified after -start-type");
                     }
                  }
                  else if (_wcsicmp(L"-account", arg) == 0)
                  {
                     if (argc > i)
                     {
                        wsSvcAccount = argv[++i];
                     }
                     else
                     {
                        throw exception("no account name after -account");
                     }
                  }
                  else if (_wcsicmp(L"-password", arg) == 0)
                  {
                     if (argc > i)
                     {
                        wsSvcPwd = argv[++i];
                     }
                     else
                     {
                        throw exception("no password  after -password");
                     }
                  }
                  else if (_wcsicmp(L"-config", arg) == 0)
                  {
                     if (argc > i)
                     {
                        _snwprintf_s(wsServiceParams, _countof(wsServiceParams), _TRUNCATE, L"%s -config \"%s\"", SERVICE_CMD, argv[++i]);
                     }
                     else
                     {
                        throw exception("no configuration file specified");
                     }
                  }
                  else
                  {
                     char errMsg[MAX_PATH];
                     _snprintf_s(errMsg, _countof(errMsg), _TRUNCATE, "unknown parameter: %S", arg);

                     throw exception(errMsg);
                  }
               }
            }

            InstallService(
               SERVICE_NAME,               // Name of service
               SERVICE_DISP_NAME,          // Display name
               SERVICE_DESC,               // Description
               wsServiceParams,            // Command-line parameters to pass to the service
               dwSvcStartType,             // Service start type
               SERVICE_DEPENDENCIES,       // Dependencies
               wsSvcAccount,               // Service running account
               wsSvcPwd,                   // Password of the account
               TRUE,                       // Register with Windows Event Log, so our log messages will be found in Event Viewer
               1,                          // We have only one event category, "Service"
               NULL                        // No separate resource file, use resources in main executable for messages (default)
            );
         }
         catch (exception const& ex)
         {
            wprintf(L"Couldn't install service: %S", ex.what());
            return 1;
         }
         catch (...)
         {
            wprintf(L"Couldn't install service: unexpected error");
            return 2;
         }
      }
      else if (_wcsicmp(L"uninstall", argv[1]) == 0)
      {
         UninstallService(SERVICE_NAME);
      }
      else if (_wcsicmp(SERVICE_CMD, argv[1]) == 0)
      {
         CSampleService service(SERVICE_NAME);

         service.SetCommandLine(argc, argv);

         if (!CServiceBase::Run(service))
         {
            DWORD dwErr = GetLastError();

            wprintf(L"Service failed to run with error code: 0x%08lx\n", dwErr);

            return dwErr;
         }
      }
      else if (_wcsicmp(PROCESS_CMD, argv[1]) == 0)
      {
         CSampleService service(SERVICE_NAME);

         service.SetCommandLine(argc, argv);

         service.Run();
      }
      else
      {
         wprintf(L"Unknown parameter: %s\n", argv[1]);
      }
   }
   else
   {
      wprintf(L"\nSample Windows Service\n\n");
      wprintf(L"Parameters:\n\n");
      wprintf(L" install [-start-type <2..4> -account <account-name> -password <account-password> -config <configuration-file-path>]\n  - to install the service.\n");
      wprintf(L"    service start types are:\n");
      wprintf(L"     2 - service started automatically by the service control manager during system startup.\n");
      wprintf(L"     3 - service started manually or by calling StartService function from another process.\n");
      wprintf(L"     4 - service installed in the \"disabled\" state, and cannot be started until enabled.\n");
      wprintf(L" run [-config <configuration-file-path>]\n  - to start as a regular process (not a service)\n");
      wprintf(L" uninstall\n  - to remove the service.\n");
   }

   return 0;
}


