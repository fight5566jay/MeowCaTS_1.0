#ifndef __WALL_H__
#define __WALL_H__

#include "../MJLibrary/MJ_Base/MahjongBaseConfig.h"
#include <vector>
#include <string>

using std::vector;
using std::string;

class Wall
{
public:
	Wall(const int& iSeed = -1);

public:
	inline int getTileFromFront() { return m_vTile[m_iFrontIndex++]; };
	inline int getTileFromRear() { return m_vTile[--m_iRearIndex]; };
	inline bool isEnd() const { return TOTAL_TILE_COUNT - m_iRearIndex + m_iFrontIndex  >= m_iUsedTileCountforGame; };
	inline void setTileCountUsedInGame(const int& iTileCount) { m_iUsedTileCountforGame = iTileCount; };
	inline int getLeftDrawCount() const { return m_iRearIndex - m_iFrontIndex - TOTAL_TILE_COUNT + m_iUsedTileCountforGame; };

	string getSgfString() const;
	string toString() const;
	inline int getTile(const int i) const { return m_vTile.at(i); };

private:
	vector<int> m_vTile;
	int m_iFrontIndex;
	int m_iRearIndex;
	int m_iUsedTileCountforGame;
};
#endif