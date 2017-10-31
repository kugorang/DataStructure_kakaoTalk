#pragma once

#include "SerializationBuffer.h"

// 소켓 함수 오류 출력 후 종료
void ErrorQuit(WCHAR* msg);

// 소켓 함수 오류 출력
void ErrorDisplay(WCHAR* msg);

// FD_WRITE 처리 함수
int FDWriteProc(SOCKET sock, SOCKETINFO* socketInfo);

// FD_READ 처리 함수
int FDReadProc(SOCKET sock, SOCKETINFO* socketInfo);

// CheckSum을 만드는 함수
int MakeCheckSum(WORD msgType, WORD payloadSize, SerializationBuffer* serializationBuffer);

// Packet을 처리하는 함수
void PacketProc(SOCKET sock, WORD type, SerializationBuffer* serializationBuffer);

// 1. Request 로그인 처리 함수
void RecvRequestLogin(SOCKET sock, SerializationBuffer* serializationBuffer);

// 2. Response 로그인 처리 함수
void SendResponseLogin(SOCKET sock, WCHAR* nickname);

// 3. Request 대화방 리스트 처리 함수
void RecvRequestRoomList(SOCKET sock, SerializationBuffer* serializationBuffer);

// 4. Response 대화방 리스트 처리 함수
void SendResponseRoomList(SOCKET sock);

// 5. Request 대화방 생성 처리 함수
void RecvRequestRoomCreate(SOCKET sock, SerializationBuffer* serializationBuffer);

// 6. Response 대화방 생성 (수시) 처리 함수
void SendResponseRoomCreate(SOCKET sock, WCHAR* roomTitle, WORD roomTitleSize);

// 7. Request 대화방 입장 처리 함수
void RecvRequestRoomEnter(SOCKET sock, SerializationBuffer* serializationBuffer);

// 8. Response 대화방 입장 처리 함수
void SendResponseRoomEnter(SOCKET sock, BYTE response, ROOMINFO* roomInfoPtr);

// 9. Request 채팅 송신 처리 함수
void RecvRequestChat(SOCKET sock, SerializationBuffer* serializationBuffer);

// 10. Response 채팅 수신 (수시) (나에겐 오지 않음) 처리 함수
void SendResponseChat(SOCKET sock, WORD msgSize, WCHAR* msg);

// 11. Request 방 퇴장 처리 함수
void RecvRequestRoomLeave(SOCKET sock, SerializationBuffer* serializationBuffer);

// 12. Response 방 퇴장 (수시) 처리 함수
void SendResponseRoomLeave(SOCKET sock);

// 13. Response 방 삭제 (수시) 처리 함수
void SendResponseRoomDelete(SOCKET sock, int roomNo);

// 14. Response 타 사용자 입장 (수시) 처리 함수
void SendResponseUserEnter(SOCKET sock, SOCKETINFO* socketInfo);

// 소켓 관리 함수
BOOL AddSocketInfo(SOCKET sock);
map<SOCKET, SOCKETINFO*>::iterator RemoveSocketInfo(SOCKET sock);

// 방을 찾는 함수
ROOMINFO* FindRoom(int roomNo);

// 그 사람에게만 송신하는 함수
void SendUnicast(SOCKET sock, 
	NetworkPacketHeader networkPacketHeader, 
	SerializationBuffer* serializationBuffer);

// 모든 사람들에게 송신하는 함수
void SendBroadcast(NetworkPacketHeader networkPacketHeader, 
	SerializationBuffer* serializationBuffer);

// 방에 있는 사람들에게 송신하는 함수
void SendBroadcastRoom(SOCKET exceptSock, 
	NetworkPacketHeader networkPacketHeader,
	SerializationBuffer* serializationBuffer,
	int roomNo);