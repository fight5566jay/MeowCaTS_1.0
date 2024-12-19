#pragma once
#include "../MJLibrary/MJ_Base/MJBaseTypedef.h"
#include "../MJLibrary/MJ_Base/PlayerTile.h"
#include "../MJLibrary/Base/ReserviorOneSampler.h"

class TestMode {
public:
	TestMode() {};
	~TestMode() {};

	void run(int argc, char* argv[]);
	void testMctsPlayer(int argc, char* argv[]);
	void testMcts2(int argc, char* argv[]);
	void testNewMinLackTable();
	void testSamplingData(int argc, char * argv[]);
	void testUsefulTileGreedySamplingData(int argc, char * argv[]);
	void testReserviorOneSampler(int argc, char * argv[]);

	//exploration term related testing function
	void testExplorationTermTable(int argc, char* argv[]);


	void moTile(const Tile& oTile);
	void initState_random(const int& iHandTileNumber, const bool& bTooManyMelds = false);
	void initState_fromFile(ifstream& fin);
	void initState1();
	void initState2();
	void initState_worstCase();
	void initState_greatCase();
	void printState() const;
	void genTestCase();

	vector<Move> getMeldMoveList(const Tile & oTargetTile, const bool& bCanEat, const bool& bCanKong) const;

	//others
	//int testArray();

private:
	PlayerTile m_oPlayerTile;
	RemainTile_t m_oRemainTile;
	ofstream fout1;
};