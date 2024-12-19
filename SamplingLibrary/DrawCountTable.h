#pragma once
#include "../MJLibrary/MJ_Base/MahjongBaseConfig.h"
#include "../MJLibrary/MJ_Base/PlayerTile.h"
#include "../MJLibrary/MJ_Base/MJBaseTypedef.h"
#include <string>
#include <array>

using std::string;
using std::array;
class DrawCountTable {
public:
	DrawCountTable();
	~DrawCountTable() {};

	void init();
	void setupDrawCountTable();
	static int getDrawCount(const int& iMinLack, const int& iLeftDrawCount) { return g_vDrawCountTable.at(iMinLack).at(iLeftDrawCount); };
	void train(const int& iIterationCount);
	void loadWinCountTableFromFile(const string& sFileName);
	void writeWinCountTableToFile(const string& sFileName);
	void updateMostWinCountList();
	void updateDrawCountTable();
	void initRandomState(const int& iHandTileNumber);
	bool checkValidDrawCountTableVersion(ifstream& fin);
	void printDrawCountTable() const;
	string getRealFileName(const string& sFileName) const;

	static bool g_bFirstSetup;
	static array<int, 6> g_vInfo;
	static array<array<int, AVERAGE_MAX_DRAW_COUNT + 1>, MAX_MINLACK + 1> g_vDrawCountTable;
	//static int m_vDrawCountTable[MAX_MINLACK + 1][AVERAGE_MAX_DRAW_COUNT + 1];
	array<array<int, AVERAGE_MAX_DRAW_COUNT + 2>, MAX_MINLACK + 1> m_vWinCountTable;
	//array<int, MAX_MINLACK + 1> m_vTotalWinCount;
	array<int, MAX_MINLACK + 1> m_vMostPossibleMaxDrawCount;

private:
	/*
	int m_vWinCountTable[MAX_MINLACK + 1][AVERAGE_MAX_DRAW_COUNT + 1];
	int m_vWinRateDataCount[MAX_MINLACK + 1];
	int m_vMostPossibleMaxDrawCount[MAX_MINLACK + 1];*/
	
	PlayerTile m_oPlayerTile;
	RemainTile_t m_oRemainTile;

	
};