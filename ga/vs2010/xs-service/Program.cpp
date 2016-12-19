#include "ServiceInstaller.h"
#include "ServiceBase.h"
#include <iostream>
#include <Windows.h>
#include "Prgram.h"

#define SERVICE_NAME             L"XSIncGAW"
#define SERVICE_DISPLAY_NAME     L"XSIncGAW"
#define SERVICE_START_TYPE       SERVICE_AUTO_START
#define SERVICE_DEPENDENCIES     L""
#define SERVICE_ACCOUNT          NULL
#define SERVICE_PASSWORD         NULL

using namespace std;

int WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR    lpCmdLine,
                     int       nCmdShow)
{
    ServerStart();
    return 0;
}
/*
int wmain(int argc, wchar_t* argv[]) {
	FreeConsole();
	ServerStart();
	/*if (argc > 1) {
		if (_wcsicmp(argv[1], L"-install") == 0) {
			InstallService(
				SERVICE_NAME,               // Name of service
				SERVICE_DISPLAY_NAME,       // Name to display
				SERVICE_START_TYPE,         // Service start type
				SERVICE_DEPENDENCIES,       // Dependencies
				SERVICE_ACCOUNT,            // Service running account
				SERVICE_PASSWORD            // Password of the account
				);
			::Sleep(1000);

			DoStart(SERVICE_NAME);
		}
		else if (_wcsicmp(argv[1], L"-delete") == 0) {
			UninstallService(SERVICE_NAME);
		}
		else {
			cout << "e.g " << argv[0] << " (-install, -delete)" << endl;
		}
	}
	else {
		CServiceBase* service = new CServiceBase(SERVICE_NAME);
		if (!CServiceBase::Run(*service))
		{
			wprintf(L"Service failed to run w/err 0x%08lx\n", GetLastError());
		}
	}
	return 0;
}*/