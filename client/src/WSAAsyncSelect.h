#pragma once

#include "Protocol.h"
#include "SerializationBuffer.h"

// 소켓 관련 윈도우 메시지 처리
void ProcessSocketMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// FD_WRITE 처리 함수
void FDWriteProc();

// FD_READ 처리 함수
void FDReadProc();

// Packet을 처리하는 함수
void PacketProc(WORD msgType, SerializationBuffer* serializationBuffer);

// CheckSum을 만드는 함수
int MakeCheckSum(WORD msgType, WORD payloadSize, SerializationBuffer* serializationBuffer);

// CheckSum을 만드는 함수
int MakeCheckSum(WORD msgType);

// 1. Request 로그인 처리 함수
void SendRequestLogin(WCHAR *id, WCHAR *pw);

// 2. Response 로그인 처리 함수
void RecvResponseLogin(SerializationBuffer* serializationBuffer);

// 3. Request 대화방 리스트 처리 함수
void SendRequestRoomList();

// 4. Response 대화방 리스트 처리 함수
void RecvResponseRoomList(SerializationBuffer* serializationBuffer);

// 5. Request 대화방 생성 처리 함수
void SendRequestRoomCreate();

// 6. Response 대화방 생성 (수시) 처리 함수
void RecvResponseRoomCreate(SerializationBuffer* serializationBuffer);

// 7. Request 대화방 입장 처리 함수
void SendRequestRoomEnter(int index);

// 8. Response 대화방 입장 처리 함수
void RecvResponseRoomEnter(SerializationBuffer* serializationBuffer);

// 9. Request 채팅 송신 처리 함수
void SendRequestChat();

// 10. Response 채팅 수신 (수시) (나에겐 오지 않음) 처리 함수
void RecvResponseChat(SerializationBuffer* serializationBuffer);

// 11. Request 방 퇴장 처리 함수
void SendRequestRoomLeave();

// 12. Response 방 퇴장 (수시) 처리 함수
void RecvResponseRoomLeave(SerializationBuffer* serializationBuffer);

// 13. Response 방 삭제 (수시) 처리 함수
void RecvResponseRoomDelete(SerializationBuffer* serializationBuffer);

// 14. Response 타 사용자 입장 (수시) 처리 함수
void RecvResponseUserEnter(SerializationBuffer* serializationBuffer);

// 15. Request 회원 가입 요청 함수
void SendRequestJoin();

// 16. Response 회원 가입 처리 함수
void RecvResponseJoin(SerializationBuffer *serializationBuffer);

// 17. Request 회원 정보 수정 요청 함수
void SendRequestEditInfo();

// 18. Response 회원 정보 수정 처리 함수
void RecvResponseEditInfo(SerializationBuffer *serializationBuffer);

// 서버로 패킷을 보내는 함수
void SendToServer(NetworkPacketHeader networkPacketHeader);

// 서버로 패킷을 보내는 함수
void SendToServer(NetworkPacketHeader networkPacketHeader,
	SerializationBuffer* serializationBuffer);

// 소켓 함수 오류 출력 후 종료
void ErrorQuit(WCHAR* msg);

// 소켓 함수 오류 출력
void ErrorDisplay(WCHAR* msg);