#pragma once

#include "TemplateLinkedList.h"
#include "User.h"

class Room
{
private:
	TemplateLinkedList<User*> roomUserList;		///< 방에 속해 있는 회원 정보 목록
	string name;								///< 방 이름
	string profileImg;							///< 방 프로필 이미지

public:
	/**
	* @fn		Room
	* @brief	방 정보를 초기화 하는 생성자
	* @pre		없음
	* @post		매개변수로 받은 이름과 프로필 이미지로 방 정보가 설정됨
	*/
	Room(string name, string profileImg);

	int AddUser(User* newUser);

	int DeleteUser(User* leaveUser);

	int GetName();
	int GetProfileImg();

	int PrintRoomInfo();
};