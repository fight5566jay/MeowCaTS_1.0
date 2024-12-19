#pragma once
#include "Tools.h"
#include <fstream>
#include <string>
#define DEBUG_LOG_BUF_SIZE 65536 //2^16
#define MAX_DEBUG_LOG_SIZE 268435456 //2^28
using std::ofstream;
using std::string;

class DebugLogger {
public:
	DebugLogger() {};
	~DebugLogger() {};

	static int start(const string& sLogName);
	static void stop();

	static size_t write(const string& sInput);
	static size_t writeLine(const string& sInput);
	static int flush();

private:
	static ofstream m_fout;
	static string m_sLogName;
	static bool m_bActive;
	static char m_vBuf[DEBUG_LOG_BUF_SIZE];
	static size_t m_iCurrentBufSize;
	static size_t m_iCurrentLogSize;
	static size_t m_iFileIndex;
};
