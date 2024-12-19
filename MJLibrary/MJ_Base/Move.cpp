#include "Move.h"
#include <iostream>
using std::cerr;
using std::endl;

MoveType Move::toMoveType(const MeldType& oMeldType)
{
	switch (oMeldType) {
	case MeldType::Meld_EatRight:
		return MoveType::Move_EatRight;
	case MeldType::Meld_EatMiddle:
		return MoveType::Move_EatMiddle;
	case MeldType::Meld_EatLeft:
		return MoveType::Move_EatLeft;
	case MeldType::Meld_Pong:
		return MoveType::Move_Pong;
	case MeldType::Meld_Kong:
		return MoveType::Move_Kong;
	case MeldType::Meld_DarkKong:
		return MoveType::Move_DarkKong;
	case MeldType::Meld_Pass:
		return MoveType::Move_Pass;
	default:
		return MoveType::Move_Unknown;
	}
}

string Move::toString() const
{
	switch (m_oMoveType) {
	case MoveType::Move_Init:
		return "Init";
	case MoveType::Move_Pass:
		return "Pass";
	case MoveType::Move_Draw:
		return "M" + m_vTiles.at(0).toString();
	case MoveType::Move_Throw:
		return "T" + m_vTiles.at(0).toString();
	case MoveType::Move_EatRight:
		return "ER" + m_vTiles.at(0).toString();
	case MoveType::Move_EatMiddle:
		return "EM" + m_vTiles.at(0).toString();
	case MoveType::Move_EatLeft:
		return "EL" + m_vTiles.at(0).toString();
	case MoveType::Move_Pong:
		return "P" + m_vTiles.at(0).toString();
	case MoveType::Move_Kong:
		return "K" + m_vTiles.at(0).toString();
	case MoveType::Move_DarkKong:
		return "DK" + m_vTiles.at(0).toString();
	case MoveType::Move_UpgradeKong:
		return "UK" + m_vTiles.at(0).toString();
	case MoveType::Move_WinByOther:
	case MoveType::Move_WinBySelf:
		return "W";
	case MoveType::Move_DrawUselessTile:
		return "M0";
	case MoveType::Move_Unknown:
		return "Unknown";
	default:
		std::cerr << "[Move::toString] Illegal type!" << std::endl;
		assert(0);
		return string();
	}
	return string();
}
