#include "Protocol.h"
#include "main.h"
#include "Select.h"
#include "MakePacket.h"

// 2. Response 로그인 처리 패킷을 만드는 함수
void MakePacketResponseLogin(NetworkPacketHeader* networkPacketHeader,
	SerializationBuffer* serializationBuffer, BYTE response, int userNo)
{
	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->MsgType = RESPONSE_LOGIN;	

	*serializationBuffer << response << userNo;

	networkPacketHeader->PayloadSize = serializationBuffer->GetUseSize();
	networkPacketHeader->checkSum = MakeCheckSum(networkPacketHeader->MsgType,
		networkPacketHeader->PayloadSize, serializationBuffer);
}

// 4. Response 대화방 리스트 처리 함수
void MakePacketResponseRoomList(NetworkPacketHeader* networkPacketHeader,
	SerializationBuffer* serializationBuffer)
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
			*serializationBuffer << (*sockInfoIter)->nickname;
		}
	}

	networkPacketHeader->PayloadSize = serializationBuffer->GetUseSize();
	networkPacketHeader->checkSum = MakeCheckSum(networkPacketHeader->MsgType,
		networkPacketHeader->PayloadSize, serializationBuffer);
}

// 6. Response 대화방 생성 (수시) 처리 함수
void MakePacketResponseRoomCreate(NetworkPacketHeader* networkPacketHeader,
	SerializationBuffer* serializationBuffer, BYTE response, ROOMINFO* roomInfoStruct)
{
	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->MsgType = RESPONSE_ROOM_CREATE;

	WORD roomTitleSize = roomInfoStruct->roomTitleSize;
	
	*serializationBuffer << response << roomInfoStruct->roomNo << roomTitleSize;

	serializationBuffer->Enqueue((BYTE*)roomInfoStruct->roomTitle, roomTitleSize);

	networkPacketHeader->PayloadSize = serializationBuffer->GetUseSize();

	networkPacketHeader->checkSum = MakeCheckSum(networkPacketHeader->MsgType,
		networkPacketHeader->PayloadSize, serializationBuffer);
}

// 8. Response 대화방 입장 처리 함수
bool MakePacketResponseRoomEnter(NetworkPacketHeader* networkPacketHeader,
	SerializationBuffer* serializationBuffer, BYTE response, ROOMINFO* roomInfoPtr)
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
			*serializationBuffer << (*socketInfoIter)->nickname << (*socketInfoIter)->userNo;
		}

		networkPacketHeader->PayloadSize = serializationBuffer->GetUseSize();

		networkPacketHeader->checkSum = MakeCheckSum(networkPacketHeader->MsgType,
			networkPacketHeader->PayloadSize, serializationBuffer);

		return true;
	}

	return false;
}

// 10. Response 채팅 수신 (수시) (나에겐 오지 않음) 처리 함수
int MakePacketResponseChat(NetworkPacketHeader* networkPacketHeader,
	SerializationBuffer* serializationBuffer, SOCKET sock, WORD msgSize, WCHAR* msg)
{
	map<SOCKET, SOCKETINFO*>::iterator socketInfoMapIter = socketInfoMap.find(sock);

	SOCKETINFO* tmpSocketInfo = socketInfoMapIter->second;

	wprintf(L"Send : 10 - 채팅 수신 [NO : %d]\n", tmpSocketInfo->userNo);

	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->MsgType = RESPONSE_CHAT;

	*serializationBuffer << tmpSocketInfo->userNo << msgSize;

	serializationBuffer->Enqueue((BYTE*)msg, msgSize);

	networkPacketHeader->PayloadSize = serializationBuffer->GetUseSize();
	networkPacketHeader->checkSum = MakeCheckSum(networkPacketHeader->MsgType, 
		networkPacketHeader->PayloadSize, serializationBuffer);

	return tmpSocketInfo->enterRoomNo;
}

// 12. Response 방 퇴장 (수시) 처리 함수
int MakePacketResponseRoomLeave(NetworkPacketHeader* networkPacketHeader,
	SerializationBuffer* serializationBuffer, SOCKET sock)
{
	map<SOCKET, SOCKETINFO*>::iterator socketInfoMapIter = socketInfoMap.find(sock);

	SOCKETINFO* tmpSocketInfo = socketInfoMapIter->second;

	int userNo = tmpSocketInfo->userNo;

	wprintf(L"Send : 12 - 방 퇴장 응답 [NO : %d]\n", userNo);
	
	*serializationBuffer << userNo;

	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->MsgType = RESPONSE_ROOM_LEAVE;				
	networkPacketHeader->PayloadSize = serializationBuffer->GetUseSize();
	networkPacketHeader->checkSum = MakeCheckSum(networkPacketHeader->MsgType, 
		networkPacketHeader->PayloadSize, serializationBuffer);

	return tmpSocketInfo->enterRoomNo;
}

// 13. Response 방 삭제 (수시) 처리 함수
void MakePacketResponseRoomDelete(NetworkPacketHeader* networkPacketHeader,
	SerializationBuffer* serializationBuffer, int roomNo)
{
	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->MsgType = RESPONSE_ROOM_DELETE;

	*serializationBuffer << roomNo;

	networkPacketHeader->PayloadSize = serializationBuffer->GetUseSize();
	networkPacketHeader->checkSum = MakeCheckSum(networkPacketHeader->MsgType, 
		networkPacketHeader->PayloadSize, serializationBuffer);
}

// 14. Response 타 사용자 입장 (수시) 처리 함수
void MakePacketResponseUserEnter(NetworkPacketHeader* networkPacketHeader,
	SerializationBuffer* serializationBuffer, SOCKET sock, SOCKETINFO* socketInfo)
{
	int userNo = socketInfo->userNo;

	wprintf(L"Send : 14 - 다른 유저 입장 응답 [NO : %d]\n", userNo);
	
	networkPacketHeader->code = NETWORK_PACKET_CODE;
	networkPacketHeader->MsgType = RESPONSE_USER_ENTER;

	serializationBuffer->Enqueue((BYTE*)socketInfo->nickname, sizeof(WCHAR) * NICK_MAX_LEN);
	*serializationBuffer << userNo;

	networkPacketHeader->PayloadSize = serializationBuffer->GetUseSize();
	networkPacketHeader->checkSum = MakeCheckSum(networkPacketHeader->MsgType,
		networkPacketHeader->PayloadSize, serializationBuffer);
}