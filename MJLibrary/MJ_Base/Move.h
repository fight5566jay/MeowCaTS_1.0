#pragma once
#include <vector>
#include "MeldTile.h"
using std::vector;

enum class MoveType
{ Move_Init
, Move_Pass
, Move_Draw
, Move_Throw
, Move_EatLeft
, Move_EatMiddle
, Move_EatRight
, Move_Pong
, Move_Kong
, Move_DarkKong
, Move_UpgradeKong
, Move_WinByOther
, Move_WinBySelf
, Move_DrawUselessTile //special usage, you can consider it as empty move.
, Move_Unknown };


class Move {
public:
	static MoveType toMoveType(const MeldType& oMeldType);

	Move() : m_oMoveType(MoveType::Move_Init), m_iMainPlayer(1), m_iRelatedPlayer(-1) {};
	Move(const MoveType& oMoveType, const int& iPlayer = 1, const int& iRelatedPlayer = -1) : m_oMoveType(oMoveType), m_iMainPlayer(iPlayer), m_iRelatedPlayer(iRelatedPlayer) {};
	Move(const MoveType& oMoveType, const vector<Tile>& vTiles, const int& iPlayer = 1, const int& iRelatedPlayer = -1) : m_oMoveType(oMoveType), m_vTiles(vTiles), m_iMainPlayer(iPlayer), m_iRelatedPlayer(iRelatedPlayer)  {};
	Move(const MoveType& oMoveType, const Tile& oTile, const int& iPlayer = 1, const int& iRelatedPlayer = -1) : m_oMoveType(oMoveType), m_vTiles({ oTile }), m_iMainPlayer(iPlayer), m_iRelatedPlayer(iRelatedPlayer) {};
	//Move(const MeldType& oMeldType, const vector<Tile>& vTiles, const int& iPlayer = 1) { *this = Move(toMoveType(oMeldType), vTiles, iPlayer); };
	//Move(const MeldType& oMeldType, const Tile& oTile, const int& iPlayer = 1) { *this = Move(toMoveType(oMeldType), oTile, iPlayer); };

	inline int getMainPlayer() const { return m_iMainPlayer; };
	inline int getRelatedPlayer() const { return m_iRelatedPlayer; };
	//inline void setMoveType(const MoveType& moveType) { m_moveType = moveType; };
	inline bool isInitMove() const { return m_oMoveType == MoveType::Move_Init; };
	inline MoveType getMoveType() const { return m_oMoveType; };
	inline vector<Tile> getTile() const { return m_vTiles; };
	inline Tile getTargetTile() const { return m_vTiles.size() > 0 ? m_vTiles[0] : Tile(); };
	string toString() const;

	inline bool isEatRight() const { return m_vTiles[0] > m_vTiles[1] && m_vTiles[0] > m_vTiles[2]; }
	inline bool isEatMiddle() const { return (m_vTiles[0] > m_vTiles[1] && m_vTiles[0] < m_vTiles[2]) || (m_vTiles[0] < m_vTiles[1] && m_vTiles[0] > m_vTiles[2]); };
	inline bool isEatLeft() const { return m_vTiles[0] < m_vTiles[1] && m_vTiles[0] < m_vTiles[2]; };

	inline bool operator==(const Move& rhs) const { return m_oMoveType == rhs.m_oMoveType && m_vTiles == rhs.m_vTiles && m_iMainPlayer == rhs.m_iMainPlayer; };
	inline bool operator!=(const Move& rhs) const { return !(*this == rhs); };
	
private:
	MoveType m_oMoveType;
	vector<Tile> m_vTiles;
	int m_iMainPlayer;	//1~4
	int m_iRelatedPlayer;//1~4
};