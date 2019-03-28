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
#include <algorithm>
#include <fstream>
#include <sstream>
namespace fs = std::experimental::filesystem;
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
	std::cout << "add <name> -- Copies [Current] obs-studio settings to a new instance." << std::endl;
	std::cout << "rename <name> -- Changes [Current] obs-studio name." << std::endl;
	std::cout << "remove <name> -- Removes instance settings (No undo!)." << std::endl;
	std::cout << "switch <name> -- Switches to given obs-studio instance." << std::endl;

	// --------------------
	// Add plugin_config to be moved as well
	// profiler_data
	// --------------------


	// Files that differ between instances:
	// ./basic/*
	// ./global.ini
	std::string path = WinGetEnv("APPDATA"); // Get AppData path string
	std::string curPath = path + "\\" + "obs-studio";

	while (true) {
		string command;
		std::cout << std::endl << "Enter a command: ";
		std::getline(std::cin, command);

		if (command.find("list") != string::npos) {
			vector<std::string> obsfolders = vector<std::string>();
			for (const auto& entry : fs::directory_iterator(path)) {// Foreach file in directory
				std::string fld = entry.path().u8string();
				std::string foldername = fld.substr(fld.find_last_of("\\") + 1, fld.length());
				if (foldername.find("obs-studio") != std::string::npos) {
					if (foldername.length() > 10) {
						foldername = foldername.substr(11, foldername.length());
						std::cout << "- " << foldername << std::endl; // Other OBS Studio profiles
					}
					else {
						std::string curName;
						std::ifstream fl(curPath + "\\name.obsinstance");
						std::stringstream cNbuffer;
						cNbuffer << fl.rdbuf();
						curName = cNbuffer.str();
						fl.close();

						if (curName.length() < 1)
						{
							std::cout << "----------------------------------------" << std::endl;
							std::cout << "--    Active profile has no name!     --" << std::endl;
							std::cout << "-- Use 'rename <name>' to give it one --" << std::endl;
							std::cout << "----------------------------------------" << std::endl;
						}
						else {
							std::cout << curName << "\t[Currently Active OBS-Studio]" << std::endl; // This one does not have text after it, so it's active.
						}
					}
				}
			}
		}
		else if (command.find("rename") != string::npos) {
			std::string obsNewName = command.erase(0, 7); // Remove "rename "
			std::replace(obsNewName.begin(), obsNewName.end(), ' ', '-'); // Replace spaces in name with hypens

			std::ofstream file(curPath + "\\name.obsinstance");
			file << obsNewName;
			file.close();

			std::cout << "Successfully changed name to '" << obsNewName << "'!" << std::endl;
		}
		else if (command.find("add") != string::npos) {
			std::string obsNewName = command.erase(0, 4); // Remove "add "
			std::replace(obsNewName.begin(), obsNewName.end(), ' ', '-'); // Replace spaces in name with hypens
			std::string newPath = path + "\\" + "obs-studio-" + obsNewName;



			if (!std::experimental::filesystem::exists(newPath)) {
				std::experimental::filesystem::create_directory(newPath);
				std::cout << "--> obs-studio-" << obsNewName << " created in %AppData% [1/3]" << std::endl;

				std::experimental::filesystem::copy_file(curPath + "\\global.ini", newPath + "\\global.ini", fs::copy_options::overwrite_existing | fs::copy_options::recursive);	// Copy ./global.ini
				std::experimental::filesystem::copy(curPath + "\\basic", newPath + "\\basic", fs::copy_options::overwrite_existing | fs::copy_options::recursive);					// Copy ./basic/*
				std::cout << "--> Current settings copied into new directory [2/3]" << std::endl;

				std::ofstream file(newPath + "\\name.obsinstance");
				file << obsNewName;
				file.close();
				std::cout << "--> Instance finalised [3/3]" << std::endl;
			}
			else {
				std::cout << "The requested OBS instance already exists (" << obsNewName << ")" << std::endl;
				std::cout << "You can use \"remove " << obsNewName << "\" to remove it." << std::endl;
			}
		}
		else if (command.find("remove") != string::npos) {
			std::string obsRemoveName = command.erase(0, 7); // Remove "switch "
			std::replace(obsRemoveName.begin(), obsRemoveName.end(), ' ', '-'); // Replace spaces in name with hypens
			std::string obsRemovePath = path + "\\" + "obs-studio-" + obsRemoveName;
			

			if (std::experimental::filesystem::exists(obsRemovePath)) {
				std::cout << "Are you sure you want to delete " << obsRemoveName << " permanently?" << std::endl;
				char response;
				do
				{
					std::cout << "Are you sure? [y/n]: ";
					std::cin >> response;
				} while (!std::cin.fail() && response != 'y' && response != 'n');
				if (response == 'y') {
					std::cout << "Removing " << obsRemoveName << "..." << std::endl;
					std::string execCommand = "rmdir /S/Q \"" + obsRemovePath + "\"";
					system(execCommand.c_str());
					std::cout << "Removed!" << std::endl;
				}
				else {
					std::cout << "Cancelled." << std::endl;
				}
			} else
				std::cout << obsRemoveName << " was not found!" << std::endl;

			
		}
		else if (command.find("switch") != string::npos) {
			std::string obsWantedName = command.erase(0, 7); // Remove "switch "
			std::replace(obsWantedName.begin(), obsWantedName.end(), ' ', '-'); // Replace spaces in name with hypens

			// Get current OBS name
			std::string curName;
			std::ifstream fl(curPath + "\\name.obsinstance");
			std::stringstream cNbuffer;
			cNbuffer << fl.rdbuf();
			curName = cNbuffer.str();
			fl.close();

			if (curName.length() < 1)
			{
				std::cout << "----------------------------------------" << std::endl;
				std::cout << "--    Active profile has no name!     --" << std::endl;
				std::cout << "-- Use 'rename <name>' to give it one --" << std::endl;
				std::cout << "----------------------------------------" << std::endl;
			}
			else {
				std::string wantedOBSPath = path + "\\" + "obs-studio-" + obsWantedName;
				std::string newOBSPath = path + "\\" + "obs-studio-" + curName;
				//CONTINUE ONLY IF WANTED OBS EXISTS!
				if (!std::experimental::filesystem::exists(obsWantedName)) {
					std::cout << "Switching from: " << curName << std::endl;
					std::cout << "To: " << obsWantedName << std::endl << std::endl;

					std::cout << "Unmounting current instance [1/2]" << std::endl;

					bool moveComplete = false;

					while (!moveComplete) {

						if (!std::experimental::filesystem::exists(newOBSPath)) {
							// MOVING OLD OBS FROM FOLDER
							std::experimental::filesystem::create_directory(newOBSPath);
							std::cout << "--> obs-studio-" << curName << " created in %AppData% [1/3]" << std::endl;

							std::string ACTIVEglobal = curPath + "\\global.ini";
							std::string ACTIVEinstance = curPath + "\\name.obsinstance";
							std::string ACTIVEbasic = curPath + "\\basic";

							std::experimental::filesystem::copy_file(ACTIVEglobal, newOBSPath + "\\global.ini", fs::copy_options::overwrite_existing);						// Copy ./global.ini
							std::experimental::filesystem::copy_file(ACTIVEinstance, newOBSPath + "\\name.obsinstance", fs::copy_options::overwrite_existing);				// Copy ./name.obsinstance
							std::experimental::filesystem::copy(ACTIVEbasic, newOBSPath + "\\basic", fs::copy_options::overwrite_existing | fs::copy_options::recursive);	// Copy ./basic/*
							std::cout << "--> Current settings copied into new directory [2/3]" << std::endl;

							std::experimental::filesystem::remove(ACTIVEglobal);					// Remove ./global.ini
							std::experimental::filesystem::remove(ACTIVEinstance);					// Remove ./name.obsinstance
							std::string basicRemoveCommand = "rmdir /S/Q \"" + ACTIVEbasic + "\"";
							system(basicRemoveCommand.c_str());										// Remove ./basic/* and ./basic itself
							std::cout << "--> Removed old settings from active OBS instance [3/3]" << std::endl;


							std::cout << "Mounting wanted instance [2/2]" << std::endl;


							// MOVING NEW OBS TO FOLDER
							std::experimental::filesystem::copy_file(wantedOBSPath + "\\global.ini", curPath + "\\global.ini", fs::copy_options::overwrite_existing);				// Copy ./global.ini
							std::experimental::filesystem::copy_file(wantedOBSPath + "\\name.obsinstance", curPath + "\\name.obsinstance", fs::copy_options::overwrite_existing);	// Copy ./name.obsinstance
							std::experimental::filesystem::copy(wantedOBSPath + "\\basic", curPath + "\\basic", fs::copy_options::overwrite_existing | fs::copy_options::recursive);	// Copy ./basic/*
							std::cout << "--> Settings copied from wanted OBS instance [1/2]" << std::endl;

							std::string execCommand = "rmdir /S/Q \"" + wantedOBSPath + "\"";
							system(execCommand.c_str());
							std::cout << "--> Wanted OBS instance folder removed [2/2]" << std::endl;


							std::cout << "Switching complete!" << std::endl;

							moveComplete = true;
						}
						else
						{
							std::string execCommand = "rmdir /S/Q \"" + newOBSPath + "\"";
							system(execCommand.c_str());
						}
					}
				}
				else
					std::cout << "Requested OBS instance could not be found! Use 'list' to see existing OBS instances" << std::endl;
			}
		}
	}
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