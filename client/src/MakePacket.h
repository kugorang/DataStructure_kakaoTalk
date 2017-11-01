#pragma once

// 1. Request 로그인 처리 패킷을 만드는 함수
void MakePacketRequestLogin(NetworkPacketHeader *networkPacketHeader,
	SerializationBuffer *serializationBuffer, WCHAR *id, WCHAR *pw);

// 3. Request 대화방 리스트 처리 패킷을 만드는 함수
void MakePacketRequestRoomList(NetworkPacketHeader *networkPacketHeader);

// 5. Request 대화방 생성 처리 패킷을 만드는 함수
void MakePacketRequestRoomCreate(NetworkPacketHeader *networkPacketHeader,
	SerializationBuffer *serializationBuffer, WORD roomNameSize, WCHAR* roomName);

// 7. Request 대화방 입장 처리 패킷을 만드는 함수
void MakePacketRequestRoomEnter(NetworkPacketHeader *networkPacketHeader,
	SerializationBuffer *serializationBuffer, int roomNo);

// 9. Request 채팅 송신 처리 패킷을 만드는 함수
void MakePacketRequestChat(NetworkPacketHeader *networkPacketHeader,
	SerializationBuffer *serializationBuffer, WORD chatSize, WCHAR *chatMsg);

// 11. Request 방 퇴장 처리 패킷을 만드는 함수
void MakePacketRequestRoomLeave(NetworkPacketHeader *networkPacketHeader);

// 15. Request 회원 가입 처리 패킷을 만드는 함수
void MakePacketRequestJoin(NetworkPacketHeader *networkPacketHeader, SerializationBuffer *serializationBuffer, WCHAR *id, WCHAR *pw, WCHAR *name, WCHAR *phoneNum);

// 17. Request 회원 정보 수정 패킷을 만드는 함수
void MakePacketRequestEditInfo(NetworkPacketHeader *networkPacketHeader, SerializationBuffer *serializationBuffer, WCHAR *id, WCHAR *pw, WCHAR *name, WCHAR *phoneNum);