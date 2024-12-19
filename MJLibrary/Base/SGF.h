#pragma once
#include <string>
#include <fstream>
using std::string;
enum SgfLogType { Type_TreeSgf, Type_GameSgf };

class SGF{
public:
	SGF();
	SGF(const string& sFileName, const SgfLogType& oSgfLogType);
    void CreateNewSGF();
    void addTag(const string& sTag, const string& sValue );
    void addRootToSgf();
    void addAppendValue(const string& sValue);
    void addTag(const string& sTag);
    void addMoveToSgf(const int& iWind, const string& sMove );
    void finish();
    void addBranchToSgf();
    void EndBranchToSgf();
	inline bool isAvailable() const { return m_bAvailable; };

	static string getWindString(const int& iWind);

public:
	string m_sFileName;
	std::ofstream m_fout;

private:
	bool m_bAvailable;
};