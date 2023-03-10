#include <stdio.h>
#include <atomic>
#include <string>
#include <iostream>
#include <conio.h>

#include "ServerContainer.h"

using namespace::std;

int32_t StartService(int argc, const char** argv)
{
	return 0;
}

int main(int argc, const char** argv)
{
	std::string serverRevision = "1";

	cout << "[Server Launcher] Server launch : " << *argv << endl;
	cout << "[Server Launcher] Server revision : " << serverRevision << endl;

	int32_t ret = StartService(argc, argv);

	try
	{
		ServerContainer serverContainer(argv[0], serverRevision);

		int32_t ret = serverContainer.Load();
		if (ret != 0)
		{
			cout << "[Server Launcher] Server container load error." << endl;
			cout << "Server shutdown." << std::endl;
			return 0;
		}

		ret = serverContainer.Start();
		if (ret != 0)
		{
			cout << "[Server Launcher] Server container start error." << endl;
			cout << "Server shutdown." << std::endl;
			return 0;
		}

		std::cout << "\n==================== SERVER HAS STARTED!!! =====================\n" << std::endl;

		int checkRet;
		bool isContinue = true;

		do
		{
			checkRet = _getch();
			switch (checkRet)
			{
			case 'Q':
				isContinue = false;
				break;
			}
		} while (isContinue);

		cout << "Shutting down server..." << std::endl;

		//mainContainer.Unload();

		cout << "Server shutdown." << std::endl;
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}

	return 0;
}
