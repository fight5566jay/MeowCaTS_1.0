#include "TilePart.h"
#include "../../Base/Tools.h"
using std::cerr;
using std::endl;

TilePart::TilePart(const MeldTile & oMeldTile)
{
	m_oTile = oMeldTile.getMinRankTile();
	switch (oMeldTile.getMeldType()) {
	case MeldType::Meld_EatRight:
	case MeldType::Meld_EatMiddle:
	case MeldType::Meld_EatLeft:
		m_oType = TilePartType::Type_Eat;
		break;
	case MeldType::Meld_Pong:
		m_oType = TilePartType::Type_Pong;
		break;
	case MeldType::Meld_Kong:
		m_oType = TilePartType::Type_Kong;
		break;
	case MeldType::Meld_DarkKong:
		m_oType = TilePartType::Type_DarkKong;
		break;
	default:
		std::cerr << "[TilePart] Illegal MeldType: " << oMeldTile.toString() << std::endl;
		assert(0);
	}
}

vector<Tile> TilePart::getLackTileList()
{
	vector<Tile> vList;
	switch (m_oType) {
	case TilePartType::Type_SubSequence_MiddleHole:
		vList.push_back(Tile(m_oTile + 1));
		break;
	case TilePartType::Type_SubSequence_TwoSide:
		if(m_oTile.getRank() > 1)
			vList.push_back(Tile(m_oTile - 1));
		if (m_oTile.getRank() < MAX_SUIT_TILE_RANK - 1)
			vList.push_back(Tile(m_oTile + 2));
		break;
	case TilePartType::Type_Pair:
		vList.push_back(m_oTile);
		break;
	/*case Type_Alone://Huge time cost. Plz skip if no need
		if (m_iNumber > 2)
			vList.push_back(m_iNumber - 2);
		if (m_iNumber > 1)
			vList.push_back(m_iNumber - 1);
		if (m_iNumber < MAX_SUIT_TILE_RANK)
			vList.push_back(m_iNumber + 1);
		if (m_iNumber < MAX_SUIT_TILE_RANK - 1)
			vList.push_back(m_iNumber + 2);
		vList.push_back(m_iNumber);
		break;*/
	default:
		break;
	}

	return vList;
}

string TilePart::toString() const
{
	if (m_oType == TilePartType::Type_Triplet)
		return toStr(m_oTile.getRank() * 111);
	else if (m_oType == TilePartType::Type_Sequence)
		return toStr(m_oTile.getRank() * 111 + 12);
	else if (m_oType == TilePartType::Type_Pair)
		return toStr(m_oTile.getRank() * 11);
	else if (m_oType == TilePartType::Type_Eat)
		return toStr(m_oTile.getRank() * 111 + 12);
	else if (m_oType == TilePartType::Type_Pong)
		return toStr(m_oTile.getRank() * 111);
	else if (m_oType == TilePartType::Type_Kong)
		return toStr(m_oTile.getRank() * 1111);
	else if (m_oType == TilePartType::Type_DarkKong)
		return toStr(m_oTile.getRank() * 1111);
	else if (m_oType == TilePartType::Type_Alone)
		return toStr(m_oTile.getRank());
	else if (m_oType == TilePartType::Type_SubSequence_TwoSide)
		return toStr(m_oTile.getRank() * 11 + 1);
	else if (m_oType == TilePartType::Type_SubSequence_MiddleHole)
		return toStr(m_oTile.getRank() * 11 + 2);
	else
		return "None";
}