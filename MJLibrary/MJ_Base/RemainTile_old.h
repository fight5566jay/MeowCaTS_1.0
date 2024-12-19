#pragma once
#include <string>
#include "HandTile.h"
#include "RemainTile.h"
#include <iostream>
using std::string;
using std::cerr;
using std::endl;

class RemainTile_old {
public:
	RemainTile_old() : RemainTile_old(RemainTileType::Type_Original) {};
	RemainTile_old(const RemainTileType& oType) { init(oType); };
	RemainTile_old(const string& sRemainTile, const RemainTileType& oType);

	void init(const RemainTileType& oType);
	void addTile(const Tile& oTile, const int& iNumber = 1);
	void popTile(const Tile& oTile, const int& iNumber = 1, const bool& bDrawFront = true);
	void undoPopTile(const Tile& oTile, const int& iNumber = 1, const bool& bDrawFront = true);
	Tile randomPopTile();
	Tile randomPopTileByHandTile(const HandTile& oHandTile, const int& iMeldNumber);
	double getDangerousFactor(const Tile& oTile) const;

	inline RemainTileType getRemainTileType() const { return m_oRemainTileType; }
	inline int getRemainNumber() const { return m_iTotalRemainNumber; };
	inline int getRemainNumber(const Tile& oTile) const { return m_vRemainNumber[oTile]; };
	inline int getRemainDrawNumber() const { return m_iRemainDrawNumber; };
	//inline const int* getId() const { return m_vId; };
	//inline void drawUnKnown() { m_iRemainDrawNumber -= 1; };
	inline float getDrawRate(const Tile& oTile) const { return static_cast<float>(getRemainNumber(oTile)) / getRemainNumber(); };
	string toString() const;
	string getReadableString() const;
	inline void emptyPopTile() {
		//other player throw tile
		m_iRemainDrawNumber--;
	};

	//void operator=(const RemainTile& rhs);
	int operator[](const int& iIndex) const { return m_vRemainNumber[iIndex]; };
	bool operator==(const RemainTile_old& rhs) const;

private:
	RemainTileType m_oRemainTileType;
	int m_vRemainNumber[MAX_DIFFERENT_TILE_COUNT];
	int m_iTotalRemainNumber;
	int m_iRemainDrawNumber;
	//int m_vId[TILE_TYPE_COUNT];


};
