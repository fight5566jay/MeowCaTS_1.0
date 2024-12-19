#include "PlayerTile.h"
#include <iostream>
#include <cassert>
using std::cerr;
using std::endl;

PlayerTile::PlayerTile(const HandTile_t& oHandTile, const vector<MeldTile>& vMeldTiles) : m_oHandTile(oHandTile), m_vMeldTiles(vMeldTiles)
{
#ifdef MJ_DEBUG_MODE
	assert(vMeldTiles.size() <= MAX_MELD_COUNT);
#endif
}

PlayerTile::PlayerTile(const string& sInput)
{
	int iPos = sInput.find('(');
	if (iPos == string::npos) {
		m_oHandTile = HandTile_t(sInput);
	}
	else {
		m_oHandTile = HandTile_t(sInput.substr(0, iPos));
		string sMeldTiles = sInput.substr(iPos + 1, sInput.length() - iPos - 2);
		stringstream ss(sMeldTiles);
		string tok;
		while (getline(ss, tok, ' ')) {
			if (tok == "")
				continue;
			m_vMeldTiles.push_back(MeldTile(tok));
		}
	}
}

void PlayerTile::clear()
{
	m_oHandTile.clear();
	m_vMeldTiles.clear();
}

void PlayerTile::doAction(const Move& oMove)
{
	Tile oTile = oMove.getTargetTile();
	switch (oMove.getMoveType()) {
	case MoveType::Move_Pass:
		assert(m_oHandTile.getTileNumber() % 3 == 1);
		break;
	case MoveType::Move_Draw:
		assert(m_oHandTile.getTileNumber() % 3 == 1);
		if (!oTile.isNull())
			m_oHandTile.putTile(oTile);
		break;
	case MoveType::Move_Throw:
		assert(m_oHandTile.getTileNumber() % 3 == 2);
		m_oHandTile.popTile(oTile);
		break;
	case MoveType::Move_EatLeft:
		assert(m_oHandTile.getTileNumber() % 3 == 1);
		eatLeft(oTile);
		break;
	case MoveType::Move_EatMiddle:
		assert(m_oHandTile.getTileNumber() % 3 == 1);
		eatMiddle(oTile);
		break;
	case MoveType::Move_EatRight:
		assert(m_oHandTile.getTileNumber() % 3 == 1);
		eatRight(oTile);
		break;
	case MoveType::Move_Pong:
		assert(m_oHandTile.getTileNumber() % 3 == 1);
		pong(oTile);
		break;
	case MoveType::Move_Kong:
		assert(m_oHandTile.getTileNumber() % 3 == 1);
		kong(oTile);
		break;
	case MoveType::Move_DarkKong:
		assert(m_oHandTile.getTileNumber() % 3 == 2);
		darkKong(oTile);
		break;
	case MoveType::Move_UpgradeKong:
		assert(m_oHandTile.getTileNumber() % 3 == 2);
		upgradeKong(oTile);
		break;
	case MoveType::Move_WinByOther:
		assert(m_oHandTile.getTileNumber() % 3 == 1);
		break;
	case MoveType::Move_WinBySelf:
		assert(m_oHandTile.getTileNumber() % 3 == 2);
		break;
	case MoveType::Move_DrawUselessTile:
		assert(m_oHandTile.getTileNumber() % 3 == 1);
		break;
	case MoveType::Move_Init:
		break;
	default:
		std::cerr << "[PlayerTile::doAction] Illegal action: " << oMove.toString() << std::endl;
		system("pause");
		exit(87);
	}
}

void PlayerTile::undoAction(const Move& oMove)
{
	Tile oTile = oMove.getTargetTile();
	switch (oMove.getMoveType()) {
	case MoveType::Move_Pass:
		break;
	case MoveType::Move_Draw:
		if (!oTile.isNull())
			m_oHandTile.popTile(oTile);
		break;
	case MoveType::Move_Throw:
		m_oHandTile.putTile(oTile);
		break;
	case MoveType::Move_EatLeft:
		for (int i = 0; i < m_vMeldTiles.size(); i++) {
			if (m_vMeldTiles[i].getMeldType() == MeldType::Meld_EatLeft && m_vMeldTiles[i].getMinRankTile() == oTile) {
				m_oHandTile.putTile(oTile + 1);
				m_oHandTile.putTile(oTile + 2);
				m_vMeldTiles.erase(m_vMeldTiles.begin() + i);
				break;
			}
		}
		break;
	case MoveType::Move_EatMiddle:
		for (int i = 0; i < m_vMeldTiles.size(); i++) {
			if (m_vMeldTiles[i].getMeldType() == MeldType::Meld_EatMiddle && m_vMeldTiles[i].getMinRankTile() == oTile - 1) {
				m_oHandTile.putTile(oTile - 1);
				m_oHandTile.putTile(oTile + 1);
				m_vMeldTiles.erase(m_vMeldTiles.begin() + i);
				break;
			}
		}
		break;
	case MoveType::Move_EatRight:
		for (int i = 0; i < m_vMeldTiles.size(); i++) {
			if (m_vMeldTiles[i].getMeldType() == MeldType::Meld_EatRight && m_vMeldTiles[i].getMinRankTile() == oTile - 2) {
				m_oHandTile.putTile(oTile - 2);
				m_oHandTile.putTile(oTile - 1);
				m_vMeldTiles.erase(m_vMeldTiles.begin() + i);
				break;
			}
		}
		break;
	case MoveType::Move_Pong:
		for (int i = 0; i < m_vMeldTiles.size(); i++) {
			if (m_vMeldTiles[i].getMeldType() == MeldType::Meld_Pong && m_vMeldTiles[i].getMinRankTile() == oTile) {
				m_oHandTile.putTile(oTile);
				m_oHandTile.putTile(oTile);
				m_vMeldTiles.erase(m_vMeldTiles.begin() + i);
				break;
			}
		}
		break;
	case MoveType::Move_Kong:
		for (int i = 0; i < m_vMeldTiles.size(); i++) {
			if (m_vMeldTiles[i].getMeldType() == MeldType::Meld_Kong && m_vMeldTiles[i].getMinRankTile() == oTile) {
				m_oHandTile.putTile(oTile);
				m_oHandTile.putTile(oTile);
				m_oHandTile.putTile(oTile);
				m_vMeldTiles.erase(m_vMeldTiles.begin() + i);
				break;
			}
		}
		break;
	case MoveType::Move_DarkKong:
		for (int i = 0; i < m_vMeldTiles.size(); i++) {
			if (m_vMeldTiles[i].getMeldType() == MeldType::Meld_DarkKong && m_vMeldTiles[i].getMinRankTile() == oTile) {
				m_oHandTile.putTile(oTile);
				m_oHandTile.putTile(oTile);
				m_oHandTile.putTile(oTile);
				m_oHandTile.putTile(oTile);
				m_vMeldTiles.erase(m_vMeldTiles.begin() + i);
				break;
			}
		}
		break;
	case MoveType::Move_UpgradeKong:
		for (int i = 0; i < m_vMeldTiles.size(); i++) {
			if (m_vMeldTiles[i].getMeldType() == MeldType::Meld_Kong && m_vMeldTiles[i].getMinRankTile() == oTile) {
				m_oHandTile.putTile(oTile);
				//m_vMeldTiles.insert(m_vMeldTiles.begin() + i + 1, MeldTile(oTile, MeldType::Meld_Pong));
				m_vMeldTiles.emplace(m_vMeldTiles.begin() + i + 1, oTile, MeldType::Meld_Pong);
				m_vMeldTiles.erase(m_vMeldTiles.begin() + i);
				break;
			}
		}
		break;
	case MoveType::Move_Init:
	case MoveType::Move_WinByOther:
	case MoveType::Move_WinBySelf:
	case MoveType::Move_DrawUselessTile:
		break;
	default:
		std::cerr << "[PlayerTile::doAction] Illegal action: " << oMove.toString() << std::endl;
		system("pause");
		exit(87);
	}
}

void PlayerTile::eat(const vector<Tile>& vTiles)
{
	assert(vTiles.size() == 3);
	if (vTiles[0] > vTiles[1] && vTiles[0] > vTiles[2])
		eatRight(vTiles[0]);
	else if (vTiles[0] < vTiles[1] && vTiles[0] < vTiles[2])
		eatLeft(vTiles[0]);
	else
		eatMiddle(vTiles[0]);
}

void PlayerTile::eatRight(const Tile& oTile)
{
	m_vMeldTiles.push_back(MeldTile(oTile, MeldType::Meld_EatRight));
	m_oHandTile.popTile(oTile - 1);
	m_oHandTile.popTile(oTile - 2);
}

void PlayerTile::eatMiddle(const Tile& oTile)
{
	m_vMeldTiles.push_back(MeldTile(oTile, MeldType::Meld_EatMiddle));
	m_oHandTile.popTile(oTile - 1);
	m_oHandTile.popTile(oTile + 1);
}

void PlayerTile::eatLeft(const Tile& oTile)
{
	m_vMeldTiles.push_back(MeldTile(oTile, MeldType::Meld_EatLeft));
	m_oHandTile.popTile(oTile + 1);
	m_oHandTile.popTile(oTile + 2);
}

void PlayerTile::pong(const Tile& oTile)
{
	m_vMeldTiles.push_back(MeldTile(oTile, MeldType::Meld_Pong));
	m_oHandTile.popTile(oTile, 2);
}

void PlayerTile::kong(const Tile& oTile)
{
	m_vMeldTiles.push_back(MeldTile(oTile, MeldType::Meld_Kong));
	m_oHandTile.popTile(oTile, 3);
}

void PlayerTile::darkKong(const Tile& oTile)
{
	m_vMeldTiles.push_back(MeldTile(oTile, MeldType::Meld_DarkKong));
	m_oHandTile.popTile(oTile, 4);
}

void PlayerTile::upgradeKong(const Tile& oTile)
{
	int iMeldCount = m_vMeldTiles.size();
	for (int i = 0; i < iMeldCount; ++i) {
		if (m_vMeldTiles[i].getMeldType() == MeldType::Meld_Pong && m_vMeldTiles[i].getTiles().at(0) == oTile) {
			m_vMeldTiles[i].upgradeKong();
			m_oHandTile.popTile(oTile);
			break;
		}
	}
}

bool PlayerTile::canUpgradeKong(const Tile & oTile) const
{
	if (m_oHandTile.getTileNumber(oTile) < 1) {
		return false;
	}

	int iSize = m_vMeldTiles.size();
	for (int i = 0; i < iSize; i++) {
		if (m_vMeldTiles[i].getMeldType() == MeldType::Meld_Pong && m_vMeldTiles[i].getTargetTile() == oTile)
			return true;
	}
	return false;
}

bool PlayerTile::canUpgradeKong() const
{
	/*for (int i = 0; i < MAX_DIFFERENT_TILE_COUNT; ++i) {
		if (canUpgradeKong(i))
			return true;
	}
	return false;*/
	int iMeldCount = m_vMeldTiles.size();
	Tile oPongTile;
	for (int i = 0; i < iMeldCount; i++) {
		if (m_vMeldTiles[i].getMeldType() != MeldType::Meld_Pong)
			continue;

		oPongTile = m_vMeldTiles[i].getTargetTile();
		if (m_oHandTile.getTileNumber(oPongTile) == 1)
			return true;
	}
	return false;
}

vector<Tile> PlayerTile::getUpgradeKongTile() const
{
	vector<Tile> result;
	/*for (int i = 0; i < m_vMeldTiles.size(); ++i) {
		if (m_vMeldTiles[i].getMeldType() == MeldType::Meld_Pong && getHandTileNumber(m_vMeldTiles[i].getMinRankTile()) == 1)
			result.push_back(m_vMeldTiles[i].getMinRankTile());
	}*/
	int iMeldCount = m_vMeldTiles.size();
	Tile oTargetTile;
	for (int i = 0; i < iMeldCount; i++) {
		if (m_vMeldTiles[i].getMeldType() != MeldType::Meld_Pong)
			continue;

		oTargetTile = m_vMeldTiles[i].getTargetTile();
		if (m_oHandTile.getTileNumber(oTargetTile) == 1)
			result.push_back(oTargetTile);
	}
	return result;
}

string PlayerTile::toString() const
{
	string str = m_oHandTile.toString();
	if (!m_vMeldTiles.empty()) {
		str += " (";
		for (MeldTile oMeld : m_vMeldTiles) {
			str += " " + oMeld.toString();
		}
		str += " )";
	}
	return str;
}

/*void PlayerTile::operator=(const PlayerTile & rhs)
{
	m_oHandTile = rhs.m_oHandTile;
	m_vMeldTiles.clear();
	m_vMeldTiles.reserve(rhs.m_vMeldTiles.size());
	for (auto meld : rhs.m_vMeldTiles) {
		m_vMeldTiles.push_back(meld);
	}
}*/

bool PlayerTile::operator==(const PlayerTile & rhs) const
{
	if (m_oHandTile != rhs.m_oHandTile || m_vMeldTiles.size() != rhs.m_vMeldTiles.size())
		return false;
	
	array<int, DIFF_SUIT_TILE_COUNT + 3 * MAX_DIFFERENT_TILE_COUNT> iCheckBox1 = { 0 };
	array<int, DIFF_SUIT_TILE_COUNT + 3 * MAX_DIFFERENT_TILE_COUNT> iCheckBox2 = { 0 };
	const int iEatStartIndex = 0;
	const int iPongStartIndex = iEatStartIndex + DIFF_SUIT_TILE_COUNT;//not actually, but enough
	const int iKongStartIndex = iPongStartIndex + MAX_DIFFERENT_TILE_COUNT;
	const int iDarkKongStartIndex = iKongStartIndex + MAX_DIFFERENT_TILE_COUNT;

	//checkbox of m_vMeldTiles
	for (int i = 0; i < m_vMeldTiles.size(); i++) {
		switch (m_vMeldTiles[i].getMeldType()) {
		case MeldType::Meld_EatLeft:
		case MeldType::Meld_EatMiddle:
		case MeldType::Meld_EatRight:
			iCheckBox1[iEatStartIndex + m_vMeldTiles[i].getMinRankTile()]++;
			break;
		case MeldType::Meld_Pong:
			iCheckBox1[iPongStartIndex + m_vMeldTiles[i].getMinRankTile()]++;
			break;
		case MeldType::Meld_Kong:
			iCheckBox1[iKongStartIndex + m_vMeldTiles[i].getMinRankTile()]++;
			break;
		case MeldType::Meld_DarkKong:
			iCheckBox1[iDarkKongStartIndex + m_vMeldTiles[i].getMinRankTile()]++;
			break;
		default:
			std::cerr << "[PlayerTile::operator==] Illegal MeldType of m_vMeldTiles: " << m_vMeldTiles[i].toString() << std::endl;
			assert(0);
			break;
		}
	}

	//checkbox of rhs.m_vMeldTiles
	for (int i = 0; i < rhs.m_vMeldTiles.size(); i++) {
		switch (rhs.m_vMeldTiles[i].getMeldType()) {
		case MeldType::Meld_EatLeft:
		case MeldType::Meld_EatMiddle:
		case MeldType::Meld_EatRight:
			iCheckBox2[iEatStartIndex + rhs.m_vMeldTiles[i].getMinRankTile()]++;
			break;
		case MeldType::Meld_Pong:
			iCheckBox2[iPongStartIndex + rhs.m_vMeldTiles[i].getMinRankTile()]++;
			break;
		case MeldType::Meld_Kong:
			iCheckBox2[iKongStartIndex + rhs.m_vMeldTiles[i].getMinRankTile()]++;
			break;
		case MeldType::Meld_DarkKong:
			iCheckBox2[iDarkKongStartIndex + rhs.m_vMeldTiles[i].getMinRankTile()]++;
			break;
		default:
			std::cerr << "[PlayerTile::operator==] Illegal MeldType of rhs.m_vMeldTiles: " << rhs.m_vMeldTiles[i].toString() << std::endl;
			assert(0);
			break;
		}
	}

	int iSize = iCheckBox1.size();
	for (int i = 0; i < iSize; i++) {
		if (iCheckBox1.at(i) != iCheckBox2.at(i))
			return false;
	}

	return true;
}
