#pragma once

#include <string>
#include "../Wind.h"
#include "../MeldTile.h"

using std::string;

enum class TilePartType { Type_Triplet, Type_Sequence, Type_Eat, Type_Pong, Type_Kong, Type_DarkKong, Type_Pair, Type_SubSequence_TwoSide, Type_SubSequence_MiddleHole, Type_Alone, Type_None };

class TilePart
{
public:
	TilePart() : m_oType(TilePartType::Type_None) {};
	//TilePart(const TilePartType& type, const Tile& oTile) : m_oType(type),  m_oTile(oTile) {};
	TilePart(const TilePartType& type, const int oTileType, const int& iNumber) : m_oType(type) { m_oTile = Tile(oTileType, iNumber); };
	TilePart(const MeldTile& oMeldTile);

public:
	inline TilePartType getPartType() const { return m_oType; };
	inline int getTileType() const { return m_oTile.getTileType(); };
	inline int getNumber() const { return m_oTile.getRank(); };
	inline void setPartType(const TilePartType& type) { m_oType = type; };
	inline void setNumber(const int& iNumber) { m_oTile = Tile(m_oTile.getTileType(), iNumber); };
	inline Wind toWind() const { return Wind(m_oTile.getRank() - 1); };

	//iTile: 1~9
	inline bool canEat(const int& iTile) const { return canEatLeft(iTile) || canEatMiddle(iTile) || canEatRight(iTile); };
	inline bool canEatLeft(const int& iTile) const { return m_oType == TilePartType::Type_SubSequence_TwoSide && iTile == m_oTile.getRank() - 1; };
	inline bool canEatMiddle(const int& iTile) const { return m_oType == TilePartType::Type_SubSequence_MiddleHole && iTile == m_oTile.getRank(); };
	inline bool canEatRight(const int& iTile) const { return m_oType == TilePartType::Type_SubSequence_TwoSide && iTile == m_oTile.getRank() + 2; };
	inline bool canPong(const int& iTile) const { return (m_oType == TilePartType::Type_Pair/* || m_oType == TilePartType::Type_Triplet*/) && iTile == m_oTile.getRank(); };
	

	vector<Tile> getLackTileList();

	string toString() const;

	inline bool operator==(const TilePart& rhs) const { return m_oType == rhs.m_oType && m_oTile == rhs.m_oTile; };
	inline bool operator!=(const TilePart& rhs) const { return !(*this == rhs); };

private:
	TilePartType m_oType;
	Tile m_oTile;
};