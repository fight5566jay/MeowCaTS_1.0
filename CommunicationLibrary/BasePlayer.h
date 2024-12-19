#pragma once
#include "../MJLibrary/MJ_Base/PlayerTile.h"
#include "../MJLibrary/MJ_Base/MJBaseTypedef.h"

class BasePlayer
{
public:
	BasePlayer() : m_oRemainTile(RemainTile_t(RemainTileType::Type_Playing)) {};
	BasePlayer(const PlayerTile& oPlayerTile, const RemainTile_t& oRemainTile) : m_oPlayerTile(oPlayerTile), m_oRemainTile(oRemainTile) {};
	~BasePlayer() {};

	virtual void reset() = 0;

	virtual Tile askThrow() = 0;
	virtual MoveType askEat(const Tile& oTile) = 0;
	virtual bool askPong(const Tile& oTile, const bool& bCanChow, const bool& bCanKong) = 0;
	virtual bool askKong(const Tile& oTile) = 0;
	virtual std::pair<MoveType, Tile> askDarkKongOrUpgradeKong() = 0;
	virtual bool askWin() { return true; };
	virtual bool askWin(const Tile& oTile) { return true; };

	virtual void throwTile(const Tile& oTile) { m_oPlayerTile.popTileFromHandTile(oTile); };
	virtual void drawTile(const Tile& oTile) {
		m_oPlayerTile.putTileToHandTile(oTile);
		m_oRemainTile.popTile(oTile);
	}
	virtual void doAction(const Move& oMove) { m_oPlayerTile.doAction(oMove); };

public:
	PlayerTile m_oPlayerTile;
	RemainTile_t m_oRemainTile;
	uint32_t m_iSeat;								//[1, 4]
};

