#include <WinSock2.h>
#include <WS2tcpip.h>
#include <locale.h>
#include <crtdbg.h>
#include "Protocol.h"
#include "main.h"
#include "Select.h"

#pragma comment (lib, "ws2_32.lib")

// 소켓 관련 전역 변수
static SOCKET listenSock;
map<SOCKET, SOCKETINFO*> socketInfoMap;
list<ROOMINFO*> roomInfoList;

int totalSockets = 0;

int wmain()
{
	//_crtBreakAlloc = 402; // 중단점을 걸어주는 문구, _CrtSetDbgFlag와 세트
	setlocale(LC_ALL, "");

	int retVal;

	// 윈속 초기화
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		return -1;
	}

	// socket()
	listenSock = socket(AF_INET, SOCK_STREAM, 0);

	if (listenSock == INVALID_SOCKET)
	{
		ErrorQuit(L"socket()");
	}

	// bind()
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(NETWORK_PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	retVal = bind(listenSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

	if (retVal == SOCKET_ERROR)
	{
		ErrorQuit(L"bind()");
	}

	// listen()
	retVal = listen(listenSock, SOMAXCONN);

	if (retVal == SOCKET_ERROR)
	{
		ErrorQuit(L"listen()");
	}

	wprintf(L"Server Open!\n");

	Network();

	for (map<SOCKET, SOCKETINFO*>::iterator socketInfoMapIter = socketInfoMap.begin();
		socketInfoMapIter != socketInfoMap.end();)
	{
		if (socketInfoMapIter->second != nullptr)
		{
			socketInfoMapIter = RemoveSocketInfo(socketInfoMapIter->first);			
		}
		else
		{
			++socketInfoMapIter;
		}
	}

	for (list<ROOMINFO*>::iterator roomInfoListIter = roomInfoList.begin();
		roomInfoListIter != roomInfoList.end();)
	{
		if ((*roomInfoListIter) != nullptr)
		{
			(*roomInfoListIter)->sockInfo->clear();
			delete[] (*roomInfoListIter)->roomTitle;
			delete (*roomInfoListIter);
			roomInfoListIter = roomInfoList.erase(roomInfoListIter);
		}
		else 
		{
			++roomInfoListIter;
		}
	}

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	return 0;
}

void Network()
{
	int retval, addrLen;
	SOCKET clientSock;
	SOCKADDR_IN clientAddr;
	FD_SET rset, wset;

	do
	{
		map<SOCKET, SOCKETINFO*>::iterator socketInfoMapIter = socketInfoMap.begin();
		map<SOCKET, SOCKETINFO*>::iterator checkSockSetIter = socketInfoMap.begin();

		for (int i = 0; i <= totalSockets / 64; i++)
		{
			int socketCount = 1;

			// 소켓 셋 초기화
			FD_ZERO(&rset);
			FD_ZERO(&wset);

			// listenSock을 읽기 셋에 넣는다.
			FD_SET(listenSock, &rset);
			map<SOCKET, SOCKETINFO*>::iterator socketInfoMapEnd = socketInfoMap.end();

			// 현재 접속 중인 모든 클라이언트들의 소켓을 읽기 셋에 넣는다.
			for (socketInfoMapIter; socketInfoMapIter != socketInfoMapEnd; ++socketInfoMapIter)
			{
				FD_SET(socketInfoMapIter->first, &rset);

				if (socketInfoMapIter->second->sendQueue.GetUseSize() > 0)
				{
					FD_SET(socketInfoMapIter->first, &wset);
				}				

				socketCount++;

				if (socketCount >= 64)
				{
					++socketInfoMapIter;
					socketCount = 1;
					break;
				}
			}

			// select() 함수를 호출한다.
			timeval time;
			time.tv_sec = 0;
			time.tv_usec = 0;

			retval = select(0, &rset, &wset, NULL, &time);

			if (retval == SOCKET_ERROR)
			{
				ErrorQuit(L"select()");
			}

			// 소켓 set 검사 : 클라이언트 접속 수용
			// select() 함수가 리턴하면 먼저 읽기 set을 검사하여, 접속한 클라이언트가 있는지 확인한다.
			// 연결 대기 소켓이 읽기 set에 있다면 클라이언트가 접속한 경우이다.
			if (FD_ISSET(listenSock, &rset))
			{
				addrLen = sizeof(clientAddr);

				// accept 함수를 호출한다.
				clientSock = accept(listenSock, (SOCKADDR*)&clientAddr, &addrLen);

				// INVALIED_SOCKET이 리턴되면 오류를 화면에 출력한다.
				if (clientSock == INVALID_SOCKET)
				{
					ErrorQuit(L"accept()");
				}
				else
				{
					// AddSocketInfo() 함수를 호출하여 소켓 정보를 추가한다.
					if (AddSocketInfo(clientSock) == FALSE)
					{
						wprintf(L"클라이언트 접속을 해제합니다!\n");
						closesocket(clientSock);
					}
				}
			}

			// 소켓 set 검사 : 데이터 통신
			// select() 함수는 조건을 만족하는 소켓의 개수를 리턴하지만 구체적으로 어떤 소켓인지
			// 가르쳐주지 않으므로, 관리하고 있는 모든 소켓에 대해 소켓 set에 들어 있는지 여부를 확인해야 한다.
			for (checkSockSetIter; checkSockSetIter != socketInfoMapEnd;)
			{
				SOCKET tmpSock = checkSockSetIter->first;
				SOCKETINFO* tmpSocketInfo = checkSockSetIter->second;

				socketCount++;

				if (socketCount > 64)
				{
					++checkSockSetIter;
					socketCount = 1;
					break;
				}

				// 소켓이 읽기 셋에 들어 있다면 recv() 함수를 호출하여 데이터를 읽는다.
				if (FD_ISSET(tmpSock, &rset))
				{
					retval = FDReadProc(tmpSock, tmpSocketInfo);

					if (retval == SOCKET_ERROR)
					{
						checkSockSetIter = RemoveSocketInfo(tmpSock);
						continue;
					}
				}

				// 소켓이 쓰기 셋에 들어 있다면 send() 함수를 호출하여 데이터를 쓴다.
				if (FD_ISSET(tmpSock, &wset))
				{
					retval = FDWriteProc(tmpSock, tmpSocketInfo);

					if (retval == SOCKET_ERROR)
					{
						checkSockSetIter = RemoveSocketInfo(tmpSock);
						continue;
					}
				}

				++checkSockSetIter;
			}
		}		
	} while (!((GetAsyncKeyState(VK_DOWN) & 0x8000) && (GetAsyncKeyState(VK_SPACE) & 0x8000)));
}