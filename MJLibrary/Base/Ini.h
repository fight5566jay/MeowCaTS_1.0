#pragma once
#include <string>
#define USE_EXTERN_INI
#ifdef USE_EXTERN_INI
#include "xini_file.h"
#else
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#endif
using std::string;

class Ini{
public:
	Ini(const std::string& sFileName = "config.ini");
	~Ini() {};

    int getIntIni(string sName);
    string getStringIni(string sName);
    double getDoubleIni(string sName);
    static Ini& getInstance();

protected:
    static Ini *g_pIni;
    
#ifdef USE_EXTERN_INI
	xini_file_t m_oFileReader;
#else
	boost::property_tree::ptree pt;
#endif
};
