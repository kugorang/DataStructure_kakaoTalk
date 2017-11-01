#pragma once

#include "resource.h"
#include "RingBuffer.h"
#include "Protocol.h"

#define WM_SOCKET (WM_USER + 1)

// 소켓 정보 저장을 위한 구조체
struct SOCKETINFO
{
	SOCKET linkSock;

	int userNo;
	int enterRoomNo;
	WCHAR id[ID_MAX_LEN];
	bool alreadyRoom;

	RingBuffer recvQueue;
	RingBuffer sendQueue;
};

extern int cmdShow, totalRoomNum;
extern bool cancelFlag;

extern HINSTANCE instance;

extern HWND hWndChoice, hWndJoin, hWndLogin, hWndLobby, hWndRoom, hWndNoti, hWndEditInfo;
extern HWND lobbyRoomList, roomChatList, roomPeopleList;

extern SOCKETINFO socketInfo;

extern WCHAR inputID[ID_MAX_LEN];
extern WCHAR inputPW[PW_MAX_LEN];
extern WCHAR inputName[NAME_MAX_LEN];
extern WCHAR inputPhoneNum[PHONENUM_MAX_LEN];

// IP 입력 다이얼로그 프로시저
INT_PTR CALLBACK IPDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 선택 다이얼로그 프로시저
INT_PTR CALLBACK ChoiceDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 회원가입 다이얼로그 프로시저
INT_PTR CALLBACK JoinDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 로그인 다이얼로그 프로시저
INT_PTR CALLBACK LoginDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 로비 다이얼로그 프로시저
INT_PTR CALLBACK LobbyDialogProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

// 방 다이얼로그 프로시저
INT_PTR CALLBACK RoomDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 알림 다이얼로그 프로시저
INT_PTR CALLBACK NotiDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 정보 수정 다이얼로그 프로시저
INT_PTR CALLBACK EditDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);