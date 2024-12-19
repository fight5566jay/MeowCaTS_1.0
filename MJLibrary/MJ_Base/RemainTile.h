#pragma once
#include <string>
#include "HandTile.h"
#include <iostream>
using std::string;
using std::array;
using std::cerr;
using std::endl;
enum class RemainTileType { Type_Original, Type_Playing };
typedef int RemainTileId_t;
typedef array<int, MAX_DIFFERENT_TILE_COUNT> RemainTileNumberArray;

class RemainTile {
public:
	RemainTile() : RemainTile(RemainTileType::Type_Original) {};
	RemainTile(const RemainTileType& oType) { init(oType); };
	RemainTile(const string& sRemainTile, const RemainTileType& oType);

	void init(const RemainTileType& oType);
	void addTile(const Tile& oTile, const int& iNumber = 1);
	void popTile(const Tile& oTile, const int& iNumber = 1, const bool& bDrawTile = true);
	void undoPopTile(const Tile& oTile, const int& iNumber = 1, const bool& bDrawTile = true);
	Tile randomPopTile();
	Tile randomPopTileByHandTile(const HandTile& oHandTile, const int& iMeldNumber);//[TODO] change to Tile randomPopTileByHandTile(const HandTileIdArray& vHandTileIds, const int& iMeldNumber)?
	inline void decreaseRemainDrawCount(const int& iNumber) { m_iRemainDrawNumber -= iNumber; };//for Server (other player dark kong)
	double getDangerousFactor(const Tile& oTile) const;

	inline RemainTileType getRemainTileType() const { return m_oRemainTileType; }
	inline int getRemainNumber() const { return m_iTotalRemainNumber; };
	int getRemainNumber(const Tile& oTile) const;
	int getRemainNumberList(RemainTileNumberArray& vTileNumbers);
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
	int operator[](const int& iIndex) const { return getRemainNumber(iIndex); };
	bool operator==(const RemainTile& rhs) const;

private:
	RemainTileType m_oRemainTileType;
	//int m_vRemainNumber[MAX_DIFFERENT_TILE_COUNT];
	//RemainTileId_t m_vIds[TILE_TYPE_COUNT];
	array<RemainTileId_t, TILE_TYPE_COUNT> m_vIds;
	int m_iTotalRemainNumber;
	int m_iRemainDrawNumber;
};
