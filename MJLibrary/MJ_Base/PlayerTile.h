#pragma once
#include "MJBaseTypedef.h"
#include "MeldTile.h"
#include "Move.h"


class PlayerTile {
public:
	PlayerTile() {};
	PlayerTile(const HandTile_t& oHandTile, const vector<MeldTile>& vMeldTiles);
	PlayerTile(const string& sInput);
	~PlayerTile() {};

	void clear();
	inline void putTileToHandTile(const Tile& oTile) { m_oHandTile.putTile(oTile); };
	inline void popTileFromHandTile(const Tile& oTile) { m_oHandTile.popTile(oTile); };
	inline void popAllTileFromHandTile() { m_oHandTile.clear(); };
	inline void popAllTileFromHandTile(const Tile& oTile) { m_oHandTile.popAllTile(oTile); };
	void doAction(const Move& oMove);
	void undoAction(const Move& oMove);
	void eat(const vector<Tile>& vTile);
	void eatRight(const Tile& oTile);
	void eatMiddle(const Tile& oTile);
	void eatLeft(const Tile& oTile);
	void pong(const Tile& oTile);
	void kong(const Tile& oTile);
	void darkKong(const Tile& oTile);
	void upgradeKong(const Tile& oTile);

	
	inline int getHandTileNumber() const { return m_oHandTile.getTileNumber(); }
	inline int getHandTileNumber(const Tile& oTile) const { return m_oHandTile.getTileNumber(oTile); }
	inline int getMeldCount() const { return m_vMeldTiles.size(); }
	inline HandTile_t getHandTile() const { return m_oHandTile; }
	inline vector<MeldTile> getMeldTiles() const { return m_vMeldTiles; }
	inline MeldTile getMeldTile(const int& iIndex) const { assert(iIndex >= 0 && iIndex < m_vMeldTiles.size()); return m_vMeldTiles[iIndex]; }
	inline int getMinLack() const { return m_oHandTile.getMinLack(); }
	inline int getMinLack(const int& iNeedGroup) const { return m_oHandTile.getMinLack(iNeedGroup - m_vMeldTiles.size()); };
	inline bool isWin(const int& iNeedGroup = NEED_GROUP) const { return m_oHandTile.isWin(iNeedGroup - m_vMeldTiles.size()); }
	inline bool canWin(const Tile& oTile, const int& iNeedGroup = NEED_GROUP) const { return m_oHandTile.canWin(oTile, iNeedGroup - getMeldCount()); }
	inline bool canPong(const Tile& oTile) const { return m_oHandTile.canPong(oTile); }
	inline bool canKong(const Tile& oTile) const { return m_oHandTile.canKong(oTile); }
	inline bool canDarkKong(const Tile& oTile) const { return m_oHandTile.canDarkKong(oTile); }
	inline bool canEat(const Tile& oTile) const { return m_oHandTile.canEat(oTile); }
	inline bool canEatRight(const Tile& oTile) const { return m_oHandTile.canEatRight(oTile); }
	inline bool canEatMiddle(const Tile& oTile) const { return m_oHandTile.canEatMiddle(oTile); }
	inline bool canEatLeft(const Tile& oTile) const { return m_oHandTile.canEatLeft(oTile); }
	inline bool canDarkKong() const { return m_oHandTile.canDarkKong(); }
	bool canUpgradeKong(const Tile& oTile) const;
	bool canUpgradeKong() const;
	inline vector<Tile> getCanDarkKongTile() const { return m_oHandTile.getCanDarkKongTile(); };
	vector<Tile> getUpgradeKongTile() const;
	inline bool isAloneTile(const Tile& oTile) const { return m_oHandTile.isAloneTile(oTile); };
	inline bool willBeAloneTile(const Tile& oTile) const { return m_oHandTile.willBeAloneTile(oTile); };
	inline bool isTooManyMeld(const int& iNeedMeldCount) const { return m_oHandTile.isTooManyMeld(iNeedMeldCount); };

	string toString() const;

	//void operator=(const PlayerTile& rhs);
	bool operator==(const PlayerTile& rhs) const;
	inline bool operator!=(const PlayerTile& rhs) const { return !(*this == rhs); };

private:
	HandTile_t m_oHandTile;
	vector<MeldTile> m_vMeldTiles;
};
