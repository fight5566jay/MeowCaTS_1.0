//
//  Tools.cpp
//  Mahjong
//
//  Created by nanj0178 on 2014/11/6.
//  Copyright (c) 2014 CGI. All rights reserved.
//

#define _CRT_SECURE_NO_WARNINGS 1
//#define PRT_DEBUG_MSG
#define NOMINMAX
#define _HAS_STD_BYTE 0 // for c++17

#include "Tools.h"
#include "stdarg.h"
#include <ctime>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <cassert>
#include "Ini.h"
#ifdef WINDOWS
#include <Windows.h>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif
using std::stringstream;
using std::cout;
using std::cerr;
using std::endl;

bool isFileExist(const string& sPath)
{
	return fs::exists(sPath) && fs::is_regular_file(sPath);
	/*if (FILE * file = fopen(sPath.c_str(), "r")) {
		fclose(file);
		return true;
	}
	return false;*/
}

bool isDirExists(const string& dirName_in)
{
	return fs::exists(dirName_in) && fs::is_directory(dirName_in);
	/*DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!*/
}

void clearFile(const string& sFileName)
{
	FILE * file = fopen(sFileName.c_str(), "w");
	if (file == nullptr) {
		std::cerr << "[Tools::clearFile] fopen error. Do you have created the log dictionary?" << std::endl;
		std::cerr << "Check the Log.LogPath in config.ini" << std::endl;
		assert(0);
	}
	fclose(file);
}

bool createFolder(const string & sFolderName)
{
	return fs::create_directory(sFolderName);
	/*bool success = CreateDirectory(sFolderName.c_str(), NULL);
	//wchar_t* sFolderName_copy = new wchar_t[sFolderName.length() + 1];
	//mbstowcs(sFolderName_copy, sFolderName.c_str(), sFolderName.length() + 1);
	//bool success = CreateDirectory(sFolderName_copy, NULL);
	//delete[] sFolderName_copy;

	if (!success) {
		if (GetLastError() != ERROR_ALREADY_EXISTS) {
			std::cerr << "[createFolder] CreateDirectory " << sFolderName << " failed:" << strerror(errno) << std::endl;
		}
		return GetLastError() == ERROR_ALREADY_EXISTS;//true if folder already exists
	}*/
}

void printTileValue(const vector<int>& vValue)
{
	if (vValue.size() != 34)
		return;

	std::cout << "   1  2  3  4  5  6  7  8  9";
	std::cout << std::endl << "M";
	for (int i = 0; i < 9; ++i)
		std::cout << " " << vValue[i];
	std::cout << std::endl << "P";
	for (int i = 9; i < 18; ++i)
		std::cout << " " << vValue[i];
	std::cout << std::endl << "S";
	for (int i = 18; i < 27; ++i)
		std::cout << " " << vValue[i];
	std::cout << std::endl << "Z";
	for (int i = 27; i < 34; ++i)
		std::cout << " " << vValue[i];
	std::cout << std::endl;
}

void printTileValue(const vector<float>& vValue)
{
	if (vValue.size() != 34)
		return;

	std::cout << "      1     2     3     4     5     6     7     8     9";
	std::cout << std::endl << "M";
	for (int i = 0; i < 9; ++i)
		printf("  %.2f", vValue[i]);
	std::cout << std::endl << "P";
	for (int i = 9; i < 18; ++i)
		printf("  %.2f", vValue[i]);
	std::cout << std::endl << "S";
	for (int i = 18; i < 27; ++i)
		printf("  %.2f", vValue[i]);
	std::cout << std::endl << "Z";
	for (int i = 27; i < 34; ++i)
		printf("  %.2f", vValue[i]);
	std::cout << std::endl;
}

void CERR(const string & str)
{
	static bool bPrintErrMsg = Ini::getInstance().getIntIni("Debug.PrintDebugMsg") > 0;
	if (!bPrintErrMsg) {
		return;
	}
	std::cerr << str;
}

/*int getNextPlayer(const int& iPlayer, const int& iNextStep)
{
	if (iNextStep == 1)
		return (iPlayer + 1) % 4;
	else if (iNextStep > 1)
		return getNextPlayer(getNextPlayer(iPlayer), iNextStep - 1);
	return -1;
}*/

int windToInt(const string& sWind)
{
	if (sWind == "E")
		return 1;
	if (sWind == "S")
		return 2;
	if (sWind == "W")
		return 3;
	if (sWind == "N")
		return 4;
	return 0;
}

string getTime()
{
#ifdef WINDOWS
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	string sTime = asctime(timeinfo);
	while (sTime.find(":") != string::npos)
		sTime.replace(sTime.find(":"), 1, "-");
	while (sTime.find("\n") != string::npos)
		sTime.replace(sTime.find("\n"), 1, "-");
	while (sTime.find("\r") != string::npos)
		sTime.replace(sTime.find("\r"), 1, "-");

	sTime += toStr((int)clock());
	return sTime;
#else
	time_t result = std::time(nullptr);
	asctime(localtime(&result));
	return toStr((int)result);
#endif
}

string FormatString(string sFormat, ...)
{
	va_list args;
	char buffer[1024];
	va_start(args, sFormat);
	vsnprintf(buffer, sizeof(buffer), sFormat.c_str(), args);
	return string(buffer);
}

string getCurrentPath()
{
	string sPath = fs::current_path().string() + "/";
/*#ifdef WINDOWS
	string sPath = "./";
	//wchar_t csPath[MAX_PATH];
	//GetModuleFileName(NULL, csPath, MAX_PATH);

	//wstring ws(csPath);
	//sPath = string(ws.begin(), ws.end());
	//string::size_type pos = sPath.find_last_of("\\");
	//sPath = sPath.substr(0, pos + 1);
	char csPath[MAX_PATH];
	GetModuleFileName(NULL, csPath, MAX_PATH);

	sPath.assign(csPath);
	string::size_type pos = sPath.find_last_of("\\");
	sPath = sPath.substr(0, pos + 1);
#endif*/
	
	return sPath;
}

string combineWith(const vector<string>& vString, const string& sCombine)
{
	if (vString.size() == 0) { return ""; }

	string result = vString[0];
	for (int i = 1; i < vString.size(); ++i)
		result += sCombine + vString[i];

	return result;
}

vector<string> splitWith(string sLine, char cSplit)
{
	vector<string> vItem;
	stringstream ss(sLine);
	string sItem;
	while (getline(ss, sItem, cSplit)) {
		if (sItem.size() > 0)
			vItem.push_back(sItem);
	}
	return vItem;
}

void ErrMsg(const string & str)
{
#ifdef PRT_DEBUG_MSG
	std::cout << str;
#endif
}

void printErrMsgAndExit(const string & str)
{
	std::cerr << str << std::endl;
	exit(0);
}

vector<string> getFilesList(const string & sDirName, const string & sFilter, bool bAbleRecursiveDirectory)
{
	if (!fs::exists(sDirName)) {
		std::cerr << "[Tools::getFilesList] Error: " << sDirName << " is not exist." << std::endl;
		return vector<string>();
	}
	if (!fs::is_directory(sDirName)) {
		std::cerr << "[Tools::getFilesList] Error: " << sDirName << " is not a directory." << std::endl;
		return vector<string>();
	}

	vector<fs::path> vPathList;
	if (bAbleRecursiveDirectory) {
		copy(fs::recursive_directory_iterator(sDirName), fs::recursive_directory_iterator(), back_inserter(vPathList));
	}
	else {
		copy(fs::directory_iterator(sDirName), fs::directory_iterator(), back_inserter(vPathList));
	}

	vector<string> vPathStringList;
	stringstream ss;
	string sFilePathString;
	std::cerr << "[Tools::getFilesList] Collecting list..." << std::endl;
	for (fs::path sFilePath : vPathList) {
		//std::cerr << sFilePath << " [sFilePath]" << std::endl;
		ss << sFilePath << std::endl;
		getline(ss, sFilePathString);
		//std::cerr << sFilePathString << " [getFileList]" << std::endl;
		sFilePathString.erase(sFilePathString.end() - 1);
		sFilePathString.erase(sFilePathString.begin());
		if(sFilePathString.find(sFilter) != string::npos)
			vPathStringList.push_back(sFilePathString);
		ss.clear();
	}

	std::cerr << "[Tools::getFilesList] Found " << vPathStringList.size() << " files." << std::endl;
	return vPathStringList;
}

vector<string> split(const std::string & str, const char & delimiter)
{
	vector<string> result;
	stringstream ss(str);
	string tok;

	while (std::getline(ss, tok, delimiter)) {
		result.push_back(tok);
	}
	return result;
}
