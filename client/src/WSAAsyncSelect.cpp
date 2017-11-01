#include "stdafx.h"
#include "Main.h"
#include "WSAAsyncSelect.h"
#include "MakePacket.h"

bool createdByMe = false;

// 소켓 관련 윈도우 메시지 처리
void ProcessSocketMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int retval;

	// 오류 발생 여부 확인
	if (WSAGETSELECTERROR(lParam))
	{
		ErrorQuit(L"WSAGETSELECTERROR(lParam)");
		exit(6001);
		//PostQuitMessage(0);
	}

	// 메세지 처리
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_CONNECT:
		retval = WSAAsyncSelect((SOCKET)wParam, hWnd, WM_SOCKET,
			FD_READ | FD_WRITE | FD_CLOSE);

		if (retval == SOCKET_ERROR)
		{
			ErrorQuit(L"WSAAsyncSelect()");
			exit(6002);
			//PostQuitMessage(0);
		}
		break;
	case FD_READ:
		FDReadProc();
		break;
	case FD_WRITE:
		FDWriteProc();
		break;
	case FD_CLOSE:
		closesocket(socketInfo.linkSock);
		exit(6003);
		//PostQuitMessage(0);
		break;
	default:
		break;
	}
}

void FDWriteProc()
{
	int retval, useSize, notBrokenGetSize;
	RingBuffer* sendQueue = &(socketInfo.sendQueue);

	while (true)
	{
		useSize = sendQueue->GetUseSize();
		notBrokenGetSize = sendQueue->GetNotBrokenGetSize();

		if (useSize <= 0)
		{
			return;
		}

		retval = send(socketInfo.linkSock, (char*)sendQueue->GetFrontPosBufferPtr(), notBrokenGetSize, 0);

		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				ErrorQuit(L"FDWriteProc()");
				exit(6004);
				//PostQuitMessage(0);
			}
		}
		else
		{
			sendQueue->MoveFrontPos(retval);
		}
	}
}

void FDReadProc()
{
	int retval, useSize, remainSize, notBrokenPutSize;
	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;
	RingBuffer* recvQueue = &(socketInfo.recvQueue);

	remainSize = recvQueue->GetRemainSize();
	notBrokenPutSize = recvQueue->GetNotBrokenPutSize();

	if (remainSize <= 0)
	{
		return;
	}

	retval = recv(socketInfo.linkSock, (char*)recvQueue->GetRearPosBufferPtr(), notBrokenPutSize, 0);

	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			ErrorQuit(L"FDReadProc()");
			exit(6005);
			//PostQuitMessage(0);
		}
	}
	else if (retval == 0)
	{
		return;
	}
	else
	{
		recvQueue->MoveRearPos(retval);
	}

	while (true)
	{
		useSize = recvQueue->GetUseSize();

		if (sizeof(networkPacketHeader) >= useSize)
		{
			return;
		}

		recvQueue->Peek((BYTE*)&networkPacketHeader, sizeof(networkPacketHeader));

		if ((int)(sizeof(networkPacketHeader) + networkPacketHeader.payloadSize) > useSize
			|| networkPacketHeader.code != NETWORK_PACKET_CODE)
		{
			return;
		}

		recvQueue->MoveFrontPos(sizeof(networkPacketHeader));

		retval = recvQueue->Dequeue(serializationBuffer.GetRearPosBufferPtr(),
			networkPacketHeader.payloadSize);

		serializationBuffer.MoveRearPos(retval);

		if (MakeCheckSum(networkPacketHeader.msgType, networkPacketHeader.payloadSize,
			&serializationBuffer) == networkPacketHeader.checkSum)
		{
			PacketProc(networkPacketHeader.msgType, &serializationBuffer);
		}
		else
		{
			wprintf(L"CheckSum 에러 - ");
			PostQuitMessage(0);
		}
	}
}

void PacketProc(WORD msgType, SerializationBuffer* serializationBuffer)
{
	switch (msgType)
	{
	case RESPONSE_LOGIN:
		RecvResponseLogin(serializationBuffer);
		break;
	case RESPONSE_ROOM_LIST:
		RecvResponseRoomList(serializationBuffer);
		break;
	case RESPONSE_ROOM_CREATE:
		RecvResponseRoomCreate(serializationBuffer);
		break;
	case RESPONSE_ROOM_ENTER:
		RecvResponseRoomEnter(serializationBuffer);
		break;
	case RESPONSE_CHAT:
		RecvResponseChat(serializationBuffer);
		break;
	case RESPONSE_ROOM_LEAVE:
		RecvResponseRoomLeave(serializationBuffer);
		break;
	case RESPONSE_ROOM_DELETE:
		RecvResponseRoomDelete(serializationBuffer);
		break;
	case RESPONSE_USER_ENTER:
		RecvResponseUserEnter(serializationBuffer);
		break;
	case RESPONSE_JOIN:
		RecvResponseJoin(serializationBuffer);
		break;
	case RESPONSE_EDIT_INFO:
		RecvResponseEditInfo(serializationBuffer);
		break;
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

// CheckSum을 만드는 함수
int MakeCheckSum(WORD msgType)
{
	WORD byteSum = 0;

	for (int i = 0; i < 2; i++)
	{
		byteSum += *((BYTE*)&msgType + i);
	}

	return byteSum % 256;
}

// 1. Request 로그인 처리 함수
void SendRequestLogin(WCHAR* id, WCHAR *pw)
{
	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	MakePacketRequestLogin(&networkPacketHeader, &serializationBuffer, id, pw);

	SendToServer(networkPacketHeader, &serializationBuffer);
}

// 2. Response 로그인 처리 함수
void RecvResponseLogin(SerializationBuffer* serializationBuffer)
{
	BYTE result;
	WCHAR id[ID_MAX_LEN], name[NAME_MAX_LEN];

	*serializationBuffer >> result;

	switch (result)
	{
	case RESPONSE_LOGIN_OK:
		serializationBuffer->Dequeue((BYTE*)id, sizeof(id));
		serializationBuffer->Dequeue((BYTE*)name, sizeof(name));

		SetDlgItemText(hWndLobby, IDC_USERNO, id);
		SetDlgItemText(hWndLobby, IDC_USERNICKNAME, name);

		SetDlgItemText(hWndNoti, IDC_NOTI_TXT, L"로그인에 성공했습니다.");
		SendRequestRoomList();
		break;
	case RESPONSE_LOGIN_PW_ERR:
		SetDlgItemText(hWndNoti, IDC_NOTI_TXT, L"비밀번호를 확인해주시기 바랍니다.");
		cancelFlag = true;
		break;
	case RESPONSE_LOGIN_ID_ERR:
		SetDlgItemText(hWndNoti, IDC_NOTI_TXT, L"존재하지 않는 아이디입니다.");
		cancelFlag = true;
		break;
	case RESPONSE_LOGIN_ALREADY:
		SetDlgItemText(hWndNoti, IDC_NOTI_TXT, L"이미 로그인한 사용자입니다.");
		cancelFlag = true;
		break;
	case RESPONSE_LOGIN_ETC:
		SetDlgItemText(hWndNoti, IDC_NOTI_TXT, L"알 수 없는 이유로 로그인에 실패했습니다.");
		cancelFlag = true;
		break;
	default:
		break;
	}

	ShowWindow(hWndNoti, cmdShow);
	UpdateWindow(hWndNoti);
}

// 3. Request 대화방 리스트 처리 함수
void SendRequestRoomList()
{
	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	MakePacketRequestRoomList(&networkPacketHeader);

	SendToServer(networkPacketHeader);
}

// 4. Response 대화방 리스트 처리 함수
void RecvResponseRoomList(SerializationBuffer* serializationBuffer)
{
	WORD roomNum;

	*serializationBuffer >> roomNum;

	for (WORD i = 0; i < roomNum; i++)
	{
		int roomNo;
		WORD roomNameSize;
		BYTE numberOfPeople;

		*serializationBuffer >> roomNo >> roomNameSize;

		WCHAR* roomName = new WCHAR[roomNameSize + 1];
		memset(roomName, 0, roomNameSize);

		serializationBuffer->Dequeue((BYTE*)roomName, roomNameSize);

		int index = (int)SendMessage(lobbyRoomList, LB_ADDSTRING, 0, (LPARAM)roomName);
		SendMessage(lobbyRoomList, LB_SETITEMDATA, index, roomNo);
		SendMessage(lobbyRoomList, LB_SETCOUNT, ++totalRoomNum, 0);

		*serializationBuffer >> numberOfPeople;

		for (BYTE j = 0; j < numberOfPeople; j++)
		{
			WCHAR name[NAME_MAX_LEN];

			serializationBuffer->Dequeue((BYTE*)name, sizeof(name));
		}

		delete[] roomName;
	}
}

// 5. Request 대화방 생성 처리 함수
void SendRequestRoomCreate()
{
	WCHAR roomName[256];

	GetDlgItemText(hWndLobby, IDC_INPUTROOMNAME, roomName, sizeof(roomName));

	WORD roomNameSize = (WORD)(wcslen(roomName) * sizeof(WCHAR));

	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	MakePacketRequestRoomCreate(&networkPacketHeader, &serializationBuffer, roomNameSize, roomName);

	createdByMe = true;

	SendToServer(networkPacketHeader, &serializationBuffer);
}

// 6. Response 대화방 생성 (수시) 처리 함수
void RecvResponseRoomCreate(SerializationBuffer* serializationBuffer)
{
	BYTE result;
	int roomNo;
	WORD roomNameSize;

	*serializationBuffer >> result;

	if (result == RESPONSE_ROOM_CREATE_OK)
	{
		*serializationBuffer >> roomNo >> roomNameSize;

		WCHAR* roomName = new WCHAR[roomNameSize];

		serializationBuffer->Dequeue((BYTE*)roomName, roomNameSize);

		int index = (int)SendMessage(lobbyRoomList, LB_ADDSTRING, 0, (LPARAM)roomName);
		SendMessage(lobbyRoomList, LB_SETITEMDATA, index, roomNo);
		SendMessage(lobbyRoomList, LB_SETCOUNT, ++totalRoomNum, 0);

		delete[] roomName;

		if (createdByMe)
		{
			createdByMe = false;
			SendRequestRoomEnter(index);
		}
	}
}

// 7. Request 대화방 입장 처리 함수
void SendRequestRoomEnter(int index)
{
	int roomNo = (int)SendMessage(lobbyRoomList, LB_GETITEMDATA, index, 0);

	if (roomNo != LB_ERR)
	{
		NetworkPacketHeader networkPacketHeader;
		SerializationBuffer serializationBuffer;

		MakePacketRequestRoomEnter(&networkPacketHeader, &serializationBuffer, roomNo);

		SendToServer(networkPacketHeader, &serializationBuffer);
	}
}

// 8. Response 대화방 입장 처리 함수
void RecvResponseRoomEnter(SerializationBuffer* serializationBuffer)
{
	BYTE result;

	*serializationBuffer >> result;

	if (result == RESPONSE_ROOM_ENTER_OK)
	{
		int roomNo;
		WORD roomNameSize;
		BYTE numberOfPeople;

		*serializationBuffer >> roomNo >> roomNameSize;

		WCHAR* roomName = new WCHAR[roomNameSize + 1];
		memset(roomName, 0, roomNameSize);

		serializationBuffer->Dequeue((BYTE*)roomName, roomNameSize);
		*serializationBuffer >> numberOfPeople;

		SetDlgItemInt(hWndRoom, IDC_ROOMNO, roomNo, false);
		SetDlgItemText(hWndRoom, IDC_ROOMNAME, roomName);
		SetDlgItemInt(hWndRoom, IDC_NUMOFPEOPLE, numberOfPeople, false);

		for (BYTE j = 0; j < numberOfPeople; j++)
		{
			WCHAR nickname[15];
			int userNo;

			serializationBuffer->Dequeue((BYTE*)nickname, sizeof(nickname));
			*serializationBuffer >> userNo;

			int index = (int)SendMessage(roomPeopleList, LB_ADDSTRING, 0, (LPARAM)nickname);
			SendMessage(roomPeopleList, LB_SETITEMDATA, index, userNo);
		}

		delete[] roomName;

		SendMessage(roomPeopleList, LB_SETCOUNT, numberOfPeople, 0);

		ShowWindow(hWndRoom, cmdShow);
		UpdateWindow(hWndRoom);
	}
}

// 9. Request 채팅 송신 처리 함수
void SendRequestChat()
{
	WCHAR chatMsg[256];

	wsprintf(chatMsg, L"%s : ", inputName);

	GetDlgItemText(hWndRoom, IDC_INPUTCHAT, chatMsg + wcslen(chatMsg), sizeof(chatMsg));

	WORD chatSize = (WORD)(wcslen(chatMsg) * sizeof(WCHAR));

	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	MakePacketRequestChat(&networkPacketHeader, &serializationBuffer, chatSize, chatMsg);

	SendToServer(networkPacketHeader, &serializationBuffer);

	SendMessage(roomChatList, LB_ADDSTRING, 0, (LPARAM)chatMsg);
}

// 10. Response 채팅 수신 (수시) (나에겐 오지 않음) 처리 함수
void RecvResponseChat(SerializationBuffer* serializationBuffer)
{
	int userNo;
	WORD chatSize;

	*serializationBuffer >> userNo >> chatSize;

	WCHAR* chatMsg = new WCHAR[chatSize + 1];
	memset(chatMsg, 0, chatSize);

	serializationBuffer->Dequeue((BYTE*)chatMsg, chatSize);

	int numOfPeople = (int)SendMessage(roomPeopleList, LB_GETCOUNT, 0, 0);

	for (int index = 0; index < numOfPeople; index++)
	{
		if (SendMessage(roomPeopleList, LB_GETITEMDATA, (WPARAM)index, 0) == userNo)
		{
			SendMessage(roomChatList, LB_ADDSTRING, 0, (LPARAM)chatMsg);
			break;
		}
	}
}

// 11. Request 방 퇴장 처리 함수
void SendRequestRoomLeave()
{
	NetworkPacketHeader networkPacketHeader;

	MakePacketRequestRoomLeave(&networkPacketHeader);

	SendToServer(networkPacketHeader);
}

// 12. Response 방 퇴장 (수시) 처리 함수
void RecvResponseRoomLeave(SerializationBuffer* serializationBuffer)
{
	int userNo;

	*serializationBuffer >> userNo;

	int listNum = (int)SendMessage(roomPeopleList, LB_GETCOUNT, 0, 0);

	for (int index = 0; index < listNum; index++)
	{
		if (SendMessage(roomPeopleList, LB_GETITEMDATA, (WPARAM)index, 0) == userNo)
		{
			SendMessage(roomPeopleList, LB_DELETESTRING, index, 0);
			SendMessage(roomPeopleList, LB_SETCOUNT, --listNum, 0);
			SetDlgItemInt(hWndRoom, IDC_NUMOFPEOPLE, listNum, false);
			break;
		}
	}
}

// 13. Response 방 삭제 (수시) 처리 함수
void RecvResponseRoomDelete(SerializationBuffer* serializationBuffer)
{
	int roomNo;

	*serializationBuffer >> roomNo;

	int listNum = (int)SendMessage(lobbyRoomList, LB_GETCOUNT, 0, 0);

	for (int index = 0; index < listNum; index++)
	{
		if (SendMessage(lobbyRoomList, LB_GETITEMDATA, (WPARAM)index, 0) == roomNo)
		{
			SendMessage(lobbyRoomList, LB_DELETESTRING, index, 0);
			SendMessage(lobbyRoomList, LB_SETCOUNT, --listNum, 0);
			break;
		}
	}
}

// 14. Response 타 사용자 입장 (수시) 처리 함수
void RecvResponseUserEnter(SerializationBuffer* serializationBuffer)
{
	WCHAR nickname[15];
	int userNo;

	serializationBuffer->Dequeue((BYTE*)nickname, sizeof(nickname));
	*serializationBuffer >> userNo;

	int index = (int)SendMessage(roomPeopleList, LB_ADDSTRING, 0, (LPARAM)nickname);
	SendMessage(roomPeopleList, LB_SETITEMDATA, index, userNo);
	SetDlgItemInt(hWndRoom, IDC_NUMOFPEOPLE, (int)SendMessage(roomPeopleList, LB_GETCOUNT, 0, 0), false);
}

// 15. Request 회원 가입 요청 함수
void SendRequestJoin()
{
	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	MakePacketRequestJoin(&networkPacketHeader, &serializationBuffer, inputID, inputPW, inputName, inputPhoneNum);

	SendToServer(networkPacketHeader, &serializationBuffer);
}

// 16. Response 회원 가입 처리 함수
void RecvResponseJoin(SerializationBuffer *serializationBuffer)
{
	BYTE result;
	int userNo;

	*serializationBuffer >> result >> userNo;

	switch (result)
	{
	case RESPONSE_JOIN_OK:
		SetDlgItemText(hWndNoti, IDC_NOTI_TXT, L"회원가입이 완료되었습니다.");
		break;
	case RESPONSE_JOIN_DNICK:
		SetDlgItemText(hWndNoti, IDC_NOTI_TXT, L"이미 존재하는 아이디입니다.");
		break;
	case RESPONSE_JOIN_MAX:
		SetDlgItemText(hWndNoti, IDC_NOTI_TXT, L"회원 수 제한으로 인해 회원가입이 제한되었습니다.");
		break;
	case RESPONSE_JOIN_ETC:
		SetDlgItemText(hWndNoti, IDC_NOTI_TXT, L"알 수 없는 이유로 회원가입에 실패했습니다.");
		break;
	}

	ShowWindow(hWndNoti, cmdShow);
	UpdateWindow(hWndNoti);
}

// 17. Request 회원 정보 수정 요청 함수
void SendRequestEditInfo()
{
	NetworkPacketHeader networkPacketHeader;
	SerializationBuffer serializationBuffer;

	MakePacketRequestEditInfo(&networkPacketHeader, &serializationBuffer, inputID, inputPW, inputName, inputPhoneNum);

	SendToServer(networkPacketHeader, &serializationBuffer);
}

// 18. Response 회원 정보 수정 처리 함수
void RecvResponseEditInfo(SerializationBuffer *serializationBuffer)
{
	BYTE result;
	int userNo;

	*serializationBuffer >> result >> userNo;

	switch (result)
	{
	case RESPONSE_EDIT_INFO_OK:
		SetDlgItemText(hWndNoti, IDC_NOTI_TXT, L"회원 정보 수정을 완료했습니다.");
		SetDlgItemText(hWndLobby, IDC_USERNICKNAME, inputName);
		break;
	case RESPONSE_EDIT_INFO_DNICK:
		SetDlgItemText(hWndNoti, IDC_NOTI_TXT, L"이미 존재하는 아이디입니다.");
		break;
	case RESPONSE_EDIT_INFO_ETC:
		SetDlgItemText(hWndNoti, IDC_NOTI_TXT, L"알 수 없는 이유로 회원 정보 수정에 실패했습니다.");
		break;
	}

	ShowWindow(hWndNoti, cmdShow);
	UpdateWindow(hWndNoti);
}

// 서버로 패킷을 보내는 함수
void SendToServer(NetworkPacketHeader networkPacketHeader)
{
	socketInfo.sendQueue.Enqueue((BYTE*)&networkPacketHeader, sizeof(networkPacketHeader));

	FDWriteProc();
}

// 서버로 패킷을 보내는 함수
void SendToServer(NetworkPacketHeader networkPacketHeader,
	SerializationBuffer* serializationBuffer)
{
	socketInfo.sendQueue.Enqueue((BYTE*)&networkPacketHeader, sizeof(networkPacketHeader));
	socketInfo.sendQueue.Enqueue((BYTE*)serializationBuffer->GetFrontPosBufferPtr(),
		serializationBuffer->GetUseSize());

	FDWriteProc();
}

// 소켓 함수 오류 출력 후 종료
void ErrorQuit(WCHAR* msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
}

// 소켓 함수 오류 출력
void ErrorDisplay(WCHAR* msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	wprintf(L"[%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}