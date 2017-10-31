#include <iostream>
#include "Protocol.h"
#include "main.h"
#include "Select.h"
#include "MakePacket.h"

using namespace std;

static int totalRooms = 0;

int FDWriteProc(SOCKET sock, SOCKETINFO* socketInfo)
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

int FDReadProc(SOCKET sock, SOCKETINFO* socketInfo)
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

			if(networkPacketHeader.code != NETWORK_PACKET_CODE)
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
int MakeCheckSum(WORD msgType, WORD payloadSize, SerializationBuffer* serializationBuffer)
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
void PacketProc(SOCKET sock, WORD type, SerializationBuffer* serializationBuffer)
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
	default:
		wprintf(L"메세지 타입 에러 - ");
		RemoveSocketInfo(sock);
		break;
	}
}

// 1. Request 로그인 처리 함수
void RecvRequestLogin(SOCKET sock, SerializationBuffer* serializationBuffer)
{
	wprintf(L"Recv : 01 - 로그인 요청 [NO : %d]\n", socketInfoMap.find(sock)->second->userNo);

	WCHAR nickname[NICK_MAX_LEN] = { 0, };

	*serializationBuffer >> nickname;

	SendResponseLogin(sock, nickname);
}

// 2. Response 로그인 처리 함수
void SendResponseLogin(SOCKET sock, WCHAR* nickname)
{
	map<SOCKET, SOCKETINFO*>::iterator socketInfoMapIter = socketInfoMap.find(sock);

	SOCKETINFO* tmpSocketInfo = socketInfoMapIter->second;

	int userNo = tmpSocketInfo->userNo;

	wprintf(L"Send : 02 - 로그인 응답 [NO : %d]\n", userNo);

	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	BYTE response = RESPONSE_LOGIN_OK;

	// 닉네임 중복 검사.
	for (socketInfoMapIter = socketInfoMap.begin(); socketInfoMapIter != socketInfoMap.end();
		++socketInfoMapIter)
	{
		if (wcscmp(socketInfoMapIter->second->nickname, nickname) == 0)
		{
			response = RESPONSE_LOGIN_DNICK;
			break;
		}
	}
	
	// 사용자 초과 확인
	if (totalSockets >= SOCKETINFO_ARRAY_MAX)
	{
		response = RESPONSE_LOGIN_MAX;
	}

	socketInfoMapIter = socketInfoMap.find(sock);

	if (response == RESPONSE_LOGIN_OK)
	{
		wcscpy_s(socketInfoMapIter->second->nickname, NICK_MAX_LEN, nickname);
	}
	
	MakePacketResponseLogin(&networkPacketHeader, &serializationBuffer, response, userNo);

	SendUnicast(sock, networkPacketHeader, &serializationBuffer);
}

// 3. Request 대화방 리스트 처리 함수
void RecvRequestRoomList(SOCKET sock, SerializationBuffer* serializationBuffer)
{
	wprintf(L"Recv : 03 - 방 리스트 요청 [NO : %d]\n", socketInfoMap.find(sock)->second->userNo);

	SendResponseRoomList(sock);
}

// 4. Response 대화방 리스트 처리 함수
void SendResponseRoomList(SOCKET sock)
{
	wprintf(L"Send : 04 - 방 리스트 응답 [NO : %d]\n", socketInfoMap.find(sock)->second->userNo);

	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	MakePacketResponseRoomList(&networkPacketHeader, &serializationBuffer);

	SendUnicast(sock, networkPacketHeader, &serializationBuffer);
}

// 5. Request 대화방 생성 처리 함수
void RecvRequestRoomCreate(SOCKET sock, SerializationBuffer* serializationBuffer)
{
	wprintf(L"Recv : 05 - 방 생성 요청 [NO : %d]\n", socketInfoMap.find(sock)->second->userNo);

	WORD roomTitleSize;
	
	*serializationBuffer >> roomTitleSize;

	WCHAR* roomTitle = new WCHAR[roomTitleSize / 2 + 1];
	
	serializationBuffer->Dequeue((BYTE*)roomTitle, roomTitleSize + 2);
	
	roomTitle[roomTitleSize / 2] = '\0';

	SendResponseRoomCreate(sock, roomTitle, roomTitleSize + 2);
}

// 6. Response 대화방 생성 (수시) 처리 함수
void SendResponseRoomCreate(SOCKET sock, WCHAR* roomTitle, WORD roomTitleSize)
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

	wprintf(L"Send : 06 - 방 생성 응답 [NO : %d]\n", socketInfoMap.find(sock)->second->userNo);
	
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
void RecvRequestRoomEnter(SOCKET sock, SerializationBuffer* serializationBuffer)
{
	map<SOCKET, SOCKETINFO*>::iterator socketInfoMapIter = socketInfoMap.find(sock);

	SOCKETINFO* tmpSocketInfo = socketInfoMapIter->second;

	wprintf(L"Recv : 07 - 방 입장 요청 [NO : %d]\n", tmpSocketInfo->userNo);

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
	if (!(roomInfoListIter == roomInfoListEnd) 
		&& (*roomInfoListIter)->numberOfPeople >= ROOM_PEOPLE_MAX)
	{
		response = RESPONSE_ROOM_ENTER_MAX;
	}
	
	SendResponseRoomEnter(sock, response, (*roomInfoListIter));
}

// 8. Response 대화방 입장 처리 함수
void SendResponseRoomEnter(SOCKET sock, BYTE response, ROOMINFO* roomInfoPtr)
{
	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	map<SOCKET, SOCKETINFO*>::iterator socketInfoMapIter = socketInfoMap.find(sock);
	SOCKETINFO* tmpSocketInfo = socketInfoMapIter->second;

	if (response == RESPONSE_ROOM_ENTER_OK)
	{
		wprintf(L"Send : 08 - 방 입장 응답 [NO : %d]\n", tmpSocketInfo->userNo);

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
void RecvRequestChat(SOCKET sock, SerializationBuffer* serializationBuffer)
{
	WORD msgSize;

	wprintf(L"Recv : 09 - 채팅 송신 [NO : %d]\n", socketInfoMap.find(sock)->second->userNo);

	*serializationBuffer >> msgSize;

	WCHAR* msg = new WCHAR[msgSize / 2 + 1];

	serializationBuffer->Dequeue((BYTE*)msg, msgSize + 2);

	msg[msgSize / 2] = '\0';

	SendResponseChat(sock, msgSize + 2, msg);

	delete[] msg;
}

// 10. Response 채팅 수신 (수시) (나에겐 오지 않음) 처리 함수
void SendResponseChat(SOCKET sock, WORD msgSize, WCHAR* msg)
{
	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	int roomNo = MakePacketResponseChat(&networkPacketHeader, &serializationBuffer, sock, msgSize, msg);

	SendBroadcastRoom(sock, networkPacketHeader, &serializationBuffer, roomNo);
}

// 11. Request 방 퇴장 처리 함수
void RecvRequestRoomLeave(SOCKET sock, SerializationBuffer* serializationBuffer)
{
	wprintf(L"Recv : 11 - 방 퇴장 요청 [NO : %d]\n", socketInfoMap.find(sock)->second->userNo);

	SendResponseRoomLeave(sock);
}

// 12. Response 방 퇴장 (수시) 처리 함수
void SendResponseRoomLeave(SOCKET sock)
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
void SendResponseRoomDelete(SOCKET sock, int roomNo)
{
	wprintf(L"Send : 13 - 방 삭제 응답 [NO : %d]\n", socketInfoMap.find(sock)->second->userNo);

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
void SendResponseUserEnter(SOCKET sock, SOCKETINFO* socketInfo)
{
	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	MakePacketResponseUserEnter(&networkPacketHeader, &serializationBuffer,	sock, socketInfo);
	
	SendBroadcastRoom(sock, networkPacketHeader, &serializationBuffer, socketInfo->enterRoomNo);
}

// 소켓 정보를 추가하는 함수이다.
BOOL AddSocketInfo(SOCKET sock)
{	
	static int userNum = 1;

	if (totalSockets > SOCKETINFO_ARRAY_MAX)
	{
		wprintf(L"[오류] 소켓 정보를 추가할 수 없습니다!\n");

		return FALSE;
	}

	SOCKETINFO* ptr = new SOCKETINFO;

	ptr->userNo = userNum++;
	ptr->alreadyRoom = false;
	ptr->enterRoomNo = -1;
	
	wprintf(L"소켓 연결 [NO : %d]\n", ptr->userNo);

	socketInfoMap.insert(map<SOCKET, SOCKETINFO*>::value_type(sock, ptr));
	//socketInfoMap[sock] = ptr; // -> 메모리를 추가 할당하기 때문에 사용하지 말 것.

	totalSockets++;

	return TRUE;
}

// 소켓 정보를 삭제하는 함수이다.
map<SOCKET, SOCKETINFO*>::iterator RemoveSocketInfo(SOCKET sock)
{	
	map<SOCKET, SOCKETINFO*>::iterator iter = socketInfoMap.find(sock);

	wprintf(L"연결 해제 [NO : %d]\n", iter->second->userNo);
	
	if (iter != socketInfoMap.end())
	{
		delete iter->second;

		closesocket(sock);

		iter = socketInfoMap.erase(iter);
		totalSockets--;
	}

	return iter;
}

// 방을 찾는 함수
ROOMINFO* FindRoom(int roomNo)
{
	list<ROOMINFO*>::iterator roomInfoListIter;

	for (roomInfoListIter = roomInfoList.begin(); roomInfoListIter != roomInfoList.end();
		++roomInfoListIter)
	{
		if ((*roomInfoListIter)->roomNo == roomNo)
		{
			return (*roomInfoListIter);
		}
	}

	return nullptr;
}

// 그 사람에게만 송신하는 함수
void SendUnicast(SOCKET sock, NetworkPacketHeader networkPacketHeader, 
	SerializationBuffer* serializationBuffer)
{
	map<SOCKET, SOCKETINFO*>::iterator iter = socketInfoMap.find(sock);

	iter->second->sendQueue.Enqueue((BYTE*)&networkPacketHeader, sizeof(networkPacketHeader));
	iter->second->sendQueue.Enqueue(serializationBuffer->GetFrontPosBufferPtr(), 
		serializationBuffer->GetUseSize());
}

// 그 사람에게만 송신하는 함수
void SendUnicast(SOCKETINFO* socketInfo, NetworkPacketHeader networkPacketHeader, 
	SerializationBuffer* serializationBuffer)
{
	socketInfo->sendQueue.Enqueue((BYTE*)&networkPacketHeader, sizeof(networkPacketHeader));
	socketInfo->sendQueue.Enqueue(serializationBuffer->GetFrontPosBufferPtr(), 
		serializationBuffer->GetUseSize());
}

// 모든 사람들에게 송신하는 함수
void SendBroadcast(NetworkPacketHeader networkPacketHeader, SerializationBuffer* serializationBuffer)
{
	map<SOCKET, SOCKETINFO*>::iterator iter;

	for (iter = socketInfoMap.begin(); iter != socketInfoMap.end(); ++iter)
	{
		SendUnicast(iter->first, networkPacketHeader, serializationBuffer);
	}
}

// 방에 있는 사람들에게 송신하는 함수
void SendBroadcastRoom(SOCKET exceptSock, NetworkPacketHeader networkPacketHeader, 
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
	int exceptSockUserNo = socketInfoMapIter->second->userNo;

	for (sockInfoListIter = sockInfo->begin(); sockInfoListIter != sockInfo->end(); ++sockInfoListIter)
	{
		if ((*sockInfoListIter)->userNo != exceptSockUserNo)
		{
			SendUnicast(*sockInfoListIter, networkPacketHeader, serializationBuffer);
		}		
	}
}

// 소켓 함수 오류 출력 후 종료
void ErrorQuit(WCHAR* msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
}