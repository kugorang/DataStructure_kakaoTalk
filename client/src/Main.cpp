#include "stdafx.h"
#include "Main.h"
#include "WSAAsyncSelect.h"

#pragma comment(lib, "ws2_32.lib")

// 전역 변수:
int cmdShow, totalRoomNum = 0;
bool cancelFlag = false;

HINSTANCE instance;

HWND hWndChoice, hWndJoin, hWndLogin, hWndLobby, hWndRoom, hWndNoti, hWndEditInfo;
HWND lobbyRoomList, roomChatList, roomPeopleList;

WCHAR inputIP[16] = { 0, };
WCHAR inputID[ID_MAX_LEN] = { 0, };
WCHAR inputPW[PW_MAX_LEN] = { 0, };
WCHAR inputName[NAME_MAX_LEN] = { 0, };
WCHAR inputPhoneNum[PHONENUM_MAX_LEN] = { 0, };

// 소켓 관련 전역 변수
SOCKETINFO socketInfo;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// 윈속 초기화
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		return -1;
	}

	// TODO: 여기에 코드를 입력합니다.
	MSG msg;

	cmdShow = nCmdShow;
	instance = hInstance;

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_INPUTIP), NULL, IPDialogProc);
	hWndChoice = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_CHOICE), NULL, ChoiceDialogProc);
	hWndJoin = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_JOIN), NULL, JoinDialogProc);
	hWndLogin = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_LOGIN), NULL, LoginDialogProc);
	hWndLobby = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_CHATLOBBY), NULL, LobbyDialogProc);
	hWndRoom = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_CHATROOM), NULL, RoomDialogProc);
	hWndNoti = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_NOTI), NULL, NotiDialogProc);
	hWndEditInfo = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_JOIN), NULL, EditDialogProc);

	if (!hWndChoice || !hWndJoin || !hWndLogin || !hWndLobby || !hWndRoom || !hWndNoti || !hWndEditInfo)
	{
		return FALSE;
	}

	ShowWindow(hWndChoice, cmdShow);
	UpdateWindow(hWndChoice);

	// 기본 메시지 루프입니다.
	BOOL bRet;

	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bRet == -1)
			break;

		bRet = FALSE;

		if (IsWindow(hWndChoice))
		{
			bRet |= IsDialogMessage(hWndChoice, &msg);
		}

		if (IsWindow(hWndJoin))
		{
			bRet |= IsDialogMessage(hWndJoin, &msg);
		}

		if (IsWindow(hWndLogin))
		{
			bRet |= IsDialogMessage(hWndLogin, &msg);
		}

		if (IsWindow(hWndLobby))
		{
			bRet |= IsDialogMessage(hWndLobby, &msg);
		}

		if (IsWindow(hWndRoom))
		{
			bRet |= IsDialogMessage(hWndRoom, &msg);
		}

		if (IsWindow(hWndNoti))
		{
			bRet |= IsDialogMessage(hWndNoti, &msg);
		}

		if (IsWindow(hWndEditInfo))
		{
			bRet |= IsDialogMessage(hWndEditInfo, &msg);
		}

		if (!bRet)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

// IP 입력 다이얼로그 프로시저
INT_PTR CALLBACK IPDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND editBoxIP;

	switch (msg)
	{
	case WM_INITDIALOG:
		editBoxIP = GetDlgItem(hWnd, IDC_INPUTIP);
		SetWindowText(editBoxIP, L"127.0.0.1");

		return TRUE;
	case WM_COMMAND:
		switch (wParam)
		{
		case IDOK:
			GetDlgItemText(hWnd, IDC_INPUTIP, inputIP, sizeof(inputIP));
			EndDialog(hWnd, 0);

			return TRUE;
		case IDCANCEL:
			EndDialog(hWnd, 0);
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		PostQuitMessage(0);
		break;
	}

	return FALSE;
}

// 선택 다이얼로그 프로시저
INT_PTR CALLBACK ChoiceDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SOCKET *linkSock = &(socketInfo.linkSock);

	switch (msg)
	{
	case WM_INITDIALOG:
		// socket()
		*linkSock = socket(AF_INET, SOCK_STREAM, 0);

		// WSAAsyncSelect()
		if (SOCKET_ERROR == WSAAsyncSelect(*linkSock, hWnd, WM_SOCKET, FD_CONNECT | FD_CLOSE))
		{
			ErrorQuit(L"WSAAsyncSelect()");
			exit(-1);
		}

		SOCKADDR_IN serverAddr;
		serverAddr.sin_family = AF_INET;
		InetPton(AF_INET, inputIP, &serverAddr.sin_addr.s_addr);
		serverAddr.sin_port = htons(5000);

		if (SOCKET_ERROR == connect(*linkSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)))
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				ErrorQuit(L"connect()");
				exit(-1);
			}
		}

		return TRUE;
	case WM_SOCKET:
		ProcessSocketMessage(hWnd, msg, wParam, lParam);
		break;
	case WM_COMMAND:
		switch (wParam)
		{
		case IDC_JOIN:
			ShowWindow(hWndJoin, cmdShow);
			UpdateWindow(hWndJoin);

			return TRUE;
		case IDC_LOGIN:
			ShowWindow(hWndLogin, cmdShow);
			UpdateWindow(hWndLogin);

			return TRUE;
		}
		return FALSE;
	case WM_CLOSE:
		// 해당 모달리스 다이알로그가 메인인 경우 < 프로그램 종료를 위함
		PostQuitMessage(0);
		break;
	}

	return FALSE;
}

// 회원가입 다이얼로그 프로시저
INT_PTR CALLBACK JoinDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		case IDOK:
			GetDlgItemText(hWnd, IDC_INPUTID, inputID, sizeof(inputID));
			GetDlgItemText(hWnd, IDC_INPUTPW, inputPW, sizeof(inputPW));
			GetDlgItemText(hWnd, IDC_INPUTNAME, inputName, sizeof(inputName));
			GetDlgItemText(hWnd, IDC_INPUTNUM, inputPhoneNum, sizeof(inputPhoneNum));

			SendRequestJoin();

			EndDialog(hWnd, TRUE);
			return TRUE;
		case IDCANCEL:
			EndDialog(hWnd, TRUE);
			break;
		}
		break;
	case WM_SOCKET:
		ProcessSocketMessage(hWnd, msg, wParam, lParam);
		break;
	case WM_CLOSE:
		EndDialog(hWnd, TRUE);
		break;
	}

	return FALSE;
}

// 로그인 다이얼로그 프로시저
INT_PTR CALLBACK LoginDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		case IDOK:
			GetDlgItemText(hWnd, IDC_INPUTID, inputID, sizeof(inputID));
			GetDlgItemText(hWnd, IDC_INPUTPW, inputPW, sizeof(inputPW));

			SendRequestLogin(inputID, inputPW);

			EndDialog(hWnd, TRUE);

			ShowWindow(hWndLobby, cmdShow);
			UpdateWindow(hWndLobby);

			return TRUE;
		case IDCANCEL:
			EndDialog(hWnd, TRUE);

			break;
		}
		break;
	case WM_SOCKET:
		ProcessSocketMessage(hWnd, msg, wParam, lParam);
		break;
	case WM_CLOSE:
		EndDialog(hWnd, TRUE);
		break;
	}

	return FALSE;
}

// 로비 다이얼로그 프로시저
INT_PTR CALLBACK LobbyDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int index;

	switch (msg)
	{
	case WM_INITDIALOG:
		lobbyRoomList = GetDlgItem(hWnd, IDC_ROOMLIST);
		return TRUE;
	case WM_SOCKET:
		ProcessSocketMessage(hWnd, msg, wParam, lParam);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_ROOMLIST:
			switch (HIWORD(wParam))
			{
			case LBN_DBLCLK:
				// 리스트 박스에 항목 더블 클릭
				index = (int)SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0);
				SendRequestRoomEnter(index);
				break;
			}
			return TRUE;
		case IDC_CREATEBUTTON:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				SendRequestRoomCreate();
				break;
			}
			return TRUE;
		case IDC_EDITINFO:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				ShowWindow(hWndEditInfo, cmdShow);
				UpdateWindow(hWndEditInfo);
				break;
			}
			return TRUE;
		}
		return FALSE;
	case WM_CLOSE:
		EndDialog(hWnd, TRUE);
		PostQuitMessage(0);
		return TRUE;
	}
	return FALSE;
}

// 방 다이얼로그 프로시저
INT_PTR CALLBACK RoomDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		roomChatList = GetDlgItem(hWnd, IDC_CHATLIST);
		roomPeopleList = GetDlgItem(hWnd, IDC_PEOPLELIST);

		return TRUE;
	case WM_SOCKET:
		ProcessSocketMessage(hWnd, msg, wParam, lParam);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_SENDBUTTON:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				SendRequestChat();
				break;
			}
		}
		return FALSE;
	case WM_CLOSE:
		SendRequestRoomLeave();
		EndDialog(hWnd, TRUE);	// 해당 모달리스가 서브 다이알로그인 경우 < 다이알로그만 종료 함
		return TRUE;
	}
	return FALSE;
}

// 알림 다이얼로그 프로시저
INT_PTR CALLBACK NotiDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		case ID_NOTI_OK:
			if (!cancelFlag)
			{
				EndDialog(hWnd, TRUE);
			}
			else
			{
				PostQuitMessage(0);
			}
			
			return TRUE;
		}
		break;
	case WM_SOCKET:
		ProcessSocketMessage(hWnd, msg, wParam, lParam);
		break;
	case WM_CLOSE:
		EndDialog(hWnd, TRUE);
		break;
	}

	return FALSE;
}

// 정보 수정 다이얼로그 프로시저
INT_PTR CALLBACK EditDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		case IDOK:
			GetDlgItemText(hWnd, IDC_INPUTID, inputID, sizeof(inputID));
			GetDlgItemText(hWnd, IDC_INPUTPW, inputPW, sizeof(inputPW));
			GetDlgItemText(hWnd, IDC_INPUTNAME, inputName, sizeof(inputName));
			GetDlgItemText(hWnd, IDC_INPUTNUM, inputPhoneNum, sizeof(inputPhoneNum));

			SendRequestEditInfo();

			EndDialog(hWnd, TRUE);
			return TRUE;
		case IDCANCEL:
			EndDialog(hWnd, TRUE);
			break;
		}
		break;
	case WM_SOCKET:
		ProcessSocketMessage(hWnd, msg, wParam, lParam);
		break;
	case WM_CLOSE:
		EndDialog(hWnd, TRUE);
		break;
	}

	return FALSE;
}