/****************************** Module Header ******************************\
* Module Name:  SampleService.h
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

#pragma once

#include <ServiceBase.h>
#include <string>

// Default service start options.
#define SERVICE_START_TYPE       SERVICE_AUTO_START

// List of service dependencies (none)
#define SERVICE_DEPENDENCIES     L""

// Default name of the account under which the service should run
#define SERVICE_ACCOUNT          L"NT AUTHORITY\\LocalService"

// Default password to the service account name
#define SERVICE_PASSWORD         NULL

// Configuration file
#define SERVICE_CONFIG_FILE      L"config.cfg"

// Command to run as a service
#define SERVICE_CMD              L"serve"

// Command to run as a stand-alone process
#define PROCESS_CMD              L"run"

// Service name
#define SERVICE_NAME             L"sample-service"

// Service name as displayed in MMC
#define SERVICE_DISP_NAME        L"Sample service"

// Service description as displayed in MMC
#define SERVICE_DESC             L"This is a sample service written in C++ using a class dervied from CServiceBase."

using namespace std;

class CSampleService: public CServiceBase
{
  public:
    CSampleService(PCWSTR pszServiceName,
                   BOOL fCanStop = TRUE,
                   BOOL fCanShutdown = TRUE,
                   BOOL fCanPauseContinue = FALSE
                  );
    ~CSampleService();

    virtual void OnStart(DWORD dwArgc, PWSTR *pszArgv);

    virtual void OnStop();

    static DWORD __stdcall  ServiceRunner(void* self);

    void Run();

  private:
    BOOL m_bIsStopping;
    HANDLE m_hHasStoppedEvent;
    wstring m_wstrParam;
};

