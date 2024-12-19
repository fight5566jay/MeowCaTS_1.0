#include "MeldTile.h"
#include <iostream>
#include <cassert>
using std::cerr;
using std::endl;

MeldTile::MeldTile() : m_oTargetTile(Tile()), m_oMeldType(MeldType::Meld_Undefined), m_iTargetPlayer(-1)
{
	std::cerr << "[MeldTile::MeldTile] Don't use default constructor, use MeldTile::MeldTile(const Tile& , const MeldType& , const Player& ) instead." << std::endl;
	assert(m_oMeldType != MeldType::Meld_Undefined);
}

MeldTile::MeldTile(const Tile& oTargetTile, const MeldType& oMeldType, const int& iTargetPlayer) : m_oTargetTile(oTargetTile), m_oMeldType(oMeldType), m_iTargetPlayer(iTargetPlayer)
{
	assert(oMeldType == MeldType::Meld_Pass || !oTargetTile.isNull());//if pass then don't care, else the target tile should not be null tile.

	switch (m_oMeldType) {
	case MeldType::Meld_EatRight:
		assert(oTargetTile.isSuit() && oTargetTile.getRank() >= 2);
		break;
	case MeldType::Meld_EatMiddle:
		assert(oTargetTile.isSuit() && oTargetTile.getRank() >= 1 && oTargetTile.getRank() <= MAX_SUIT_TILE_RANK - 1);
		break;
	case MeldType::Meld_EatLeft:
		assert(oTargetTile.isSuit() && oTargetTile.getRank() <= MAX_SUIT_TILE_RANK - 2);
		break;
	//case MeldType::Meld_Pong:
	//case MeldType::Meld_Kong:
	//case MeldType::Meld_DarkKong:
	//case MeldType::Meld_Undefined:
	default:
		break;
	}
}

MeldTile::MeldTile(const string& sInput) : m_iTargetPlayer(-1)
{
	assert(!sInput.empty());
		
	//parse the input string
	//char cTileType = sInput.at(sInput.length() - 1);
	char cTileType = sInput.back();
	int iTileType;
	if (isupper(cTileType))
		iTileType = cTileType - 'A' + SUIT_COUNT;
	else if (islower(cTileType))
		iTileType = cTileType - 'a';
	else {
		std::cerr << "[MeldTile::MeldTile] illegal input string: " << sInput << std::endl;
		assert(isupper(cTileType) || islower(cTileType));
	}

	//setup m_oTargetTile
	int iTargetTileRank = sInput[0] - '0';
	m_oTargetTile = Tile(iTileType, iTargetTileRank);

	//setup m_oMeldType
	int iNextTileRank = sInput[1] - '0';
	if (sInput.length() == 5) {//2222m
		m_oMeldType = MeldType::Meld_Kong;//BUG: darkKong?
	}
	else if (iNextTileRank == iTargetTileRank) {//222m
		m_oMeldType = MeldType::Meld_Pong;
	}
	else if (iTargetTileRank - iNextTileRank == 2) {//312m
		m_oMeldType = MeldType::Meld_EatRight;
	}
	else if (iTargetTileRank - iNextTileRank == 1) {//213m
		m_oMeldType = MeldType::Meld_EatMiddle;
	}
	else if (iTargetTileRank - iNextTileRank == -1) {//123m
		m_oMeldType = MeldType::Meld_EatLeft;
	}
}

vector<Tile> MeldTile::getTiles() const
{
	if (m_oMeldType == MeldType::Meld_Pass) {
		return vector<Tile>();
	}

	vector<Tile> vTiles;
	vTiles.reserve(MAX_TILE_COUNT_IN_ONE_MELD);
	vTiles.push_back(m_oTargetTile);

	switch (m_oMeldType) {
	case MeldType::Meld_EatRight:
		vTiles.push_back(m_oTargetTile - 2);
		vTiles.push_back(m_oTargetTile - 1);
		break;
	case MeldType::Meld_EatMiddle:
		vTiles.push_back(m_oTargetTile - 1);
		vTiles.push_back(m_oTargetTile + 1);
		break;
	case MeldType::Meld_EatLeft:
		vTiles.push_back(m_oTargetTile + 1);
		vTiles.push_back(m_oTargetTile + 2);
		break;
	case MeldType::Meld_Pong:
		vTiles.push_back(m_oTargetTile);
		vTiles.push_back(m_oTargetTile);
		break;
	case MeldType::Meld_Kong:
	case MeldType::Meld_DarkKong:
		vTiles.push_back(m_oTargetTile);
		vTiles.push_back(m_oTargetTile);
		vTiles.push_back(m_oTargetTile);
		break;
	//case MeldType::Meld_Undefined:
	default:
		std::cerr << "[MeldTile::getTiles] Illegal meld type: " << getMeldString() << std::endl;
		assert(m_oMeldType != MeldType::Meld_Undefined);
		break;
	}

	return vTiles;
}

Tile MeldTile::getMinRankTile() const
{
	switch (m_oMeldType) {
	case MeldType::Meld_Pass:
		return Tile();
	case MeldType::Meld_EatRight:
		return m_oTargetTile - 2;
	case MeldType::Meld_EatMiddle:
		return m_oTargetTile - 1;
	case MeldType::Meld_EatLeft:
	case MeldType::Meld_Pong:
	case MeldType::Meld_Kong:
	case MeldType::Meld_DarkKong:
		return m_oTargetTile;
	//case MeldType::Meld_Undefined:
	default:
		std::cerr << "[MeldTile::getMinRankTile] Illegal meld type: " << getMeldString() << std::endl;
		break;
	}

	assert(m_oMeldType != MeldType::Meld_Undefined);
	return Tile();
}

void MeldTile::upgradeKong()
{
#ifdef MJ_DEBUG_MODE
	assert(m_oMeldType == MeldType::Meld_Pong);
#endif
	m_oMeldType = MeldType::Meld_Kong;
}

string MeldTile::toString() const
{
	if (m_oMeldType == MeldType::Meld_Pass) {
		//[BUG]it should be happen
		//need to seperate enum class to MeldType (meld tiles (n.)) and MakeMeldType(meld (vt.))
		return "Pass";
	}
	
	if (m_oMeldType == MeldType::Meld_Undefined) {
		return "UnknownMeld";
	}

	string str;
	vector<Tile> vTiles = getTiles();
	assert(!vTiles.empty());
	for (Tile oTile : vTiles) {
		str += std::to_string(oTile.getRank());
	}
	str += vTiles[0].getTileTypeStr();
	return str;
}

string MeldTile::getMeldString() const
{
	switch (m_oMeldType) {
	case MeldType::Meld_Pass:
		return "Pass";
	case MeldType::Meld_EatRight:
		return "EatRight";
	case MeldType::Meld_EatMiddle:
		return "EatMiddle";
	case MeldType::Meld_EatLeft:
		return "EatLeft";
	case MeldType::Meld_Pong:
		return "Pong";
	case MeldType::Meld_Kong:
		return "Kong";
	case MeldType::Meld_DarkKong:
		return "DarkKong";
	case MeldType::Meld_Undefined:
		return "Undefined";
	default:
		break;
	}
	return "Unknown";
}
