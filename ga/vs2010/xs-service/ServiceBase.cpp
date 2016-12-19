#include "ServiceBase.h"
#include <assert.h>
#include <strsafe.h>

#include <wtsapi32.h>
#pragma comment(lib, "wtsapi32.lib")
#include <locale>
#include <userenv.h>
#pragma comment(lib, "userenv.lib")

#include <wtypes.h>
#pragma comment(lib, "ws2_32.lib")

#include "Prgram.h"
#include "ThreadPool.h"

DWORD GetSessionIdOfUser(PCWSTR, PCWSTR);
BOOL DisplayInteractiveMessage(DWORD, PWSTR, PWSTR, DWORD, BOOL, DWORD, DWORD *);
BOOL CreateInteractiveProcess(DWORD, PWSTR, BOOL, DWORD, DWORD *);

#pragma region Static Members

// Initialize the singleton service instance.
CServiceBase *CServiceBase::s_service = NULL;

BOOL CServiceBase::Run(CServiceBase &service)
{
	s_service = &service;

	SERVICE_TABLE_ENTRY serviceTable[] =
	{
		{ service.m_name, ServiceMain },
		{ NULL, NULL }
	};

	return StartServiceCtrlDispatcher(serviceTable);
}


void WINAPI CServiceBase::ServiceMain(DWORD dwArgc, PWSTR *pszArgv)
{
	assert(s_service != NULL);

	// Register the handler function for the service
	s_service->m_statusHandle = RegisterServiceCtrlHandler(s_service->m_name, ServiceCtrlHandler);
	if (s_service->m_statusHandle == NULL)
	{
		throw GetLastError();
	}

	// Start the service.
	s_service->Start(dwArgc, pszArgv);

	//ServerStart();
}

void WINAPI CServiceBase::ServiceCtrlHandler(DWORD dwCtrl)
{
	switch (dwCtrl)
	{
	case SERVICE_CONTROL_STOP: s_service->Stop(); break;
	case SERVICE_CONTROL_PAUSE: s_service->Pause(); break;
	case SERVICE_CONTROL_CONTINUE: s_service->Continue(); break;
	case SERVICE_CONTROL_SHUTDOWN: s_service->Shutdown(); break;
	case SERVICE_CONTROL_INTERROGATE: break;
	default: break;
	}
}

#pragma endregion


#pragma region Service Constructor and Destructor


CServiceBase::CServiceBase(PWSTR pszServiceName,
	BOOL fCanStop,
	BOOL fCanShutdown,
	BOOL fCanPauseContinue)
{
	// Service name must be a valid string and cannot be NULL.
	m_name = (pszServiceName == NULL) ? L"" : pszServiceName;

	m_statusHandle = NULL;

	// The service runs in its own process.
	m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

	// The service is starting.
	m_status.dwCurrentState = SERVICE_START_PENDING;

	// The accepted commands of the service.
	DWORD dwControlsAccepted = 0;
	if (fCanStop)
		dwControlsAccepted |= SERVICE_ACCEPT_STOP;
	if (fCanShutdown)
		dwControlsAccepted |= SERVICE_ACCEPT_SHUTDOWN;
	if (fCanPauseContinue)
		dwControlsAccepted |= SERVICE_ACCEPT_PAUSE_CONTINUE;
	m_status.dwControlsAccepted = dwControlsAccepted;

	m_status.dwWin32ExitCode = NO_ERROR;
	m_status.dwServiceSpecificExitCode = 0;
	m_status.dwCheckPoint = 0;
	m_status.dwWaitHint = 0;
}

CServiceBase::~CServiceBase(void)
{
}

#pragma endregion


#pragma region Service Start, Stop, Pause, Continue, and Shutdown

void CServiceBase::Start(DWORD dwArgc, PWSTR *pszArgv)
{
	try
	{
		// Tell SCM that the service is starting.
		SetServiceStatus(SERVICE_START_PENDING);

		// Perform service-specific initialization.
		OnStart(dwArgc, pszArgv);

		// Tell SCM that the service is started.
		SetServiceStatus(SERVICE_RUNNING);
	}
	catch (DWORD dwError)
	{
		// Log the error.
		WriteErrorLogEntry(L"Service Start", dwError);

		// Set the service status to be stopped.
		SetServiceStatus(SERVICE_STOPPED, dwError);
	}
	catch (...)
	{
		// Log the error.
		WriteEventLogEntry(L"Service failed to start.", EVENTLOG_ERROR_TYPE);

		// Set the service status to be stopped.
		SetServiceStatus(SERVICE_STOPPED);
	}
}

/////////////////////
BOOL DisplayInteractiveMessage(DWORD dwSessionId, PWSTR pszTitle, PWSTR pszMessage, DWORD dwStyle, BOOL fWait, DWORD dwTimeoutSeconds, DWORD *pResponse)
{
	DWORD cbTitle = wcslen(pszTitle) * sizeof(*pszTitle);
	DWORD cbMessage = wcslen(pszMessage) * sizeof(*pszMessage);

	return WTSSendMessage(
		WTS_CURRENT_SERVER_HANDLE,  // The current server
		dwSessionId,                // Identify the session to display message
		pszTitle,                   // Title bar of the message box
		cbTitle,                    // Length, in bytes, of the title
		pszMessage,                 // Message to display
		cbMessage,                  // Length, in bytes, of the message
		dwStyle,                    // Contents and behavior of the message
		dwTimeoutSeconds,           // Timeout of the message in seconds
		pResponse,                  // Receive the user's response
		fWait                       // Whether wait for user's response or not
	);
}

DWORD GetSessionIdOfUser(PCWSTR pszUserName, PCWSTR pszDomain)
{
	DWORD dwSessionId = 0xFFFFFFFF;

	if (pszUserName == NULL)
	{
		dwSessionId = WTSGetActiveConsoleSessionId();
	}
	else
	{
		PWTS_SESSION_INFO *pSessionsBuffer = NULL;
		DWORD dwSessionCount = 0;

		// Enumerate the sessions on the current server.
		if (WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1,
			pSessionsBuffer, &dwSessionCount))
		{
			for (DWORD i = 0; (dwSessionId == -1) && (i < dwSessionCount); i++)
			{
				DWORD sid = pSessionsBuffer[i]->SessionId;

				// Get the user name from the session ID.
				PWSTR pszSessionUserName = NULL;
				DWORD dwSize;
				if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, sid,
					WTSUserName, &pszSessionUserName, &dwSize))
				{
					// Compare with the provided user name (case insensitive).
					if (_wcsicmp(pszUserName, pszSessionUserName) == 0)
					{
						// Get the domain from the session ID.
						PWSTR pszSessionDomain = NULL;
						if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
							sid, WTSDomainName, &pszSessionDomain, &dwSize))
						{
							// Compare with the provided domain (case insensitive).
							if (_wcsicmp(pszDomain, pszSessionDomain) == 0)
							{
								// The session of the provided user is found.
								dwSessionId = sid;
							}
							WTSFreeMemory(pszSessionDomain);
						}
					}
					WTSFreeMemory(pszSessionUserName);
				}
			}

			WTSFreeMemory(pSessionsBuffer);
			pSessionsBuffer = NULL;
			dwSessionCount = 0;

			// Cannot find the session of the provided user.
			if (dwSessionId == 0xFFFFFFFF)
			{
				SetLastError(ERROR_INVALID_PARAMETER);
			}
		}
	}

	return dwSessionId;
}

BOOL CreateInteractiveProcess(DWORD dwSessionId, PWSTR pszCommandLine, BOOL fWait, DWORD dwTimeout, DWORD *pExitCode)
{
	DWORD dwError = ERROR_SUCCESS;
	HANDLE hToken = NULL;
	LPVOID lpvEnv = NULL;
	wchar_t szUserProfileDir[MAX_PATH];
	DWORD cchUserProfileDir = ARRAYSIZE(szUserProfileDir);
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi = { 0 };
	DWORD dwWaitResult;

	// Obtain the primary access token of the logged-on user specified by the 
	// session ID.
	if (!WTSQueryUserToken(dwSessionId, &hToken))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	// This creates the default environment block for the user.
	if (!CreateEnvironmentBlock(&lpvEnv, hToken, TRUE))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	// Retrieve the path to the root directory of the user's profile.
	if (!GetUserProfileDirectory(hToken, szUserProfileDir,
		&cchUserProfileDir))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	// Specify that the process runs in the interactive desktop.
	si.lpDesktop = L"winsta0\\default";

	// Launch the process.
	if (!CreateProcessAsUser(hToken, NULL, pszCommandLine, NULL, NULL, FALSE,
		CREATE_UNICODE_ENVIRONMENT, lpvEnv, szUserProfileDir, &si, &pi))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	if (fWait)
	{
		// Wait for the exit of the process.
		dwWaitResult = WaitForSingleObject(pi.hProcess, dwTimeout);
		if (dwWaitResult == WAIT_OBJECT_0)
		{
			// If the process exits before timeout, get the exit code.
			GetExitCodeProcess(pi.hProcess, pExitCode);
		}
		else if (dwWaitResult == WAIT_TIMEOUT)
		{
			// If it times out, TerminateProcess
			TerminateProcess(pi.hProcess, IDTIMEOUT);
			*pExitCode = IDTIMEOUT;
		}
		else
		{
			dwError = GetLastError();
			goto Cleanup;
		}
	}
	else
	{
		*pExitCode = IDASYNC;
	}

Cleanup:

	// Centralized cleanup for all allocated resources.
	if (hToken)
	{
		CloseHandle(hToken);
		hToken = NULL;
	}
	if (lpvEnv)
	{
		DestroyEnvironmentBlock(lpvEnv);
		lpvEnv = NULL;
	}
	if (pi.hProcess)
	{
		CloseHandle(pi.hProcess);
		pi.hProcess = NULL;
	}
	if (pi.hThread)
	{
		CloseHandle(pi.hThread);
		pi.hThread = NULL;
	}

	// Set the last error if something failed in the function.
	if (dwError != ERROR_SUCCESS)
	{
		SetLastError(dwError);
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


// Test
void CServiceBase::InteractiveServiceWorkerThread(void)
{
	// Get the ID of the session attached to the physical console.
	DWORD dwSessionId = GetSessionIdOfUser(NULL, NULL);
	if (dwSessionId == 0xFFFFFFFF)
	{
		// Log the error and exit.
		WriteErrorLogEntry(L"GetSessionIdOfUser", GetLastError());
		return;
	}

	// Display an interactive message in the session.
	wchar_t szTitle[] = L"XSINC Cache Service";
	wchar_t szMessage[] = L"Do you want to start Notepad?";
	DWORD dwResponse;

	if (!DisplayInteractiveMessage(dwSessionId, szTitle, szMessage, MB_YESNO, TRUE, 5, &dwResponse))
	{
		// Log the error and exit.
		WriteErrorLogEntry(L"DisplayInteractiveMessage", GetLastError());
		return;
	}

	if (IDYES == dwResponse) // If the user choose 'Yes'
	{
		// Launch notepad.
		wchar_t szCommandLine[] = L"notepad.exe ";
		DWORD dwExitCode;
		if (!CreateInteractiveProcess(dwSessionId, szCommandLine, FALSE, 0, &dwExitCode))
		{
			// Log the error and exit.
			WriteErrorLogEntry(L"CreateInteractiveProcess", GetLastError());
			return;
		}
	}
}

////////////////////////

void CServiceBase::GAWStart() {
	ServerStart();
}

void CServiceBase::OnStart(DWORD dwArgc, PWSTR *pszArgv)
{
	//ServerStart();
	//CThreadPool::QueueUserWorkItem(&CServiceBase::InteractiveServiceWorkerThread, this);
}

void CServiceBase::Stop()
{
	DWORD dwOriginalState = m_status.dwCurrentState;
	try
	{
		// Tell SCM that the service is stopping.
		SetServiceStatus(SERVICE_STOP_PENDING);

		// Perform service-specific stop operations.
		OnStop();

		// Tell SCM that the service is stopped.
		SetServiceStatus(SERVICE_STOPPED);
	}
	catch (DWORD dwError)
	{
		// Log the error.
		WriteErrorLogEntry(L"Service Stop", dwError);

		// Set the orginal service status.
		SetServiceStatus(dwOriginalState);
	}
	catch (...)
	{
		// Log the error.
		WriteEventLogEntry(L"Service failed to stop.", EVENTLOG_ERROR_TYPE);

		// Set the orginal service status.
		SetServiceStatus(dwOriginalState);
	}
}

//
void CServiceBase::OnStop()
{
}

void CServiceBase::Pause()
{
	try
	{
		// Tell SCM that the service is pausing.
		SetServiceStatus(SERVICE_PAUSE_PENDING);

		// Perform service-specific pause operations.
		OnPause();

		// Tell SCM that the service is paused.
		SetServiceStatus(SERVICE_PAUSED);
	}
	catch (DWORD dwError)
	{
		// Log the error.
		WriteErrorLogEntry(L"Service Pause", dwError);

		// Tell SCM that the service is still running.
		SetServiceStatus(SERVICE_RUNNING);
	}
	catch (...)
	{
		// Log the error.
		WriteEventLogEntry(L"Service failed to pause.", EVENTLOG_ERROR_TYPE);

		// Tell SCM that the service is still running.
		SetServiceStatus(SERVICE_RUNNING);
	}
}


void CServiceBase::OnPause()
{
}

void CServiceBase::Continue()
{
	try
	{
		// Tell SCM that the service is resuming.
		SetServiceStatus(SERVICE_CONTINUE_PENDING);

		// Perform service-specific continue operations.
		OnContinue();

		// Tell SCM that the service is running.
		SetServiceStatus(SERVICE_RUNNING);
	}
	catch (DWORD dwError)
	{
		// Log the error.
		WriteErrorLogEntry(L"Service Continue", dwError);

		// Tell SCM that the service is still paused.
		SetServiceStatus(SERVICE_PAUSED);
	}
	catch (...)
	{
		// Log the error.
		WriteEventLogEntry(L"Service failed to resume.", EVENTLOG_ERROR_TYPE);

		// Tell SCM that the service is still paused.
		SetServiceStatus(SERVICE_PAUSED);
	}
}


void CServiceBase::OnContinue()
{
}

void CServiceBase::Shutdown()
{
	try
	{
		// Perform service-specific shutdown operations.
		OnShutdown();

		// Tell SCM that the service is stopped.
		SetServiceStatus(SERVICE_STOPPED);
	}
	catch (DWORD dwError)
	{
		// Log the error.
		WriteErrorLogEntry(L"Service Shutdown", dwError);
	}
	catch (...)
	{
		// Log the error.
		WriteEventLogEntry(L"Service failed to shut down.", EVENTLOG_ERROR_TYPE);
	}
}


void CServiceBase::OnShutdown()
{
}

#pragma endregion


#pragma region Helper Functions

void CServiceBase::SetServiceStatus(DWORD dwCurrentState,
	DWORD dwWin32ExitCode,
	DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;

	// Fill in the SERVICE_STATUS structure of the service.

	m_status.dwCurrentState = dwCurrentState;
	m_status.dwWin32ExitCode = dwWin32ExitCode;
	m_status.dwWaitHint = dwWaitHint;

	m_status.dwCheckPoint =
		((dwCurrentState == SERVICE_RUNNING) ||
		(dwCurrentState == SERVICE_STOPPED)) ?
		0 : dwCheckPoint++;

	// Report the status of the service to the SCM.
	::SetServiceStatus(m_statusHandle, &m_status);
}

void CServiceBase::WriteEventLogEntry(PWSTR pszMessage, WORD wType)
{
	HANDLE hEventSource = NULL;
	LPCWSTR lpszStrings[2] = { NULL, NULL };

	hEventSource = RegisterEventSource(NULL, m_name);
	if (hEventSource)
	{
		lpszStrings[0] = m_name;
		lpszStrings[1] = pszMessage;

		ReportEvent(hEventSource,  // Event log handle
			wType,                 // Event type
			0,                     // Event category
			0,                     // Event identifier
			NULL,                  // No security identifier
			2,                     // Size of lpszStrings array
			0,                     // No binary data
			lpszStrings,           // Array of strings
			NULL                   // No binary data
		);

		DeregisterEventSource(hEventSource);
	}
}

void CServiceBase::WriteErrorLogEntry(PWSTR pszFunction, DWORD dwError)
{
	wchar_t szMessage[260];
	StringCchPrintf(szMessage, ARRAYSIZE(szMessage), L"%s failed w/err 0x%08lx", pszFunction, dwError);
	WriteEventLogEntry(szMessage, EVENTLOG_ERROR_TYPE);
}

#pragma endregion