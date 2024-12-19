#pragma once
#include <string>
#include <array>
#include <vector>
#include "MahjongBaseConfig.h"
#include "Tile.h"
//#include "MinLackTable/MinLackTable.h"
#include "MinLackTable/MinLackTable_old.h"
#include "WinTable.h"
using std::string;
using std::array;
using std::vector;

class HandTile_old {
public:
	HandTile_old() { clear(); };
	HandTile_old(const string& sInput);
	~HandTile_old() {};

	//basic function
	void clear();
	inline int getTileNumber() const { return m_iTotalTileNumber; };
	inline int getTileNumber(const Tile& oTile) const { return oTile.isNull() ? 0 : m_vTileNumbers[oTile]; };
	void putTile(const Tile& oTile, const int& iNumber = 1);
	void popTile(const Tile& oTile, const int& iNumber = 1);
	inline void popAllTile(const Tile& oTile) { popTile(oTile, getTileNumber(oTile)); };
	string toString() const;
	array<int, TILE_TYPE_COUNT> getHandTileId() const;

	//appendix
	inline int getNeedMeldNumber() const { return getTileNumber() / 3; /* WRONG? */ };
	inline bool canPong(const Tile& oTile) const { return m_vTileNumbers[oTile] >= 2; };
	inline bool canKong(const Tile& oTile) const { return m_vTileNumbers[oTile] == 3; };
	inline bool canDarkKong(const Tile& oTile) const { return m_vTileNumbers[oTile] == 4; };
	inline bool canEat(const Tile& oTile) const { return canEatLeft(oTile) || canEatMiddle(oTile) || canEatRight(oTile); };
	bool canEatRight(const Tile& oTile) const;
	bool canEatMiddle(const Tile& oTile) const;
	bool canEatLeft(const Tile& oTile) const;
	bool canDarkKong() const;
	vector<Tile> getCanDarkKongTile() const;
	bool isAloneTile(const Tile& oTile) const;
	bool willBeAloneTile(const Tile& oTile) const;
	bool isTooManyMeld(const int& iNeedMeldCount) const;

	int getId(boost::shared_ptr<int[]>& vIds) const;
	inline int getMinLack() const {
		boost::shared_ptr<int[]> vIds;
		getId(vIds);
		return MinLackTable_old::getMinLackNumber(vIds.get());
	};
	inline int getMinLack(const int& iNeedGroup) const {
		boost::shared_ptr<int[]> vIds;
		getId(vIds);
		return MinLackTable_old::getMinLackNumber(vIds.get(), iNeedGroup, NEED_EYEs);
	};
	inline bool isWin() const { return isWin(getNeedMeldNumber()); };
	inline bool isWin(const int& iNeedMeldNumber) const {
		//return WinTable::isWin(iNeedMeldNumber, getId().get()); //BUG: not working correctly
		return getMinLack(iNeedMeldNumber) == 0;
	};
	inline bool canWin(const Tile& oTile) const { return canWin(oTile, getNeedMeldNumber()); };
	bool canWin(const Tile& oTile, const int& iMeldNumber) const;

	bool operator==(const HandTile_old& rhs) const;
	inline bool operator!=(const HandTile_old& rhs) const { return !(*this == rhs); };
	//void operator=(const HandTile_old& rhs);

	friend class HandTileTest_getTileTest_Test;

private:
	array<int, MAX_DIFFERENT_TILE_COUNT> m_vTileNumbers;
	//int m_vTileNumbers[MAX_DIFFERENT_TILE_COUNT];
	int m_iTotalTileNumber;
};
