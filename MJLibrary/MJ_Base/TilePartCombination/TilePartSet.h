#pragma once

#include <vector>
#include "TilePart.h"

using std::vector;

class TilePartSet
{
public:
	static vector<TilePartSet> getAllPossibleTripletAndSequence(vector<int>& vTileCounts, const bool& bNoSequence = false, const int& iGivenMinIndex = -1);

public:
	TilePartSet(bool legal = true) : /*m_bAllPair(false), */m_bLegal(legal) {};
	TilePartSet(vector<TilePart> vTileParts) : m_vTilePart(vTileParts), m_bLegal(true) {};
	~TilePartSet() {};

public:
	
	bool isLegal() const;
	int getSize() const;
	void addToTilePart(const TilePart& tilePart);
	void addToTilePartAndGreedyMerge(const TilePart& tilePart, const bool& bIsHonor);
	string toString() const;
	inline TilePart at(const int& iIndex) const { return m_vTilePart.at(iIndex); };
	inline void erase(const int& iIndex) { m_vTilePart.erase(m_vTilePart.begin() + iIndex); };
	vector<TilePart> getTileParts() const { return m_vTilePart; };

	/*
	//only used in Jpn Mahjong
	bool isAllPair() const;
	void setToAllPair();
	void addToAllPairTilePart(const TilePart& tilePart);
	TilePartSet getAllPairTilePart() const;
	*/

private:
	vector<TilePart> m_vTilePart;
	//bool m_bAllPair;
	//vector<TilePart> m_vAllPairTilePart;
	bool m_bLegal;

public:
	TilePartSet operator+(const TilePartSet& rhs) const;
	TilePart& operator[](const int& index);
	bool operator==(const TilePartSet& rhs) const;
};