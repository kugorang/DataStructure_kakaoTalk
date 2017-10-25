#pragma once

// 크크크

class App
{
private:
public:
	/**
	* @fn		App
	* @brief	애플리케이션이 실행된 후 초기화 작업을 진행
	* @detail	Server와 Client 클래스를 생성하여 카카오톡 프로그램을 확인할 환경을 만듦
	* @pre		없음
	* @post		Server 및 Client 클래스들이 생성되어 원활한 프로그램 수행 환경 조성
	*/
	App();

	/**
	* @fn		~App
	* @brief	애플리케이션이 종료하기 전 정리 작업을 진행
	* @pre		없음
	* @post		App에서 만든 Server와 Client 클래스를 삭제
	*/
	~App();

	/**
	* @fn		Run
	* @brief	애플리케이션을 실행
	* @pre		애플리케이션 생성자를 통해 초기화 작업이 완료
	* @post		애플리케이션이 실행되어 애플리케이션의 동작이 수행 가능
	*/
	void Run();
};