//
//  Tools.h
//  Mahjong
//
//  Created by nanj0178 on 2014/11/6.
//  Copyright (c) 2014 CGI. All rights reserved.
//

#ifndef __Mahjong__Tools__
#define __Mahjong__Tools__

#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <random>
#include <functional>
#include "Type.h"
#include <time.h>
#include <climits>

using std::stringstream;
using std::vector;
using std::string;

static uint64_t uiRandGenCount = 0;
thread_local static std::random_device rd;
//thread_local static default_random_engine gen = default_random_engine(time(0) + uiRandGenCount++);
//thread_local static default_random_engine gen = default_random_engine(0);
thread_local static std::default_random_engine gen = std::default_random_engine(rd());
thread_local static std::uniform_real_distribution<double> dis_double(0, 1);
thread_local static auto random_double = bind(dis_double, gen);
thread_local static std::uniform_int_distribution<int> dis_int(0, INT_MAX);
thread_local static auto random_int = bind(dis_int, gen);

/*void setRandomSeed(uint32_t uiSeed)
{
	gen = default_random_engine(uiSeed);
	random_double = bind(dis_double, gen);
}*/

template<typename T>
string toStr(const T& value)
{
	stringstream ss;
	ss << value;
	return ss.str();
};

template<>
inline string toStr(const bool& bValue) { return bValue ? "true" : "false"; };

template<typename T>
int toInt(const T& value)
{
	int result;
	stringstream ss;
	ss << value;
	ss >> result;
	return result;
}

template<typename T>
int64 toInt64(const T& value)
{
	int64 result;
	stringstream ss;
	ss << value;
	ss >> result;
	return result;
}

void CERR(const string& str);
string FormatString(string sFormat, ...);

template<class T>
inline type getType(const T& var) { return typeid(var).hash_code(); };
template<class T>
inline type getType() { return typeid(T).hash_code(); };
inline bool toBool(const int& value) { return value != 0; };
//inline bool toBool(const string& str) { string s = boost::to_upper_copy(str); return s == "T" || s == "TRUE"; };
inline bool isVaildTile(const int& n) { return n >= 1 && n <= 9; };
inline bool hitRate(const double& dRate) {
	double num = random_double();
	//CERR(FormatString("[hitRate] random number = %lf\n", num));
	return num < dRate;
};
inline bool rand(const int& iFalseWeight, const int& iTrueWeight) { return hitRate(double(iTrueWeight) / double(iFalseWeight + iTrueWeight)); };
//inline void trim(string& str) { boost::trim(str); };
//inline void strToUpper(string& str) { boost::to_upper(str); };
//inline void strToLower(string& str) { boost::to_lower(str); };
inline char charToUpper_copy(const char& c) { return c >= 'a' && c <= 'z' ? c - 'a' + 'A' : c; };
//inline string strToUpper_copy(const string& str) { return boost::to_upper_copy(str); };
//inline string strToLower_copy(const string& str) { return boost::to_lower_copy(str); };
inline float toFloat(string str) { return static_cast<float>(atof(str.c_str())); };

//int getNextPlayer(const int& iPlayer, const int& iNextStep = 1);
int windToInt(const string& sWind);
bool isFileExist(const string& sPath);
bool isDirExists(const string& dirName_in);
void clearFile(const string& sFileName);
bool createFolder(const string& sFolderName);
void printTileValue(const vector<int>& vValue);
void printTileValue(const vector<float>& vValue);
string getTime();
string getCurrentPath();
string combineWith(const vector<string>& vString, const string& sCombine = " ");
vector<string> splitWith(string sLine, char cSplit);
void ErrMsg(const string& str);
void printErrMsgAndExit(const string& str);
vector<string> getFilesList(const string& sDirName, const string& sFilter, bool bAbleRecursiveDirectory);
vector<string> split(const string &str, const char &delimiter);

#endif