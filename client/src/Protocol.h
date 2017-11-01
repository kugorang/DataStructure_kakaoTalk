#pragma once

// 채팅 프로토콜

#include <Windows.h>

#define NETWORK_PORT 5000

#define ID_MAX_LEN 15		// 유니코드 글자 길이 NULL 문자 포함
#define PW_MAX_LEN 15
#define NAME_MAX_LEN 15
#define PHONENUM_MAX_LEN 15
#define NETWORK_PACKET_CODE ((BYTE)0x89)

// 패킷 헤더
//------------------------------------------------------
//
//	| PacketCode | CheckSum | MsgType | PayloadSize | * Payload * |
//		1 Byte      1 Byte	  2 Bytes      2 Byte      Size Byte     
//
//	checkSum - 각 MsgType, Payload 의 각 바이트 더하기 % 256
//
//------------------------------------------------------

#pragma pack(push, 1)
struct NetworkPacketHeader	// 총 6 bytes
{
	BYTE code;
	BYTE checkSum;

	WORD msgType;
	WORD payloadSize;
};
#pragma pack(pop)

// Request = Client->Server
// Response = Server->Client

//------------------------------------------------------------
// 1. Request 로그인
//
// WCHAR[15] : 아이디 (유니코드)
// WCHAR[15] : 비밀번호 (유니코드)
//------------------------------------------------------------
#define REQUEST_LOGIN	1

//------------------------------------------------------------
// 2. Response 로그인
// 
// 1 Byte	: 결과 (1 : OK / 2 : 비밀번호 오류 / 3 : 아이디 오류 / 4 : 기타 오류)
// WCHAR[15] : 아이디 (유니코드)
// WCHAR[15] : 이름 (유니코드)
//------------------------------------------------------------
#define RESPONSE_LOGIN	2

#define RESPONSE_LOGIN_OK		1
#define RESPONSE_LOGIN_PW_ERR	2
#define RESPONSE_LOGIN_ID_ERR	3
#define RESPONSE_LOGIN_ALREADY	4
#define RESPONSE_LOGIN_ETC		5

//------------------------------------------------------------
// 3. Request 대화방 리스트
//
//	None
//------------------------------------------------------------
#define REQUEST_ROOM_LIST	3

//------------------------------------------------------------
// 4. Response 대화방 리스트
//
//  2 Bytes	: 개수
//  {
//		4 Bytes : 방 No
//		2 Bytes : 방 이름 byte size
//		Size  : 방 이름 (유니코드)
//
//		1 Byte : 참여인원		
//		{
//			WCHAR[15] : 닉네임
//		}
//	}
//------------------------------------------------------------
#define RESPONSE_ROOM_LIST	4

//------------------------------------------------------------
// 5. Request 대화방 생성
//
// 2 Bytes : 방 제목 Size			유니코드 문자 길이 (널 제외)
// Size  : 방 제목 (유니코드)
//------------------------------------------------------------
#define REQUEST_ROOM_CREATE	5

//------------------------------------------------------------
// 6. Response 대화방 생성 (수시)
//
// 1 Byte : 결과 (1 : OK / 2 : 방 이름 중복 / 3 : 개수 초과 / 4 : 기타 오류)
//
// 4 Bytes : 방 번호
// 2 Bytes : 방 제목 Size
// Size  : 방 제목 (유니코드)
//------------------------------------------------------------
#define RESPONSE_ROOM_CREATE	6

#define RESPONSE_ROOM_CREATE_OK			1
#define RESPONSE_ROOM_CREATE_DNICK		2
#define RESPONSE_ROOM_CREATE_MAX		3
#define RESPONSE_ROOM_CREATE_ETC		4

//------------------------------------------------------------
// 7. Request 대화방 입장
//
// 4 Bytes : 방 번호
//------------------------------------------------------------
#define REQUEST_ROOM_ENTER		7

//------------------------------------------------------------
// 8. Response 대화방 입장
//
// 1 Byte : 결과 (1 : OK / 2 : 방 번호 오류 / 3 : 인원 초과 / 4 : 기타 오류)
//
// OK 의 경우에만 다음 전송
//	{
//		4 Bytes : 방 No
//		2 Bytes : 방 제목 Size
//		Size  : 방 제목 (유니코드)
//
//		1 Byte : 참가인원
//		{
//			WCHAR[15] : 닉네임 (유니코드)
//			4 Bytes     : 사용자 번호
//		}
//	}
//------------------------------------------------------------
#define RESPONSE_ROOM_ENTER		8

#define RESPONSE_ROOM_ENTER_OK		1
#define RESPONSE_ROOM_ENTER_NOT		2
#define RESPONSE_ROOM_ENTER_MAX		3
#define RESPONSE_ROOM_ENTER_ETC		4

//------------------------------------------------------------
// 9. Request 채팅 송신
//
// 2 Bytes : 메시지 Size
// Size  : 대화 내용 (유니코드)
//------------------------------------------------------------
#define REQUEST_CHAT				9

//------------------------------------------------------------
// 10. Response 채팅 수신 (수시) (나에겐 오지 않음)
//
// 4 Bytes : 송신자 번호
//
// 2 Bytes : 메세지 Size
// Size  : 대화 내용 (유니코드)
//------------------------------------------------------------
#define RESPONSE_CHAT				10

//------------------------------------------------------------
// 11. Request 방 퇴장
//
// None
//------------------------------------------------------------
#define REQUEST_ROOM_LEAVE		11

//------------------------------------------------------------
// 12. Response 방 퇴장 (수시)
//
// 4 Bytes : 사용자 번호
//------------------------------------------------------------
#define RESPONSE_ROOM_LEAVE		12

//------------------------------------------------------------
// 13. Response 방 삭제 (수시)
//
// 4 Bytes : 방 번호
//------------------------------------------------------------
#define RESPONSE_ROOM_DELETE		13

//------------------------------------------------------------
// 14. Response 타 사용자 입장 (수시)
//
// WCHAR[15] : 이름 (유니코드)
// 4 Bytes : 사용자 번호
//------------------------------------------------------------
#define RESPONSE_USER_ENTER		14

//------------------------------------------------------------
// 15. Request 회원 가입
//
// WCHAR[15] : 아이디 (유니코드)
// WCHAR[15] : 비밀번호 (유니코드)
// WCHAR[15] : 이름 (유니코드)
// WCHAR[15] : 전화번호 (유니코드)
//------------------------------------------------------------
#define REQUEST_JOIN			15

//------------------------------------------------------------
// 16. Response 회원 가입
// 
// 1 Byte : 결과 (1 : OK / 2 : 중복 닉네임 / 3 : 사용자 초과 / 4 : 기타 오류)
// 4 Bytes	: 사용자 번호
//------------------------------------------------------------
#define RESPONSE_JOIN			16

#define RESPONSE_JOIN_OK		1
#define RESPONSE_JOIN_DNICK		2
#define RESPONSE_JOIN_MAX		3
#define RESPONSE_JOIN_ETC		4

//------------------------------------------------------------
// 17. Request 회원 정보 수정
//
// WCHAR[15] : 아이디 (유니코드)
// WCHAR[15] : 비밀번호 (유니코드)
// WCHAR[15] : 이름 (유니코드)
// WCHAR[15] : 전화번호 (유니코드)
//------------------------------------------------------------
#define REQUEST_EDIT_INFO		17

//------------------------------------------------------------
// 18. Response 회원 정보 수정
// 
// 1 Byte : 결과 (1 : OK / 2 : 중복 닉네임 / 3 : 기타 오류)
// WCHAR[15] : 아이디 (유니코드)
// WCHAR[15] : 이름 (유니코드)
//------------------------------------------------------------
#define RESPONSE_EDIT_INFO		 18

#define RESPONSE_EDIT_INFO_OK	 1
#define RESPONSE_EDIT_INFO_DNICK 2
#define RESPONSE_EDIT_INFO_ETC	 3