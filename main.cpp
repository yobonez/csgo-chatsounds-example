#include <iostream>
#include <vector>
#include <regex>
#include <filesystem>
#include <windows.h>

#include "proc.h"

#include "Chatsounds.h"


namespace fs = std::filesystem;

std::pair<HANDLE, uintptr_t> getChatAddr()
{
	std::pair<HANDLE, uintptr_t> hProcess_and_ChatAddr;

	DWORD procId = GetProcId(L"csgo.exe");
	uintptr_t moduleBase = GetModuleBaseAddress(procId, L"panorama.dll");
	HANDLE hProcess = 0;

	hProcess = OpenProcess(PROCESS_VM_READ, NULL, procId);

	uintptr_t dynamicPtrBaseAddr = moduleBase + 0x0023B09C;

	std::vector<unsigned int> chatOffsets = { 0x14C, 0x74, 0xBC, 0x24, 0x2A8, 0x124, 0x0 };
	uintptr_t chatAddr = FindDMAAddy(hProcess, dynamicPtrBaseAddr, chatOffsets);

	hProcess_and_ChatAddr = std::make_pair(hProcess, chatAddr);

	return hProcess_and_ChatAddr;
}


int main()
{
	Chatsounds chatsounds;

	std::pair<HANDLE, uintptr_t> hProcess_and_ChatAddr = getChatAddr();

	std::regex message_pattern("<[^>]*>");
	std::regex message_content_pattern("^[^:]+:\s*");

	char message[512];
	std::string prevMessage;

	while (true)
	{
		ReadProcessMemory(hProcess_and_ChatAddr.first,			// handle
			(BYTE*)hProcess_and_ChatAddr.second,  // chat memory address
			&message,
			sizeof(message),
			nullptr);

		std::string msg = (std::string)message;

		if (msg != prevMessage)
		{
			std::smatch match;
			std::string normal_chat_message = std::regex_replace(msg, message_pattern, "");
			std::string content;

			if (std::regex_search(normal_chat_message, match, message_content_pattern))
			{
				content = std::regex_replace(normal_chat_message, message_content_pattern, "");
				content.erase(0, 1);

				std::cout << "Input: " << content << std::endl;

				chatsounds.Parse(content);
			}
			else
			{
				Sleep(100);
				continue;
			};
		}

		prevMessage = msg;
		Sleep(100);
	}

	return 0;
}
