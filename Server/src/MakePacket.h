#pragma once

// 2. Response 로그인 처리 패킷을 만드는 함수
void MakePacketResponseLogin(NetworkPacketHeader* networkPacketHeader,
	SerializationBuffer* serializationBuffer, BYTE response, int userNo);

// 4. Response 대화방 리스트 처리 함수
void MakePacketResponseRoomList(NetworkPacketHeader* networkPacketHeader,
	SerializationBuffer* serializationBuffer);

// 6. Response 대화방 생성 (수시) 처리 함수
void MakePacketResponseRoomCreate(NetworkPacketHeader* networkPacketHeader,
	SerializationBuffer* serializationBuffer, BYTE response, ROOMINFO* roomInfoStruct);

// 8. Response 대화방 입장 처리 함수
bool MakePacketResponseRoomEnter(NetworkPacketHeader* networkPacketHeader,
	SerializationBuffer* serializationBuffer, BYTE response, ROOMINFO* roomInfoPtr);

// 10. Response 채팅 수신 (수시) (나에겐 오지 않음) 처리 함수
int MakePacketResponseChat(NetworkPacketHeader* networkPacketHeader,
	SerializationBuffer* serializationBuffer, SOCKET sock, WORD msgSize, WCHAR* msg);

// 12. Response 방 퇴장 (수시) 처리 함수
int MakePacketResponseRoomLeave(NetworkPacketHeader* networkPacketHeader,
	SerializationBuffer* serializationBuffer, SOCKET sock);

// 13. Response 방 삭제 (수시) 처리 함수
void MakePacketResponseRoomDelete(NetworkPacketHeader* networkPacketHeader,
	SerializationBuffer* serializationBuffer, int roomNo);

// 14. Response 타 사용자 입장 (수시) 처리 함수
void MakePacketResponseUserEnter(NetworkPacketHeader* networkPacketHeader,
	SerializationBuffer* serializationBuffer, SOCKET sock, SOCKETINFO* socketInfo);