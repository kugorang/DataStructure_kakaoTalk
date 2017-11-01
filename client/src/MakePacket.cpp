#include "stdafx.h"
#include "Main.h"
#include "WSAAsyncSelect.h"
#include "MakePacket.h"

// 1. Request 로그인 처리 패킷을 만드는 함수
void MakePacketRequestLogin(NetworkPacketHeader *networkPacketHeader,
	SerializationBuffer *serializationBuffer, WCHAR *id, WCHAR *pw)
{
	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->msgType = REQUEST_LOGIN;

	serializationBuffer->Enqueue((BYTE*)id, ID_MAX_LEN * sizeof(WCHAR));
	serializationBuffer->Enqueue((BYTE*)pw, PW_MAX_LEN * sizeof(WCHAR));

	networkPacketHeader->payloadSize = serializationBuffer->GetUseSize();
	networkPacketHeader->checkSum = MakeCheckSum(REQUEST_LOGIN, networkPacketHeader->payloadSize, serializationBuffer);
}

// 3. Request 대화방 리스트 처리 패킷을 만드는 함수
void MakePacketRequestRoomList(NetworkPacketHeader *networkPacketHeader)
{
	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->msgType = REQUEST_ROOM_LIST;
	networkPacketHeader->payloadSize = 0;
	networkPacketHeader->checkSum = MakeCheckSum(REQUEST_ROOM_LIST);
}

// 5. Request 대화방 생성 처리 패킷을 만드는 함수
void MakePacketRequestRoomCreate(NetworkPacketHeader *networkPacketHeader,
	SerializationBuffer *serializationBuffer, WORD roomNameSize, WCHAR* roomName)
{
	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->msgType = REQUEST_ROOM_CREATE;

	*serializationBuffer << roomNameSize;

	serializationBuffer->Enqueue((BYTE*)roomName, roomNameSize);

	networkPacketHeader->payloadSize = serializationBuffer->GetUseSize();
	networkPacketHeader->checkSum = MakeCheckSum(REQUEST_ROOM_CREATE,
		networkPacketHeader->payloadSize, serializationBuffer);
}

// 7. Request 대화방 입장 처리 패킷을 만드는 함수
void MakePacketRequestRoomEnter(NetworkPacketHeader *networkPacketHeader,
	SerializationBuffer *serializationBuffer, int roomNo)
{
	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->msgType = REQUEST_ROOM_ENTER;

	*serializationBuffer << roomNo;

	networkPacketHeader->payloadSize = serializationBuffer->GetUseSize();
	networkPacketHeader->checkSum = MakeCheckSum(REQUEST_ROOM_ENTER,
		networkPacketHeader->payloadSize, serializationBuffer);
}

// 9. Request 채팅 송신 처리 패킷을 만드는 함수
void MakePacketRequestChat(NetworkPacketHeader *networkPacketHeader,
	SerializationBuffer *serializationBuffer, WORD chatSize, WCHAR *chatMsg)
{
	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->msgType = REQUEST_CHAT;

	*serializationBuffer << chatSize;

	serializationBuffer->Enqueue((BYTE*)chatMsg, chatSize);

	networkPacketHeader->payloadSize = serializationBuffer->GetUseSize();
	networkPacketHeader->checkSum = MakeCheckSum(REQUEST_CHAT,
		networkPacketHeader->payloadSize, serializationBuffer);
}

// 11. Request 방 퇴장 처리 패킷을 만드는 함수
void MakePacketRequestRoomLeave(NetworkPacketHeader *networkPacketHeader)
{
	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->msgType = REQUEST_ROOM_LEAVE;
	networkPacketHeader->payloadSize = 0;
	networkPacketHeader->checkSum = MakeCheckSum(REQUEST_ROOM_LEAVE);
}

// 15. Request 회원 가입 처리 패킷을 만드는 함수
void MakePacketRequestJoin(NetworkPacketHeader *networkPacketHeader, SerializationBuffer *serializationBuffer, WCHAR *id, WCHAR *pw, WCHAR *name, WCHAR *phoneNum)
{
	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->msgType = REQUEST_JOIN;

	serializationBuffer->Enqueue((BYTE*)id, ID_MAX_LEN * sizeof(WCHAR));
	serializationBuffer->Enqueue((BYTE*)pw, PW_MAX_LEN * sizeof(WCHAR));
	serializationBuffer->Enqueue((BYTE*)name, NAME_MAX_LEN * sizeof(WCHAR));
	serializationBuffer->Enqueue((BYTE*)phoneNum, PHONENUM_MAX_LEN * sizeof(WCHAR));

	networkPacketHeader->payloadSize = serializationBuffer->GetUseSize();
	networkPacketHeader->checkSum = MakeCheckSum(REQUEST_JOIN, networkPacketHeader->payloadSize, serializationBuffer);
}

// 17. Request 회원 정보 수정 패킷을 만드는 함수
void MakePacketRequestEditInfo(NetworkPacketHeader *networkPacketHeader, SerializationBuffer *serializationBuffer, WCHAR *id, WCHAR *pw, WCHAR *name, WCHAR *phoneNum)
{
	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->msgType = REQUEST_EDIT_INFO;

	serializationBuffer->Enqueue((BYTE*)id, ID_MAX_LEN * sizeof(WCHAR));
	serializationBuffer->Enqueue((BYTE*)pw, PW_MAX_LEN * sizeof(WCHAR));
	serializationBuffer->Enqueue((BYTE*)name, NAME_MAX_LEN * sizeof(WCHAR));
	serializationBuffer->Enqueue((BYTE*)phoneNum, PHONENUM_MAX_LEN * sizeof(WCHAR));

	networkPacketHeader->payloadSize = serializationBuffer->GetUseSize();
	networkPacketHeader->checkSum = MakeCheckSum(REQUEST_EDIT_INFO, networkPacketHeader->payloadSize, serializationBuffer);
}