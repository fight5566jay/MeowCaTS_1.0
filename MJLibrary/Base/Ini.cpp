#include "Ini.h"
#include <iostream>
#include <cassert>
#include "Tools.h"
using std::cout;
using std::cerr;
using std::endl;

Ini* Ini::g_pIni = nullptr;

Ini::Ini(const std::string& sFileName)
{
    string sPath = getCurrentPath() + sFileName;
	if (!isFileExist(sPath)) {
		std::cerr << "Cannot find file: " << sPath << std::endl;
		std::cerr << "Please make sure you have the config file in the correct path." << std::endl;
		assert(0);
		//system("pause");
		exit(0);
	}

#ifdef USE_EXTERN_INI
	//std::cerr << "[Ini::Ini] Initial ini: " << sPath << std::endl;
	//oIniReader = inih::INIReader(sPath);
	if (!m_oFileReader.load(sPath)) {
		std::cerr << "[Ini::Ini] Failed to load ini: " << sPath << std::endl;
		assert(0);
	}
	//std::cerr << "[Ini::Ini] Find ini file: " << sPath << std::endl;
#else
	boost::property_tree::ini_parser::read_ini(sPath, pt);
#endif
}
Ini& Ini::getInstance()
{
	//[TODO] given config file name?
    if(g_pIni==NULL){
        g_pIni = new Ini();
    }
    return *g_pIni;
}

int Ini::getIntIni(string sName)
{
#ifdef USE_EXTERN_INI
	int pos = sName.find(".");
	assert(pos != string::npos);

	string sSection = sName.substr(0, pos);
	string sKey = sName.substr(pos + 1);
	//std::cerr << "[Ini::getIntIni] " << sSection << " " << sKey << std::endl;
	return (int)m_oFileReader[sSection][sKey];
#else
	return pt.get<int>(sName);
#endif
}

string Ini::getStringIni(string sName)
{
#ifdef USE_EXTERN_INI
	int pos = sName.find(".");
	assert(pos != string::npos);

	string sSection = sName.substr(0, pos);
	string sKey = sName.substr(pos + 1);
	//std::cerr << "[Ini::getStringIni] " << sSection << " " << sKey << std::endl;
	return (string)m_oFileReader[sSection][sKey];
#else
	return pt.get<string>(sName);
#endif
}

double Ini::getDoubleIni(string sName)
{
#ifdef USE_EXTERN_INI
	int pos = sName.find(".");
	assert(pos != string::npos);

	string sSection = sName.substr(0, pos);
	string sKey = sName.substr(pos + 1);
	//std::cerr << "[Ini::getDoubleIni] " << sSection << " " << sKey << std::endl;
	return (double)m_oFileReader[sSection][sKey];
#else
	return pt.get<double>(sName);
#endif
}
