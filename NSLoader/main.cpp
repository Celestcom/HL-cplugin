#include "stdafx.h"
#include <iostream>
#include "NSLoader.h"
#include <thread>
#include <fstream>
#include <vector>
int main() {
	using namespace std;

	auto ptr = NSVR_Create();
	std::ifstream input("test_all.haptic", std::ios::binary);
	if (!input.is_open()) {
		throw std::runtime_error("Couldn't find the file");
	}
	// copies all data into buffer
	std::vector<char> buffer((
		std::istreambuf_iterator<char>(input)),
		(std::istreambuf_iterator<char>()));

	auto handle = NSVR_GenHandle(ptr);
	NSVR_CreateHaptic(ptr, handle, (void*)buffer.data(), buffer.size());
	NSVR_HandleCommand(ptr, handle, 0);
	//while (true) {
	//	int status = NSVR_PollStatus(ptr);
	//	std::cout << status << '\n';
	//	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	//}
	std::cin.get();
	NSVR_Delete(ptr);
	return 0;
}