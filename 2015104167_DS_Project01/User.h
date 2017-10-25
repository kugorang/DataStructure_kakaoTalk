#pragma once

#include <string>

using namespace std;

class User
{
private:
	string name;			///< 회원 이름
	string phoneNumber;		///< 회원 전화 번호
	string kakaoID;			///< 회원 카카오ID
	string password;		///< 회원 비밀번호
	string profileImg;		///< 회원 프로필 이미지
	string bgImg;			///< 회원 배경 이미지

public:
	// testasdf
	User(string name, string phoneNumber, string kakaoID, string password, string profileImg, string bgImg);

	string GetName();
	string GetPhoneNumber();
	string GetKakaoID();
	string GetPassword();
	string GetProfileImg();
	string GetBgImg();

	void PrintUserInfo();
};