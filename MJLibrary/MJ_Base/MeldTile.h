#pragma once
#include "Tile.h"
#include <vector>
using std::vector;

//need to seperate enum class to MeldType (meld tiles (n.)) and MakeMeldType(meld (vt.))
enum class MeldType { Meld_Pass, Meld_EatRight, Meld_EatMiddle, Meld_EatLeft, Meld_Pong, Meld_Kong, Meld_DarkKong, Meld_Undefined };

class MeldTile {
public:
	MeldTile();
	MeldTile(const Tile& oTargetTile, const MeldType& oMeldType, const int& iTargetPlayer = -1);
	MeldTile(const string& sInput);
	~MeldTile() {};

	vector<Tile> getTiles() const;
	inline Tile getTargetTile() const { return m_oTargetTile; };
	inline MeldType getMeldType() const { return m_oMeldType; };
	inline int getTargetPlayer() const { return m_iTargetPlayer; };
	Tile getMinRankTile() const;
	void upgradeKong();
	string toString() const;


	bool operator==(const MeldTile& rhs) const { return m_oTargetTile == rhs.m_oTargetTile && m_oMeldType == rhs.m_oMeldType; };
	bool operator!=(const MeldTile& rhs) const { return !(*this == rhs); };

private:
	string getMeldString() const;

private:
	Tile m_oTargetTile;
	MeldType m_oMeldType;
	int m_iTargetPlayer;
};