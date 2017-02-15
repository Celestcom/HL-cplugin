#include "stdafx.h"
#include <iostream>
#include "NSLoader.h"
#include <thread>
int main() {

	auto ptr = NSVR_Create();
	while (true) {
		int status = NSVR_PollStatus(ptr);
		std::cout << status << '\n';
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	std::cin.get();
	NSVR_Delete(ptr);
	return 0;
}