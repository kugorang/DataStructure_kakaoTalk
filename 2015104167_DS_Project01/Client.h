#pragma once

#include "TemplateLinkedList.h"
#include "User.h"

class Client
{
private:
	User *loginedUser;							//< 현재 클라이언트에 로그인된 회원 정보	
	TemplateLinkedList<User*> friendList;		//< 로그인된 회원의 친구 목록
	TemplateLinkedList<User*> requestedList;	//< 로그인된 회원의 친구 요청 목록

public:
	/**
	* @fn		RequestJoin
	* @brief	서버에 회원 가입을 요청
	* @pre		회원 가입 하려는 정보가 이미 존재하지 않아야 함
	* @post		입력한 정보를 서버에 전송
	*/
	void RequestJoin();

	/**
	* @fn		ReceiveJoin
	* @brief	회원 가입 요청 응답을 서버로부터 수신
	* @pre		RequestJoin을 통해 회원 가입 요청을 보냈어야 함
	* @post		회원 가입 성공, 실패 여부를 서버로부터 수신
	*/
	void ReceiveJoin();

	/**
	* @fn		RequestLeave
	* @brief	서버에 회원 탈퇴를 요청
	* @pre		회원 탈퇴 하려는 정보가 서버에 존재해야 함
	* @post		입력한 정보를 서버에 전송
	*/
	void RequestLeave();

	/**
	* @fn		ReceiveLeave
	* @brief	회원 탈퇴 요청 응답을 서버로부터 수신
	* @pre		RequestLeave을 통해 회원 탈퇴 요청을 보냈어야 함
	* @post		회원 탈퇴 성공, 실패 여부를 서버로부터 수신
	*/
	void ReceiveLeave();

	/**
	* @fn		RequestLogin
	* @brief	서버에 로그인를 요청
	* @pre		로그인 하려는 정보가 서버에 존재해야 함
	* @post		입력한 정보를 서버에 전송
	*/
	void RequestLogin();

	/**
	* @fn		ReceiveLogin
	* @brief	로그인 요청 응답을 서버로부터 수신
	* @pre		RequestLogin을 통해 로그인 요청을 보냈어야 함
	* @post		로그인 성공, 실패 여부를 서버로부터 수신
	*/
	void ReceiveLogin();

	/**
	* @fn		RequestLogout
	* @brief	서버에 로그아웃을 요청
	* @pre		로그아웃 하려는 정보가 서버에 존재해야 함
	* @post		입력한 정보를 서버에 전송
	*/
	void RequestLogout();

	/**
	* @fn		ReceiveLogout
	* @brief	로그아웃 요청 응답을 서버로부터 수신
	* @pre		RequestLogout을 통해 로그아웃 요청을 보냈어야 함
	* @post		로그아웃 성공, 실패 여부를 서버로부터 수신
	*/
	void ReceiveLogout();

	/**
	* @fn		RequestFindUserByID
	* @brief	서버에 아이디로 회원 찾기를 요청
	* @pre		찾으려는 정보가 서버에 존재해야 함
	* @post		입력한 정보를 서버에 전송
	*/
	void RequestFindUserByID();

	/**
	* @fn		ReceiveFindUserByID
	* @brief	회원 찾기 요청 응답을 서버로부터 수신
	* @pre		RequestFindUserByID를 통해 회원 찾기 요청을 보냈어야 함
	* @post		회원 찾기 성공, 실패 여부를 서버로부터 수신
	*/
	void ReceiveFindUserByID();

	/**
	* @fn		RequestFindUserByName
	* @brief	서버에 이름으로 회원 찾기를 요청
	* @pre		찾으려는 정보가 서버에 존재해야 함
	* @post		입력한 정보를 서버에 전송
	*/
	void RequestFindUserByName();

	/**
	* @fn		ReceiveFindUserByName
	* @brief	회원 찾기 요청 응답을 서버로부터 수신
	* @pre		RequestFindUserByName를 통해 회원 찾기 요청을 보냈어야 함
	* @post		회원 찾기 성공, 실패 여부를 서버로부터 수신
	*/
	void ReceiveFindUserByName();

	/**
	* @fn		RequestUpdateInfo
	* @brief	서버에 회원 정보 수정을 요청
	* @pre		수정하려는 회원 정보가 서버에 존재해야 함
	* @post		입력한 정보를 서버에 전송
	*/
	void RequestUpdateInfo();

	/**
	* @fn		ReceiveUpdateInfo
	* @brief	회원 정보 수정 요청 응답을 서버로부터 수신
	* @pre		RequestUpdateInfo를 통해 회원 정보 수정 요청을 보냈어야 함
	* @post		회원 정보 수정 성공, 실패 여부를 서버로부터 수신
	*/
	void ReceiveUpdateInfo();

	void RequestAddFriend();
	void ReceiveAddFriend();

	void RequestDeleteFriend();
	void ReceiveDeleteFriend();

	void RequestSendMessage();
	void ReceiveSendMessage();

	void RequestDeleteMessage();
	void ReceiveDeleteMessage();
};