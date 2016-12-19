#pragma once
#include <stdio.h>
#include <windows.h>
//
//   FUNCTION: InstallService
//
void InstallService(PWSTR pszServiceName,
	PWSTR pszDisplayName,
	DWORD dwStartType,
	PWSTR pszDependencies,
	PWSTR pszAccount,
	PWSTR pszPassword);

//
//   FUNCTION: DoStart
//
void DoStart(PWSTR pszServiceName);

//
//   FUNCTION: UninstallService
//
void UninstallService(PWSTR pszServiceName);