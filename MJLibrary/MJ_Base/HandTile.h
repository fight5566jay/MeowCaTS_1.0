#pragma once
#include <string>
#include <array>
#include <vector>
#include <memory>
#include "MahjongBaseConfig.h"
#include "Tile.h"
//#include "MinLackTable/MinLackTable.h"
#include "MinLackTable/MinLackTable_old.h"
using std::string;
using std::array;
using std::vector;
typedef int HandTileId_t;
typedef array<int ,MAX_DIFFERENT_TILE_COUNT> HandTileNumberArray;
typedef array<HandTileId_t, TILE_TYPE_COUNT> HandTileIdArray;

class HandTile {
public:
	HandTile();
	HandTile(const string& sInput);
	~HandTile() {};

	//basic function
	void clear();
	inline int getTileNumber() const { return m_iTotalTileNumber; };
	int getTileNumber(const Tile& oTile) const;
	int getTileNumberList(HandTileNumberArray& vTileNumbers) const;
	void putTile(const Tile& oTile, const int& iNumber = 1);
	void popTile(const Tile& oTile, const int& iNumber = 1);
	inline void popAllTile(const Tile& oTile) { popTile(oTile, getTileNumber(oTile)); };
	string toString() const;
	HandTileIdArray getHandTileId() const;

	//appendix
	inline int getNeedMeldNumber() const { return getTileNumber() / 3; /* WRONG? */ };
	inline bool canPong(const Tile& oTile) const { return getTileNumber(oTile) >= 2; };
	inline bool canKong(const Tile& oTile) const { return getTileNumber(oTile) == 3; };
	inline bool canDarkKong(const Tile& oTile) const { return getTileNumber(oTile) == 4; };
	inline bool canEat(const Tile& oTile) const { return canEatLeft(oTile) || canEatMiddle(oTile) || canEatRight(oTile); };
	bool canEatRight(const Tile& oTile) const;
	bool canEatMiddle(const Tile& oTile) const;
	bool canEatLeft(const Tile& oTile) const;
	bool canDarkKong() const;
	vector<Tile> getCanDarkKongTile() const;
	bool isAloneTile(const Tile& oTile) const;
	bool willBeAloneTile(const Tile& oTile) const;
	bool isTooManyMeld(const int& iNeedMeldCount) const;

	int getId(HandTileIdArray& vIds) const;//return the size of id array
	inline int getMinLack() const { 	return MinLackTable_old::getMinLackNumber(m_vIds.data());	};
	inline int getMinLack(const int& iNeedGroup) const {	return MinLackTable_old::getMinLackNumber(m_vIds.data(), iNeedGroup, NEED_EYEs);	};
	inline bool isWin() const { return isWin(getNeedMeldNumber()); };
	inline bool isWin(const int& iNeedMeldNumber) const { 
		//return WinTable::isWin(iNeedMeldNumber, getId().get()); //BUG: not working correctly
		return getMinLack(iNeedMeldNumber) == 0;
	};
	inline bool canWin(const Tile& oTile) const { return canWin(oTile, getNeedMeldNumber()); };
	bool canWin(const Tile& oTile, const int& iMeldNumber) const;

	bool operator==(const HandTile& rhs) const;
	inline bool operator!=(const HandTile& rhs) const { return !(*this == rhs); };
	//void operator=(const HandTile& rhs);

	friend class HandTileTest_getTileTest_Test;

private:
	void _init();

private:
	//array<int , MAX_DIFFERENT_TILE_COUNT> m_vTileNumbers;
	HandTileIdArray m_vIds;
	int m_iTotalTileNumber;

	//static bool m_bFirstSetup;
	//static array<int, MAX_RANK + 1> g_vTileIds;
};