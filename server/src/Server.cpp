#include "Server.h"

Server::~Server()
{
	for (map<int, USERINFO*>::iterator userInfoMapIter = userInfoMap.begin(); userInfoMapIter != userInfoMap.end();)
	{
		if (userInfoMapIter->second != nullptr)
		{
			delete userInfoMapIter->second;
			userInfoMapIter = userInfoMap.erase(userInfoMapIter);
		}
		else
		{
			++userInfoMapIter;
		}
	}

	for (map<SOCKET, SOCKETINFO*>::iterator socketInfoMapIter = socketInfoMap.begin(); socketInfoMapIter != socketInfoMap.end();)
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
			delete[](*roomInfoListIter)->roomTitle;
			delete (*roomInfoListIter);
			roomInfoListIter = roomInfoList.erase(roomInfoListIter);
		}
		else
		{
			++roomInfoListIter;
		}
	}

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
}

void Server::Network(SOCKET listenSock)
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

int Server::FDWriteProc(SOCKET sock, SOCKETINFO* socketInfo)
{
	int retval, useSize, notBrokenGetSize;
	RingBuffer* sendQueue = &(socketInfo->sendQueue);

	while (true)
	{
		useSize = sendQueue->GetUseSize();
		notBrokenGetSize = sendQueue->GetNotBrokenGetSize();

		if (useSize <= 0)
		{
			return 0;
		}

		retval = send(sock, (char*)sendQueue->GetFrontPosBufferPtr(), notBrokenGetSize, 0);

		if (retval == SOCKET_ERROR)
		{
			return SOCKET_ERROR;
		}
		else
		{
			sendQueue->MoveFrontPos(retval);
		}
	}
}

int Server::FDReadProc(SOCKET sock, SOCKETINFO* socketInfo)
{
	int retval, useSize, remainSize, notBrokenPutSize;
	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;
	RingBuffer* recvQueue = &(socketInfo->recvQueue);

	while (true)
	{
		remainSize = recvQueue->GetRemainSize();
		notBrokenPutSize = recvQueue->GetNotBrokenPutSize();

		retval = recv(sock, (char*)recvQueue->GetRearPosBufferPtr(), notBrokenPutSize, 0);

		if (retval == SOCKET_ERROR || retval == 0)
		{
			return SOCKET_ERROR;
		}
		else
		{
			recvQueue->MoveRearPos(retval);
		}

		while (true)
		{
			useSize = recvQueue->GetUseSize();

			if (sizeof(networkPacketHeader) > useSize)
			{
				return 0;
			}

			recvQueue->Peek((BYTE*)&networkPacketHeader, sizeof(networkPacketHeader));

			if ((int)(sizeof(networkPacketHeader) + networkPacketHeader.PayloadSize) > useSize)
			{
				return 0;
			}

			if (networkPacketHeader.code != NETWORK_PACKET_CODE)
			{
				return SOCKET_ERROR;
			}

			recvQueue->MoveFrontPos(sizeof(networkPacketHeader));

			retval = recvQueue->Dequeue(serializationBuffer.GetRearPosBufferPtr(),
				networkPacketHeader.PayloadSize);

			serializationBuffer.MoveRearPos(retval);

			if (MakeCheckSum(networkPacketHeader.MsgType, networkPacketHeader.PayloadSize,
				&serializationBuffer) == networkPacketHeader.checkSum)
			{
				PacketProc(sock, networkPacketHeader.MsgType, &serializationBuffer);
			}
			else
			{
				wprintf(L"CheckSum 에러 - ");
				RemoveSocketInfo(sock);
			}
		}
	}
}

// CheckSum을 만드는 함수
int Server::MakeCheckSum(WORD msgType, WORD payloadSize, SerializationBuffer* serializationBuffer)
{
	BYTE* confirmCheckSum = new BYTE[payloadSize];

	serializationBuffer->Peek(confirmCheckSum, payloadSize);

	WORD byteSum = 0;

	for (int i = 0; i < 2; i++)
	{
		byteSum += *((BYTE*)&msgType + i);
	}

	for (int i = 0; i < payloadSize; i++)
	{
		byteSum += *(confirmCheckSum + i);
	}

	delete[] confirmCheckSum;

	return byteSum % 256;
}

// Packet을 처리하는 함수
void Server::PacketProc(SOCKET sock, WORD type, SerializationBuffer* serializationBuffer)
{
	switch (type)
	{
	case REQUSET_LOGIN:
		RecvRequestLogin(sock, serializationBuffer);
		break;
	case REQUEST_ROOM_LIST:
		RecvRequestRoomList(sock, serializationBuffer);
		break;
	case REQUEST_ROOM_CREATE:
		RecvRequestRoomCreate(sock, serializationBuffer);
		break;
	case REQUEST_ROOM_ENTER:
		RecvRequestRoomEnter(sock, serializationBuffer);
		break;
	case REQUEST_CHAT:
		RecvRequestChat(sock, serializationBuffer);
		break;
	case REQUEST_ROOM_LEAVE:
		RecvRequestRoomLeave(sock, serializationBuffer);
		break;
	case REQUEST_JOIN:
		RecvRequestJoin(sock, serializationBuffer);
		break;
	case REQUEST_EDIT_INFO:
		RecvRequestEditInfo(sock, serializationBuffer);
		break;
	default:
		wprintf(L"메세지 타입 에러 - ");
		RemoveSocketInfo(sock);
		break;
	}
}

// 1. Request 로그인 처리 함수
void Server::RecvRequestLogin(SOCKET sock, SerializationBuffer* serializationBuffer)
{
	wprintf(L"Recv : 01 - 로그인 요청 [SOCK : %lld]\n", sock);

	WCHAR id[ID_MAX_LEN], pw[PW_MAX_LEN];

	*serializationBuffer >> id >> pw;

	SendResponseLogin(sock, id, pw);
}

// 2. Response 로그인 처리 함수
void Server::SendResponseLogin(SOCKET sock, WCHAR* id, WCHAR *pw)
{
	map<SOCKET, SOCKETINFO*>::iterator sockInfoMapIter = socketInfoMap.find(sock);
	SOCKETINFO* sockInfo = sockInfoMapIter->second;

	wprintf(L"Send : 02 - 로그인 응답 [SOCK : %lld]\n", sock);

	BYTE response = RESPONSE_LOGIN_ID_ERR;
	WCHAR idBuf[ID_MAX_LEN];
	WCHAR nameBuf[NAME_MAX_LEN];

	map<int, USERINFO*>::iterator userInfoMapIter;
	map<int, USERINFO*>::iterator userInfoMapEnd = userInfoMap.end();

	for (userInfoMapIter = userInfoMap.begin(); userInfoMapIter != userInfoMapEnd; ++userInfoMapIter)
	{
		USERINFO *userInfo = userInfoMapIter->second;

		// 아이디가 같을 때
		if (wcscmp(userInfo->id, id) == 0)
		{
			// 비밀번호까지 같다면
			if (wcscmp(userInfo->pw, pw) == 0)
			{
				map<SOCKET, SOCKETINFO*>::iterator socketInfoMapEnd = socketInfoMap.end();

				for (sockInfoMapIter = socketInfoMap.begin(); sockInfoMapIter != socketInfoMapEnd; ++sockInfoMapIter)
				{
					if ((*sockInfoMapIter).second->userInfo == userInfo)
					{
						response = RESPONSE_LOGIN_ALREADY;
						break;
					}
				}

				if (sockInfoMapIter == socketInfoMapEnd)
				{
					// 로그인 성공
					response = RESPONSE_LOGIN_OK;
					sockInfo->userInfo = userInfo;
					wcscpy_s(idBuf, sizeof(idBuf), userInfo->id);
					wcscpy_s(nameBuf, sizeof(nameBuf), userInfo->name);
					break;
				}
			}
			else
			{
				// 비밀번호가 다름을 알림.
				response = RESPONSE_LOGIN_PW_ERR;
				break;
			}
		}
	}

	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	MakePacketResponseLogin(&networkPacketHeader, &serializationBuffer, response, idBuf, nameBuf);

	SendUnicast(sock, networkPacketHeader, &serializationBuffer);
}

// 3. Request 대화방 리스트 처리 함수
void Server::RecvRequestRoomList(SOCKET sock, SerializationBuffer* serializationBuffer)
{
	wprintf(L"Recv : 03 - 방 리스트 요청 [NO : %d]\n", socketInfoMap.find(sock)->second->userInfo->userNo);

	SendResponseRoomList(sock);
}

// 4. Response 대화방 리스트 처리 함수
void Server::SendResponseRoomList(SOCKET sock)
{
	wprintf(L"Send : 04 - 방 리스트 응답 [NO : %d]\n", socketInfoMap.find(sock)->second->userInfo->userNo);

	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	MakePacketResponseRoomList(&networkPacketHeader, &serializationBuffer);

	SendUnicast(sock, networkPacketHeader, &serializationBuffer);
}

// 5. Request 대화방 생성 처리 함수
void Server::RecvRequestRoomCreate(SOCKET sock, SerializationBuffer* serializationBuffer)
{
	wprintf(L"Recv : 05 - 방 생성 요청 [NO : %d]\n", socketInfoMap.find(sock)->second->userInfo->userNo);

	WORD roomTitleSize;

	*serializationBuffer >> roomTitleSize;

	WCHAR* roomTitle = new WCHAR[roomTitleSize / 2 + 1];

	serializationBuffer->Dequeue((BYTE*)roomTitle, roomTitleSize + 2);

	roomTitle[roomTitleSize / 2] = '\0';

	SendResponseRoomCreate(sock, roomTitle, roomTitleSize + 2);
}

// 6. Response 대화방 생성 (수시) 처리 함수
void Server::SendResponseRoomCreate(SOCKET sock, WCHAR* roomTitle, WORD roomTitleSize)
{
	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	static WORD roomNum = 1;
	BYTE response = RESPONSE_ROOM_CREATE_OK;

	// 방 이름 중복 검사
	for (list<ROOMINFO*>::iterator iter = roomInfoList.begin(); iter != roomInfoList.end(); ++iter)
	{
		if (wcscmp((*iter)->roomTitle, roomTitle) == 0)
		{
			response = RESPONSE_ROOM_CREATE_DNICK;
			break;
		}
	}

	// 방 개수 초과 확인
	if (totalRooms >= ROOM_MAX)
	{
		response = RESPONSE_ROOM_CREATE_MAX;
	}

	wprintf(L"Send : 06 - 방 생성 응답 [NO : %d]\n", socketInfoMap.find(sock)->second->userInfo->userNo);

	ROOMINFO* roomInfoStruct = new ROOMINFO;

	roomInfoStruct->roomNo = roomNum++;
	roomInfoStruct->roomTitleSize = roomTitleSize;
	roomInfoStruct->roomTitle = roomTitle;
	roomInfoStruct->numberOfPeople = 0;

	MakePacketResponseRoomCreate(&networkPacketHeader, &serializationBuffer,
		response, roomInfoStruct);

	if (response == RESPONSE_ROOM_CREATE_OK)
	{
		totalRooms++;
		roomInfoList.push_back(roomInfoStruct);

		SendBroadcast(networkPacketHeader, &serializationBuffer);
	}
	else
	{
		SendUnicast(sock, networkPacketHeader, &serializationBuffer);
	}

}

// 7. Request 대화방 입장 처리 함수
void Server::RecvRequestRoomEnter(SOCKET sock, SerializationBuffer* serializationBuffer)
{
	map<SOCKET, SOCKETINFO*>::iterator socketInfoMapIter = socketInfoMap.find(sock);

	SOCKETINFO* tmpSocketInfo = socketInfoMapIter->second;

	wprintf(L"Recv : 07 - 방 입장 요청 [NO : %d]\n", tmpSocketInfo->userInfo->userNo);

	int roomNum;

	*serializationBuffer >> roomNum;

	BYTE response = RESPONSE_ROOM_ENTER_OK;

	// 방 번호 오류 검사
	list<ROOMINFO*>::iterator roomInfoListIter;
	list<ROOMINFO*>::iterator roomInfoListEnd = roomInfoList.end();

	for (roomInfoListIter = roomInfoList.begin(); roomInfoListIter != roomInfoListEnd;
		++roomInfoListIter)
	{
		if ((*roomInfoListIter)->roomNo == roomNum)
		{
			break;
		}
	}

	if (roomInfoListIter == roomInfoListEnd || tmpSocketInfo->alreadyRoom)
	{
		response = RESPONSE_ROOM_ENTER_NOT;
	}

	// 인원 초과
	if (!(roomInfoListIter == roomInfoListEnd) && (*roomInfoListIter)->numberOfPeople >= ROOM_PEOPLE_MAX)
	{
		response = RESPONSE_ROOM_ENTER_MAX;
	}

	SendResponseRoomEnter(sock, response, (*roomInfoListIter));
}

// 8. Response 대화방 입장 처리 함수
void Server::SendResponseRoomEnter(SOCKET sock, BYTE response, ROOMINFO* roomInfoPtr)
{
	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	map<SOCKET, SOCKETINFO*>::iterator socketInfoMapIter = socketInfoMap.find(sock);
	SOCKETINFO* tmpSocketInfo = socketInfoMapIter->second;

	if (response == RESPONSE_ROOM_ENTER_OK)
	{
		wprintf(L"Send : 08 - 방 입장 응답 [NO : %d]\n", tmpSocketInfo->userInfo->userNo);

		roomInfoPtr->sockInfo->push_back(tmpSocketInfo);

		tmpSocketInfo->alreadyRoom = true;
		tmpSocketInfo->enterRoomNo = roomInfoPtr->roomNo;

		(roomInfoPtr->numberOfPeople)++;
	}

	if (MakePacketResponseRoomEnter(&networkPacketHeader, &serializationBuffer, response, roomInfoPtr))
	{
		SendUnicast(sock, networkPacketHeader, &serializationBuffer);

		SendResponseUserEnter(sock, tmpSocketInfo);
	}
}

// 9. Request 채팅 송신 처리 함수
void Server::RecvRequestChat(SOCKET sock, SerializationBuffer* serializationBuffer)
{
	WORD msgSize;

	wprintf(L"Recv : 09 - 채팅 송신 [NO : %d]\n", socketInfoMap.find(sock)->second->userInfo->userNo);

	*serializationBuffer >> msgSize;

	WCHAR* msg = new WCHAR[msgSize / 2 + 1];

	serializationBuffer->Dequeue((BYTE*)msg, msgSize + 2);

	msg[msgSize / 2] = '\0';

	SendResponseChat(sock, msgSize + 2, msg);

	delete[] msg;
}

// 10. Response 채팅 수신 (수시) (나에겐 오지 않음) 처리 함수
void Server::SendResponseChat(SOCKET sock, WORD msgSize, WCHAR* msg)
{
	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	int roomNo = MakePacketResponseChat(&networkPacketHeader, &serializationBuffer, sock, msgSize, msg);

	SendBroadcastRoom(sock, networkPacketHeader, &serializationBuffer, roomNo);
}

// 11. Request 방 퇴장 처리 함수
void Server::RecvRequestRoomLeave(SOCKET sock, SerializationBuffer* serializationBuffer)
{
	wprintf(L"Recv : 11 - 방 퇴장 요청 [NO : %d]\n", socketInfoMap.find(sock)->second->userInfo->userNo);

	SendResponseRoomLeave(sock);
}

// 12. Response 방 퇴장 (수시) 처리 함수
void Server::SendResponseRoomLeave(SOCKET sock)
{
	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	int roomNo = MakePacketResponseRoomLeave(&networkPacketHeader, &serializationBuffer, sock);

	map<SOCKET, SOCKETINFO*>::iterator socketInfoMapIter = socketInfoMap.find(sock);
	SOCKETINFO* tmpSocketInfo = socketInfoMapIter->second;

	tmpSocketInfo->alreadyRoom = false;
	tmpSocketInfo->enterRoomNo = -1;

	SendBroadcast(networkPacketHeader, &serializationBuffer);

	ROOMINFO* tmpRoomInfo = FindRoom(roomNo);

	if (tmpRoomInfo == nullptr)
	{
		wprintf(L"방을 찾지 못하였습니다. (12)\n");
		return;
	}

	list<SOCKETINFO*>* sockInfo = tmpRoomInfo->sockInfo;
	list<SOCKETINFO*>::iterator sockInfoIter;
	list<SOCKETINFO*>::iterator sockInfoEnd = sockInfo->end();

	for (sockInfoIter = sockInfo->begin(); sockInfoIter != sockInfoEnd; ++sockInfoIter)
	{
		if ((*sockInfoIter) == tmpSocketInfo)
		{
			sockInfo->erase(sockInfoIter);
			break;
		}
	}

	(tmpRoomInfo->numberOfPeople)--;

	if (tmpRoomInfo->numberOfPeople == 0)
	{
		SendResponseRoomDelete(sock, roomNo);
	}
}

// 13. Response 방 삭제 (수시) 처리 함수
void Server::SendResponseRoomDelete(SOCKET sock, int roomNo)
{
	wprintf(L"Send : 13 - 방 삭제 응답 [NO : %d]\n", socketInfoMap.find(sock)->second->userInfo->userNo);

	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	MakePacketResponseRoomDelete(&networkPacketHeader, &serializationBuffer, roomNo);

	ROOMINFO* tmpRoomInfo = FindRoom(roomNo);

	if (tmpRoomInfo != nullptr)
	{
		delete[] tmpRoomInfo->roomTitle;
		delete tmpRoomInfo;

		for (list<ROOMINFO*>::iterator roomInfoListIter = roomInfoList.begin();
			roomInfoListIter != roomInfoList.end(); ++roomInfoListIter)
		{
			if ((*roomInfoListIter) == tmpRoomInfo)
			{
				roomInfoList.erase(roomInfoListIter);
				break;
			}
		}
	}

	totalRooms--;

	SendBroadcast(networkPacketHeader, &serializationBuffer);
}

// 14. Response 타 사용자 입장 (수시) 처리 함수
void Server::SendResponseUserEnter(SOCKET sock, SOCKETINFO* socketInfo)
{
	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	MakePacketResponseUserEnter(&networkPacketHeader, &serializationBuffer, sock, socketInfo);

	SendBroadcastRoom(sock, networkPacketHeader, &serializationBuffer, socketInfo->enterRoomNo);
}

// 15. Request 회원 가입 처리 함수
void Server::RecvRequestJoin(SOCKET sock, SerializationBuffer *serializationBuffer)
{
	wprintf(L"Recv : 15 - 회원 가입 요청 [SOCK : %lld]\n", sock);

	WCHAR id[ID_MAX_LEN], pw[PW_MAX_LEN], name[NAME_MAX_LEN], phoneNum[PHONENUM_MAX_LEN];

	*serializationBuffer >> id >> pw >> name >> phoneNum;

	SendResponseJoin(sock, id, pw, name, phoneNum);
}

// 16. Response 회원 가입 처리 함수
void Server::SendResponseJoin(SOCKET sock, WCHAR *id, WCHAR *pw, WCHAR *name, WCHAR *phoneNum)
{
	wprintf(L"Send : 16 - 회원 가입 응답 [SOCK : %lld]\n", sock);

	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	BYTE response = RESPONSE_JOIN_OK;

	// 아이디 중복 검사.
	map<int, USERINFO*>::iterator userInfoMapIter;

	for (userInfoMapIter = userInfoMap.begin(); userInfoMapIter != userInfoMap.end(); ++userInfoMapIter)
	{
		if (wcscmp(userInfoMapIter->second->id, id) == 0)
		{
			response = RESPONSE_JOIN_DNICK;
			break;
		}
	}

	// 사용자 초과 확인
	if (totalSockets >= SOCKETINFO_ARRAY_MAX)
	{
		response = RESPONSE_JOIN_MAX;
	}

	static int userNo = 1;

	if (response == RESPONSE_LOGIN_OK)
	{
		USERINFO *newUser = new USERINFO;

		newUser->userNo = userNo++;
		wcscpy_s(newUser->id, ID_MAX_LEN, id);
		wcscpy_s(newUser->pw, PW_MAX_LEN, pw);
		wcscpy_s(newUser->name, NAME_MAX_LEN, name);
		wcscpy_s(newUser->phoneNum, PHONENUM_MAX_LEN, phoneNum);

		userInfoMap.insert(map<int, USERINFO*>::value_type(userNo, newUser));
	}

	MakePacketResponseJoin(&networkPacketHeader, &serializationBuffer, response, userNo);

	SendUnicast(sock, networkPacketHeader, &serializationBuffer);
}

// 17. Request 회원 정보 수정 함수
void Server::RecvRequestEditInfo(SOCKET sock, SerializationBuffer *serializationBuffer)
{
	wprintf(L"Recv : 17 - 회원 정보 수정 요청 [NO : %d]\n", socketInfoMap.find(sock)->second->userInfo->userNo);

	WCHAR id[ID_MAX_LEN], pw[PW_MAX_LEN], name[NAME_MAX_LEN], phoneNum[PHONENUM_MAX_LEN];

	*serializationBuffer >> id >> pw >> name >> phoneNum;

	SendResponseEditInfo(sock, id, pw, name, phoneNum);
}

// 18. Response 회원 정보 수정 함수
void Server::SendResponseEditInfo(SOCKET sock, WCHAR *id, WCHAR *pw, WCHAR *name, WCHAR *phoneNum)
{
	USERINFO *editUser = socketInfoMap.find(sock)->second->userInfo;
	wprintf(L"Send : 16 - 회원 정보 수정 응답 [NO : %d]\n", editUser->userNo);

	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	BYTE response = RESPONSE_EDIT_INFO_OK;

	// 아이디 중복 검사.
	map<int, USERINFO*>::iterator userInfoMapIter;

	for (userInfoMapIter = userInfoMap.begin(); userInfoMapIter != userInfoMap.end(); ++userInfoMapIter)
	{
		if (wcscmp(userInfoMapIter->second->id, id) == 0)
		{
			response = RESPONSE_EDIT_INFO_DNICK;
			break;
		}
	}

	if (response == RESPONSE_LOGIN_OK)
	{
		wcscpy_s(editUser->id, ID_MAX_LEN, id);
		wcscpy_s(editUser->pw, PW_MAX_LEN, pw);
		wcscpy_s(editUser->name, NAME_MAX_LEN, name);
		wcscpy_s(editUser->phoneNum, PHONENUM_MAX_LEN, phoneNum);
	}

	MakePacketResponseEditInfo(&networkPacketHeader, &serializationBuffer, response, editUser->id, editUser->name);

	SendUnicast(sock, networkPacketHeader, &serializationBuffer);
}

// 소켓 정보를 추가하는 함수이다.
BOOL Server::AddSocketInfo(SOCKET sock)
{
	if (totalSockets > SOCKETINFO_ARRAY_MAX)
	{
		wprintf(L"[오류] 소켓 정보를 추가할 수 없습니다!\n");

		return FALSE;
	}

	SOCKETINFO* ptr = new SOCKETINFO;

	ptr->alreadyRoom = false;
	ptr->enterRoomNo = -1;

	wprintf(L"소켓 연결 [SOCK : %lld]\n", sock);

	socketInfoMap.insert(map<SOCKET, SOCKETINFO*>::value_type(sock, ptr));
	//socketInfoMap[sock] = ptr; // -> 메모리를 추가 할당하기 때문에 사용하지 말 것.

	totalSockets++;

	return TRUE;
}

// 소켓 정보를 삭제하는 함수이다.
map<SOCKET, SOCKETINFO*>::iterator Server::RemoveSocketInfo(SOCKET sock)
{
	map<SOCKET, SOCKETINFO*>::iterator iter = socketInfoMap.find(sock);
	SOCKETINFO *sockInfo = iter->second;

	wprintf(L"연결 해제 [SOCK : %lld]\n", sock);

	if (iter != socketInfoMap.end())
	{
		if (sockInfo->enterRoomNo != -1)
		{
			SendResponseRoomLeave(sock);
		}

		delete sockInfo;

		closesocket(sock);

		iter = socketInfoMap.erase(iter);
		totalSockets--;
	}

	return iter;
}

// 방을 찾는 함수
ROOMINFO* Server::FindRoom(int roomNo)
{
	list<ROOMINFO*>::iterator roomInfoListIter;

	for (roomInfoListIter = roomInfoList.begin(); roomInfoListIter != roomInfoList.end();	++roomInfoListIter)
	{
		if ((*roomInfoListIter)->roomNo == roomNo)
		{
			return (*roomInfoListIter);
		}
	}

	return nullptr;
}

// 그 사람에게만 송신하는 함수
void Server::SendUnicast(SOCKET sock, NetworkPacketHeader networkPacketHeader,
	SerializationBuffer* serializationBuffer)
{
	map<SOCKET, SOCKETINFO*>::iterator iter = socketInfoMap.find(sock);

	iter->second->sendQueue.Enqueue((BYTE*)&networkPacketHeader, sizeof(networkPacketHeader));
	iter->second->sendQueue.Enqueue(serializationBuffer->GetFrontPosBufferPtr(),
		serializationBuffer->GetUseSize());
}

// 그 사람에게만 송신하는 함수
void Server::SendUnicast(SOCKETINFO* socketInfo, NetworkPacketHeader networkPacketHeader, SerializationBuffer* serializationBuffer)
{
	socketInfo->sendQueue.Enqueue((BYTE*)&networkPacketHeader, sizeof(networkPacketHeader));
	socketInfo->sendQueue.Enqueue(serializationBuffer->GetFrontPosBufferPtr(),
		serializationBuffer->GetUseSize());
}

// 모든 사람들에게 송신하는 함수
void Server::SendBroadcast(NetworkPacketHeader networkPacketHeader, SerializationBuffer* serializationBuffer)
{
	map<SOCKET, SOCKETINFO*>::iterator iter;

	for (iter = socketInfoMap.begin(); iter != socketInfoMap.end(); ++iter)
	{
		SendUnicast(iter->first, networkPacketHeader, serializationBuffer);
	}
}

// 방에 있는 사람들에게 송신하는 함수
void Server::SendBroadcastRoom(SOCKET exceptSock, NetworkPacketHeader networkPacketHeader,
	SerializationBuffer* serializationBuffer, int roomNo)
{
	ROOMINFO* tmpRoomInfo = FindRoom(roomNo);

	if (tmpRoomInfo == nullptr)
	{
		wprintf(L"방을 찾지 못하였습니다 (SBR).\n");
		return;
	}

	list<SOCKETINFO*>* sockInfo = tmpRoomInfo->sockInfo;

	list<SOCKETINFO*>::iterator sockInfoListIter;

	map<SOCKET, SOCKETINFO*>::iterator socketInfoMapIter = socketInfoMap.find(exceptSock);
	int exceptSockUserNo = socketInfoMapIter->second->userInfo->userNo;

	for (sockInfoListIter = sockInfo->begin(); sockInfoListIter != sockInfo->end(); ++sockInfoListIter)
	{
		if ((*sockInfoListIter)->userInfo->userNo != exceptSockUserNo)
		{
			SendUnicast(*sockInfoListIter, networkPacketHeader, serializationBuffer);
		}
	}
}

// 소켓 함수 오류 출력 후 종료
void Server::ErrorQuit(WCHAR* msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
}

// 2. Response 로그인 처리 패킷을 만드는 함수
void Server::MakePacketResponseLogin(NetworkPacketHeader* networkPacketHeader, SerializationBuffer* serializationBuffer, BYTE response, WCHAR *id, WCHAR *name)
{
	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->MsgType = RESPONSE_LOGIN;

	*serializationBuffer << response;
	serializationBuffer->Enqueue((BYTE*)id, ID_MAX_LEN * sizeof(WCHAR));
	serializationBuffer->Enqueue((BYTE*)name, NAME_MAX_LEN * sizeof(WCHAR));

	networkPacketHeader->PayloadSize = serializationBuffer->GetUseSize();
	networkPacketHeader->checkSum = MakeCheckSum(networkPacketHeader->MsgType, networkPacketHeader->PayloadSize, serializationBuffer);
}

// 4. Response 대화방 리스트 처리 함수
void Server::MakePacketResponseRoomList(NetworkPacketHeader* networkPacketHeader, SerializationBuffer* serializationBuffer)
{
	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->MsgType = RESPONSE_ROOM_LIST;

	list<ROOMINFO*>::size_type roomNum = roomInfoList.size();

	*serializationBuffer << (WORD)roomNum;

	for (list<ROOMINFO*>::iterator iter = roomInfoList.begin(); iter != roomInfoList.end(); ++iter)
	{
		int roomNo = (*iter)->roomNo;
		WORD roomTitleSize = (*iter)->roomTitleSize;
		WCHAR* roomTitle = (*iter)->roomTitle;
		BYTE* numberOfPeople = &((*iter)->numberOfPeople);
		list<SOCKETINFO*>* sockInfo = (*iter)->sockInfo;

		*serializationBuffer << roomNo << roomTitleSize;

		serializationBuffer->Enqueue((BYTE*)roomTitle, roomTitleSize);

		*serializationBuffer << *numberOfPeople;

		list<SOCKETINFO*>::iterator sockInfoIter;

		for (sockInfoIter = sockInfo->begin(); sockInfoIter != sockInfo->end(); ++sockInfoIter)
		{
			*serializationBuffer << (*sockInfoIter)->userInfo->name;
		}
	}

	networkPacketHeader->PayloadSize = serializationBuffer->GetUseSize();
	networkPacketHeader->checkSum = MakeCheckSum(networkPacketHeader->MsgType, networkPacketHeader->PayloadSize, serializationBuffer);
}

// 6. Response 대화방 생성 (수시) 처리 함수
void Server::MakePacketResponseRoomCreate(NetworkPacketHeader* networkPacketHeader, SerializationBuffer* serializationBuffer, BYTE response, ROOMINFO* roomInfoStruct)
{
	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->MsgType = RESPONSE_ROOM_CREATE;

	WORD roomTitleSize = roomInfoStruct->roomTitleSize;

	*serializationBuffer << response << roomInfoStruct->roomNo << roomTitleSize;

	serializationBuffer->Enqueue((BYTE*)roomInfoStruct->roomTitle, roomTitleSize);

	networkPacketHeader->PayloadSize = serializationBuffer->GetUseSize();

	networkPacketHeader->checkSum = MakeCheckSum(networkPacketHeader->MsgType, networkPacketHeader->PayloadSize, serializationBuffer);
}

// 8. Response 대화방 입장 처리 함수
bool Server::MakePacketResponseRoomEnter(NetworkPacketHeader* networkPacketHeader, SerializationBuffer* serializationBuffer, BYTE response, ROOMINFO* roomInfoPtr)
{
	if (response == RESPONSE_ROOM_ENTER_OK)
	{
		WORD roomTitleSize = roomInfoPtr->roomTitleSize;
		list<SOCKETINFO*>* sockInfo = roomInfoPtr->sockInfo;

		networkPacketHeader->code = NETWORK_PACKET_CODE;
		networkPacketHeader->MsgType = RESPONSE_ROOM_ENTER;

		*serializationBuffer << response << roomInfoPtr->roomNo << roomTitleSize;

		serializationBuffer->Enqueue((BYTE*)roomInfoPtr->roomTitle, roomTitleSize);

		*serializationBuffer << roomInfoPtr->numberOfPeople;

		list<SOCKETINFO*>::iterator socketInfoIter;

		for (socketInfoIter = sockInfo->begin(); socketInfoIter != sockInfo->end(); ++socketInfoIter)
		{
			*serializationBuffer << (*socketInfoIter)->userInfo->name << (*socketInfoIter)->userInfo->userNo;
		}

		networkPacketHeader->PayloadSize = serializationBuffer->GetUseSize();

		networkPacketHeader->checkSum = MakeCheckSum(networkPacketHeader->MsgType, networkPacketHeader->PayloadSize, serializationBuffer);

		return true;
	}

	return false;
}

// 10. Response 채팅 수신 (수시) (나에겐 오지 않음) 처리 함수
int Server::MakePacketResponseChat(NetworkPacketHeader* networkPacketHeader,
	SerializationBuffer* serializationBuffer, SOCKET sock, WORD msgSize, WCHAR* msg)
{
	map<SOCKET, SOCKETINFO*>::iterator socketInfoMapIter = socketInfoMap.find(sock);

	SOCKETINFO* tmpSocketInfo = socketInfoMapIter->second;

	wprintf(L"Send : 10 - 채팅 수신 [NO : %d]\n", tmpSocketInfo->userInfo->userNo);

	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->MsgType = RESPONSE_CHAT;

	*serializationBuffer << tmpSocketInfo->userInfo->userNo << msgSize;

	serializationBuffer->Enqueue((BYTE*)msg, msgSize);

	networkPacketHeader->PayloadSize = serializationBuffer->GetUseSize();
	networkPacketHeader->checkSum = MakeCheckSum(networkPacketHeader->MsgType, networkPacketHeader->PayloadSize, serializationBuffer);

	return tmpSocketInfo->enterRoomNo;
}

// 12. Response 방 퇴장 (수시) 처리 함수
int Server::MakePacketResponseRoomLeave(NetworkPacketHeader* networkPacketHeader, SerializationBuffer* serializationBuffer, SOCKET sock)
{
	map<SOCKET, SOCKETINFO*>::iterator socketInfoMapIter = socketInfoMap.find(sock);

	SOCKETINFO* tmpSocketInfo = socketInfoMapIter->second;

	int userNo = tmpSocketInfo->userInfo->userNo;

	wprintf(L"Send : 12 - 방 퇴장 응답 [NO : %d]\n", userNo);

	*serializationBuffer << userNo;

	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->MsgType = RESPONSE_ROOM_LEAVE;
	networkPacketHeader->PayloadSize = serializationBuffer->GetUseSize();
	networkPacketHeader->checkSum = MakeCheckSum(networkPacketHeader->MsgType, networkPacketHeader->PayloadSize, serializationBuffer);

	return tmpSocketInfo->enterRoomNo;
}

// 13. Response 방 삭제 (수시) 처리 함수
void Server::MakePacketResponseRoomDelete(NetworkPacketHeader* networkPacketHeader, SerializationBuffer* serializationBuffer, int roomNo)
{
	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->MsgType = RESPONSE_ROOM_DELETE;

	*serializationBuffer << roomNo;

	networkPacketHeader->PayloadSize = serializationBuffer->GetUseSize();
	networkPacketHeader->checkSum = MakeCheckSum(networkPacketHeader->MsgType, networkPacketHeader->PayloadSize, serializationBuffer);
}

// 14. Response 타 사용자 입장 (수시) 처리 함수
void Server::MakePacketResponseUserEnter(NetworkPacketHeader* networkPacketHeader, SerializationBuffer* serializationBuffer, SOCKET sock, SOCKETINFO* socketInfo)
{
	int userNo = socketInfo->userInfo->userNo;

	wprintf(L"Send : 14 - 다른 유저 입장 응답 [NO : %d]\n", userNo);

	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->MsgType = RESPONSE_USER_ENTER;

	serializationBuffer->Enqueue((BYTE*)socketInfo->userInfo->name, sizeof(WCHAR) * ID_MAX_LEN);
	*serializationBuffer << userNo;

	networkPacketHeader->PayloadSize = serializationBuffer->GetUseSize();
	networkPacketHeader->checkSum = MakeCheckSum(networkPacketHeader->MsgType, networkPacketHeader->PayloadSize, serializationBuffer);
}

// 16. Response 회원 가입 처리 함수
void Server::MakePacketResponseJoin(NetworkPacketHeader *networkPacketHeader, SerializationBuffer *serializationBuffer, BYTE response, int userNo)
{
	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->MsgType = RESPONSE_JOIN;

	*serializationBuffer << response << userNo;

	networkPacketHeader->PayloadSize = serializationBuffer->GetUseSize();
	networkPacketHeader->checkSum = MakeCheckSum(networkPacketHeader->MsgType, networkPacketHeader->PayloadSize, serializationBuffer);
}

// 18. Response 회원 정보 수정 처리 함수
void Server::MakePacketResponseEditInfo(NetworkPacketHeader *networkPacketHeader, SerializationBuffer *serializationBuffer, BYTE response, WCHAR *idBuf, WCHAR *nameBuf)
{
	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->MsgType = RESPONSE_EDIT_INFO;

	*serializationBuffer << response;
	
	serializationBuffer->Enqueue((BYTE*)idBuf, ID_MAX_LEN * sizeof(WCHAR));
	serializationBuffer->Enqueue((BYTE*)nameBuf, NAME_MAX_LEN * sizeof(WCHAR));

	networkPacketHeader->PayloadSize = serializationBuffer->GetUseSize();
	networkPacketHeader->checkSum = MakeCheckSum(networkPacketHeader->MsgType, networkPacketHeader->PayloadSize, serializationBuffer);
}