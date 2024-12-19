#include "DebugLogger.h"
#include "Ini.h"
using std::cerr;
using std::endl;
ofstream DebugLogger::m_fout;
string DebugLogger::m_sLogName;
bool DebugLogger::m_bActive = true;
char DebugLogger::m_vBuf[DEBUG_LOG_BUF_SIZE] = { 0 };
size_t DebugLogger::m_iCurrentBufSize = 0;
size_t DebugLogger::m_iCurrentLogSize = 0;
size_t DebugLogger::m_iFileIndex = 1;

int DebugLogger::start(const string & sLogName)
{
	Ini& oIni = Ini::getInstance();
	m_bActive = oIni.getIntIni("Debug.LoggingDebugMsg") > 0;

	if (!m_bActive)
		return 0;

	string sDirPath = getCurrentPath() + sLogName;
	createFolder(sDirPath);
	m_fout.open(sDirPath + "\\" + sLogName + "_" + std::to_string(m_iFileIndex) + ".log");
	if (!m_fout.is_open()) {
		std::cerr << "[DebugLogger::start] Cannot create log file " << sDirPath + "\\" + sLogName + "_" + std::to_string(m_iFileIndex) + ".log" << std::endl;
		return -1;
	}

	m_sLogName = sLogName;
	return 0;
}

void DebugLogger::stop()
{
	if (!m_bActive)
		return;
	m_fout.close();
}

size_t DebugLogger::write(const string & sInput)
{
	if (!m_bActive)
		return 0;

	size_t iOldBufSize = m_iCurrentBufSize;
	//if buffer will be full, flush buffer to file first.
	if (m_iCurrentBufSize + sInput.length() >= DEBUG_LOG_BUF_SIZE) {
		flush();
	}

	//write to buffer
	for (int i = 0, index = m_iCurrentBufSize; i < sInput.length(); i++, index++) {
		m_vBuf[index] = sInput.at(i);
	}
	m_iCurrentBufSize += sInput.length();
	m_vBuf[m_iCurrentBufSize] = '\0';
	return m_iCurrentBufSize - iOldBufSize;
}

size_t DebugLogger::writeLine(const string & sInput)
{
	size_t size = write(sInput + '\n');
	flush();
	return size;
}

int DebugLogger::flush()
{
	//if current file size is too big, switch to next file.
	if (m_iCurrentLogSize + m_iCurrentBufSize >= MAX_DEBUG_LOG_SIZE) {
		m_fout.close();
		m_fout.open(getCurrentPath() + m_sLogName + "\\" + m_sLogName + "_" + std::to_string(++m_iFileIndex) + ".log");
		if (!m_fout.is_open()) {
			std::cerr << "[DebugLogger::start] Cannot create log file " << getCurrentPath() + m_sLogName + "\\" + m_sLogName + "_" + std::to_string(m_iFileIndex) + ".log" << std::endl;
			return -1;
		}
		m_iCurrentLogSize = 0;
	}

	//write buffer to file
	m_fout << m_vBuf;
	m_fout.flush();
	m_iCurrentLogSize += m_iCurrentBufSize;
	m_iCurrentBufSize = 0;
	return 0;
}
