#pragma once
#include <string>
#include <fstream>
//#include <memory>

using std::string;
using std::ifstream;
using std::ofstream;
//using std::shared_ptr;

class MinLackEntry
{
public:
	MinLackEntry();
	~MinLackEntry();

public:
	inline bool hasCostPair() { return m_bCostPair; };
	inline bool hasSecondChoice() { return m_bSecondChoice; };

	void setSecondChoice(const MinLackEntry &entry);
	string toString();
	void inputEntry(ifstream& inputFd);
	void outputEntry(ofstream& outputFd);

public:
	int m_iTileNumber;
	int m_iMeld;
	int m_iPair;
	int m_iSequ;
	int m_iUsefulBits;
	// number of tile count more than 1
	int m_iLargerOne;
	int m_iAlone;
	int m_iSevenPairUsefulBits;
	int m_iGoshimusoPart;
	int m_bGoshimusoPair;
	int m_iGoshimusoUsefulBits;
	bool m_bCostPair;
	bool m_bSecondChoice;
	MinLackEntry* m_eSecond;
};