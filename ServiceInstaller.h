/****************************** Module Header ******************************\
* Module Name:  ServiceInstaller.h
* Project:      service-base
* Copyright (c) Microsoft Corporation.
* Copyright (c) Tromgy (tromgy@yahoo.com)
*
* The file declares functions that install and uninstall the service.
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

#include <stdio.h>
#include <windows.h>

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
//   * bRegisterWithEventLog - set this to TRUE if the service is intended to
//     write log messages into Windows Application log (default).
//   * dwNumMessageCategories - number of message categories that will be logged.
//   * pszMessageResourceFilePath - the full path to a file containing message
//     string resources to be displayed for event ids and message categories.
//     if NULL and bRegisterWithEventLog is TRUE, it expected that
//     the main service executable contains the message string resources.
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
                    BOOL bRegisterWithEventLog = TRUE,
                    DWORD dwNumMessageCategories = 0,
                    PCWSTR pszMessageResourceFilePath = NULL
                   );

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
void UninstallService(PCWSTR pszServiceName);