// OBS-Instance-Manager.cpp : Defines the entry point for the console application.
//

// Last tested with OBS 23.0.2
#include "stdafx.h"
#include <SDKDDKVer.h>
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <iostream>
#include <string>
#include <cstdio>
#include <vector>
#include <filesystem>
namespace fs = std::filesystem;
using std::string;
using std::vector;

//  Forward declarations:
DWORD FindProcessId(const std::wstring& processName);
const char * WinGetEnv(const char * name);

int main()
{
	if (FindProcessId(L"obs64.exe") || FindProcessId(L"obs.exe"))
	{
		std::cout << "OBS is already running!" << std::endl;
		std::cout << "Continuing with the process may cause issues." << std::endl;
	}
	std::cout << "------------" << std::endl;
	std::cout << "Commands: " << std::endl;
	std::cout << "list -- Lists existing obs-studio instances." << std::endl;
	std::cout << "add <name> -- Copies obs-studio settings to a new instance." << std::endl;
	std::cout << "remove <name> -- Removes instance settings." << std::endl;
	std::cout << "switch <name> -- Switches to given obs-studio instance." << std::endl;


	// Files that differ between instances:
	// ./basic/*
	// ./global.ini

	string command;
	std::cout << "Enter a command: ";
	std::cin >> command;

	if (command.find("list") != string::npos) {
		vector<std::string> obsfolders = vector<std::string>();
		std::string path = WinGetEnv("APPDATA"); // Get AppData path string
		for (const auto & entry : fs::directory_iterator(path)) {// Foreach file in directory
			std::string fld = entry.path().u8string();
			std::string foldername = fld.substr(fld.find_last_of("\\")+1, fld.length());
			if (foldername.find("obs-studio") != std::string::npos) {

				//obsfolders << foldername;
				// Insert string to obsfolders

				// WHen list called, iterate through list of strings
				// Move this whole section otu of list, below should be left here


				//std::cout << fld << std::endl;
				if (foldername.length() > 10){
					foldername = foldername.substr(11, foldername.length());
					std::cout << foldername << std::endl; // Other OBS Studio profiles
				}
				else {
					std::cout << /* Name from file here << */ "\t[Currently Active OBS-Studio]" << std::endl; // This one does not have text after it, so it's active.
				}
			}
		}
	}

	system("pause");
    return 0;
}


DWORD FindProcessId(const std::wstring& processName)
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processesSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	Process32First(processesSnapshot, &processInfo);
	if (!processName.compare(processInfo.szExeFile))
	{
		CloseHandle(processesSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(processesSnapshot, &processInfo))
	{
		if (!processName.compare(processInfo.szExeFile))
		{
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}

	CloseHandle(processesSnapshot);
	return 0;
}

const char * WinGetEnv(const char * name)
{
	const DWORD buffSize = 65535;
	static char buffer[buffSize];
	if (GetEnvironmentVariableA(name, buffer, buffSize))
	{
		return buffer;
	}
	else
	{
		return 0;
	}
}