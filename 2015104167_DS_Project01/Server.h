#pragma once

#include "TemplateLinkedList.h"
#include "User.h"

class Server
{
private:
	// 회원 목록을 관리하기 위한 Template LinkedList
	TemplateLinkedList<User*> userList;

	// Sever의 아래 함수들은 외부에서 사용하면 안 되므로
	// 접근 지시자를 private로 설정함.

	bool Join();

	bool Leave();

	bool Login();

	bool Logout();

	bool FindUserByID();

	bool FindUserByName();

	bool UpdateInfo();

	bool AddFriend();

	bool DeleteFriend();

	bool SendMessage();

	bool DeleteMessage();

public:
	Server();

	~Server();
};