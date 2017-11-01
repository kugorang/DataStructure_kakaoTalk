#pragma once

#include "Server.h"

class Application
{
private:
	// 서버
	Server server;

	// listen 소켓
	SOCKET listenSock;

	// 소켓 함수 오류 출력 후 종료
	void ErrorQuit(WCHAR* msg);

public:
	void Run();
};