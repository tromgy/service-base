/****************************** Module Header ******************************\
* Module Name:  SampleService.cpp
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


#include "stdafx.h"
#include "SampleService.h"
#include "event_ids.h"


CSampleService::CSampleService(PCWSTR pszServiceName,
                               BOOL fCanStop,
                               BOOL fCanShutdown,
                               BOOL fCanPauseContinue) :
    CServiceBase(pszServiceName, fCanStop, fCanShutdown, fCanPauseContinue, MSG_SVC_FAILURE, CATEGORY_SERVICE)
{
    m_bIsStopping = FALSE;

    m_hHasStoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (m_hHasStoppedEvent == NULL)
    {
        throw GetLastError();
    }
}

void CSampleService::OnStart(DWORD /* useleses */, PWSTR* /* useless */)
{
    const wchar_t* wsConfigFullPath = SERVICE_CONFIG_FILE;
    bool bRunAsService = true;

    // Log a service start message to the Application log.
    WriteLogEntry(L"Sample Service is starting...", EVENTLOG_INFORMATION_TYPE, MSG_STARTUP, CATEGORY_SERVICE);

    if (m_argc > 1)
    {
        bRunAsService = (_wcsicmp(SERVICE_CMD, m_argv[1]) == 0);

        // Check if the config file was specified on the service command line
        if (m_argc > 2) // the argument at 1 should be "run mode", so we start at 2
        {
            if (_wcsicmp(L"-config", m_argv[2]) == 0)
            {
                if (m_argc > 3)
                {
                    wsConfigFullPath = m_argv[3];
                }
                else
                {
                    throw exception("no configuration file name");
                }
            }
        }
    }
    else
    {
        WriteLogEntry(L"Sample Service:\nNo run mode specified.", EVENTLOG_ERROR_TYPE, MSG_STARTUP, CATEGORY_SERVICE);
        throw exception("no run mode specified");
    }

    try
    {
        // Here we would load configuration file
        // but instead we're just writing to event log the configuration file name
        wstring infoMsg = L"Sample Service\n The service is pretending to read configuration from ";
        infoMsg += wsConfigFullPath;
        WriteLogEntry(infoMsg.c_str(), EVENTLOG_INFORMATION_TYPE, MSG_STARTUP, CATEGORY_SERVICE);
    }
    catch (exception const& e)
    {
        WCHAR wszMsg[MAX_PATH];

        _snwprintf_s(wszMsg, _countof(wszMsg), _TRUNCATE, L"Sample Service\nError reading configuration %S", e.what());

        WriteLogEntry(wszMsg, EVENTLOG_ERROR_TYPE, MSG_STARTUP, CATEGORY_SERVICE);
    }

    if (bRunAsService)
    {
        WriteLogEntry(L"Sample Service will run as a service.", EVENTLOG_INFORMATION_TYPE, MSG_STARTUP, CATEGORY_SERVICE);

        // Add the main service function for execution in a worker thread.
        if (!CreateThread(NULL, 0, ServiceRunner, this, 0, NULL))
        {
            WriteLogEntry(L"Sample Service couldn't create worker thread.", EVENTLOG_ERROR_TYPE, MSG_STARTUP, CATEGORY_SERVICE);
        }
    }
    else
    {
        wprintf(L"Sample Service is running as a regular process.\n");

        CSampleService::ServiceRunner(this);
    }
}

CSampleService::~CSampleService()
{
}

void CSampleService::Run()
{
    OnStart(0, NULL);
}

DWORD __stdcall CSampleService::ServiceRunner(void* self)
{
    CSampleService* pService = (CSampleService*)self;

    pService->WriteLogEntry(L"Sample Service has started.", EVENTLOG_INFORMATION_TYPE, MSG_STARTUP, CATEGORY_SERVICE);

    // Periodically check if the service is stopping.
    for (bool once = true; !pService->m_bIsStopping; once = false)
    {
        if (once)
        {
            // Log multi-line message
            pService->WriteLogEntry(L"Sample Service is pretending to be working:\nStarting fake job 1...\nStarting fake job 2...\nStarting fake job 3...", EVENTLOG_INFORMATION_TYPE, MSG_OPERATION, CATEGORY_SERVICE);
        }

        // Just pretend to do some work
        Sleep(5000);
    }

    // Signal the stopped event.
    SetEvent(pService->m_hHasStoppedEvent);
    pService->WriteLogEntry(L"Sample Service has stopped.", EVENTLOG_INFORMATION_TYPE, MSG_SHUTDOWN, CATEGORY_SERVICE);

    return 0;
}

void CSampleService::OnStop()
{
    // Log a service stop message to the Application log.
    WriteLogEntry(L"Sample Service is stopping", EVENTLOG_INFORMATION_TYPE, MSG_SHUTDOWN, CATEGORY_SERVICE);

    // Indicate that the service is stopping and wait for the finish of the
    // main service function (ServiceWorkerThread).
    m_bIsStopping = TRUE;

    if (WaitForSingleObject(m_hHasStoppedEvent, INFINITE) != WAIT_OBJECT_0)
    {
        throw GetLastError();
    }
}
