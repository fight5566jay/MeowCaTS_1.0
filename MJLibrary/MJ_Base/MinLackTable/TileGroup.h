#pragma once
#include <string>
#include <array>
#include "../MahjongBaseConfig.h"
using std::string;
using std::array;

class TileGroup
{
public:
	//TileGroup();
	TileGroup(const int& id = 0) : m_iID(id) { init(); m_sTiles = convertIdToString(id); };
	TileGroup(const string &tiles) : m_sTiles(tiles), m_iID(convertStringToId(tiles)) { init(); };
	~TileGroup() {};

	inline bool isEmpty() const { return m_iID == 0; };
	inline string toString() const { return m_sTiles; };
	int getTileNumber() const;
	bool hasPair() const;
	bool hasMeld() const;
	bool canPickOut(const TileGroup& tiles) const;
	bool canPickOut(const string& tiles) const;
	void pickOut(const TileGroup& tiles);
	bool canPutIn(const TileGroup& tiles);
	bool canPutIn(const string& tiles);
	void putIn(const TileGroup& tiles);
	bool isOnePair() const;
	int getFirstPair() const;
	bool isAllPair() const;

	char operator[](const int& index) const { return m_sTiles[index]; };
	TileGroup operator=(const int& id);
	TileGroup& operator++();
	bool operator==(const string& rhs) const { return m_iID == convertStringToId(rhs); };
	bool operator!=(const string& rhs) const { return m_iID != convertStringToId(rhs); };
	bool operator==(const TileGroup& rhs) const { return m_iID == rhs.m_iID; };
	bool operator!=(const TileGroup& rhs) const { return m_iID != rhs.m_iID; };
	operator int() const { return (int)m_iID; };

private:
	void init();
	void setupRegularId();
	static string convertIdToString(const int& id);
	static int convertStringToId(const string& tiles);

public:
	static array<TileGroup, MAX_RANK> g_allTile;
	static array<TileGroup, MAX_SUIT_TILE_RANK - 2 + MAX_RANK> g_allMeld;
	static array<TileGroup, MAX_RANK> g_allPair;
	static array<TileGroup, MAX_SUIT_TILE_RANK * 2 - 3> g_allSubsequ;
	//static size_t g_iAllTileSize;
	//static size_t g_iAllMeldSize;
	//static size_t g_iAllPairSize;
	//static size_t g_iAllSequSize;

	static bool g_bFirstInit;

private:
	uint64_t m_iID;//[future work] how to implement if id is greater than maximum of uint64_t?
	string m_sTiles;
	static string m_sInitTiles;
};

/*extern TileGroup allTile[9];
extern TileGroup allMeld[16];
extern TileGroup allPair[9];
extern TileGroup allSequ[15];
extern TileGroup allTriplet[9];*/