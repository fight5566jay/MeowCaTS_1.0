#include "SGF.h"
#include "Tools.h"
#include "Ini.h"

using std::cerr;
using std::endl;
using std::flush;

SGF::SGF()
{
	m_sFileName = getCurrentPath() + "MJ_log\\" + getTime() + ".sgf";
	Ini &oIni = Ini::getInstance();
	m_bAvailable = oIni.getIntIni("Sgf.LogGameSgf") > 0 || oIni.getIntIni("Sgf.LogTreeSgf") > 0;
}

SGF::SGF(const string& sFileName, const SgfLogType& oSgfLogType)
{
	Ini &oIni = Ini::getInstance();
	m_bAvailable = oIni.getIntIni("Sgf.LogGameSgf") > 0 || oIni.getIntIni("Sgf.LogTreeSgf") > 0;
	string sDirName = getCurrentPath();
	if (oSgfLogType == Type_GameSgf && oIni.getIntIni("Sgf.LogGameSgf") > 0) {
		sDirName += oIni.getStringIni("Log.LogPath");
	}
	else if (oSgfLogType == Type_TreeSgf && oIni.getIntIni("Sgf.LogTreeSgf") > 0) {
		sDirName += oIni.getStringIni("Sgf.LogTreeSgfPath");
	}


	if (!isDirExists(sDirName)) {
		createFolder(sDirName);
	}
	m_sFileName = sDirName + "/" + sFileName + ".sgf";
	
}

void SGF::CreateNewSGF(){
	if (!m_bAvailable)
		return;
	m_fout.open(m_sFileName, std::ios::out);
	if (!m_fout.is_open()) {
		std::cerr << "SGF file open failed!!" << std::endl;
		assert(m_fout.is_open());
	}
}

void SGF::addTag(const string& sTag, const string& sValue )
{
	if (!m_bAvailable)
		return;
	m_fout << sTag << "[" << sValue <<"]" << flush;
}
void SGF::addAppendValue(const string& sValue )
{
	if (!m_bAvailable)
		return;
	m_fout <<  "[" << sValue <<"]" << flush;
}
void SGF::addTag(const string& sTag )
{
	m_fout <<  sTag << flush;
}

void SGF::addRootToSgf()
{
	if (!m_bAvailable)
		return;
	m_fout << "(;GM[84]" << flush;
}

void SGF::addMoveToSgf(const int& iWind, const string& sMove )
{
	if (!m_bAvailable)
		return;
	m_fout << ";" << getWindString(iWind) << "[" << sMove + "]" << flush;
}


void SGF::addBranchToSgf()
{
	if (!m_bAvailable)
		return;
	m_fout << ";(" << flush;
}

void SGF::EndBranchToSgf()
{
	if (!m_bAvailable)
		return;
	m_fout << ")" << flush;
}

void SGF::finish(){
	if (!m_bAvailable)
		return;
	m_fout << ")"<< flush;
	m_fout.close();
}

string SGF::getWindString(const int& iWind){
    switch(iWind){
        case 1:
            return "E";
        case 2:
            return "S";
        case 3:
            return "W";
        case 4:
            return "N";
    }
    return "";
}

