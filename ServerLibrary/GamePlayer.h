#pragma once
#include <vector>
#include <memory>
#include <fstream>
#include "../CommunicationLibrary/BasePlayer.h"
using std::vector;
using std::string;
using std::fstream;

class GamePlayer
{
public:
	GamePlayer();
	GamePlayer(std::unique_ptr<BasePlayer>& pPlayer);
	~GamePlayer();

	inline PlayerTile& getPlayerTile() const { return m_pPlayer->m_oPlayerTile; };
	inline RemainTile_t& getRemainTile() const { return m_pPlayer->m_oRemainTile; };
	inline vector<int> getHandIdTile() const { return m_vHandIdTile; };
	inline vector<Tile> getThrownTiles() const { return m_vThrownTiles; };

	void initSeat(const int& iSeat);
	void initGame(const int& iTableWind, const int& iRound, const int& iDealer);			// initGame( [1, 4], [1, 4], [1, 4] )
	void initHandTile(const vector<int>& vHandTile);
	bool askWin();
	bool askWin(const int& iTile);
	int askThrow();
	vector<int> askChow(const int& iTile);
	vector<int> askPong(const int& iTile, const bool& bCanChow);
	bool askKong(const int& iTile);
	std::pair<MoveType, int> askDarkKongOrUpgradeKong();

	void drawTile(const int& iTile, const bool& bInitializingHand = false);
	int throwTile(const int& iSeat, const int& iTile, const bool& bPreviousMoveIsDraw);
	//int throwTile_Tile(const Tile& oTile);
	int eatRight(const int& iSeat, const vector<int>& vTiles);
	int eatMiddle(const int& iSeat, const vector<int>& vTilese);
	int eatLeft(const int& iSeat, const vector<int>& vTiles);
	int pong(const int& iSeat, const vector<int>& vTiles);
	int kong(const int& iSeat, const vector<int>& vTiles);
	int darkKong(const int& iTile);
	int darkKong_otherPlayer(const int& iSeat);
	int upgradeKong(const int& iSeat, const int& iTile);
	int winByChunk(const int& iWinnerSeat, const int& iTile, const int& iChunkerSeat, vector<int> vWinningHandTile);
	int winBySelfDraw(const int& iWinnerSeat, const int& iTile, vector<int> vWinningHandTile);
	int endGame();

	inline bool canWin() const { return getPlayerTile().isWin(); };
	inline bool canWin(const int& iTile) const { return getPlayerTile().getHandTile().canWin(Tile::sgfIdToTile(iTile), getPlayerTile().getHandTile().getNeedMeldNumber()); };
	inline bool canChow(const int& iTile) const { return getPlayerTile().getHandTile().canEat(Tile::sgfIdToTile(iTile)); };
	inline bool canPong(const int& iTile) const { return getPlayerTile().getHandTile().canPong(Tile::sgfIdToTile(iTile)); };
	inline bool canKong(const int& iTile) const { return getPlayerTile().getHandTile().canKong(Tile::sgfIdToTile(iTile)); };
	inline bool canDarkKongOrUpgradeKong() const { return getPlayerTile().canDarkKong() || getPlayerTile().canUpgradeKong(); };

	void setLeftTileCount(const int& iTileCount);

protected:
	inline PlayerTile& _getPlayerTile() { return m_pPlayer->m_oPlayerTile; };
	void _putTileToHand(const int& iTile);
	int _popTileFromHandIdTile_id(const int& iTile, const bool& bAlert = true);
	int _popTileFromHandIdTile_Tile(const Tile& oTile, const int& iCount = 1);
	int _getTileIdFromHand(const Tile& oTile) const;
	vector<int> _getTileIdsFromHand(const Tile& oTile, const int& iCount) const;

	//iTile related (maybe move if new class "IdTile" is created?)
	inline int _getIdTileType(const int& iTile) const { return iTile / PROTOCOL_SYMBOL_PLACE; }; //[1, TILE_TYPE_COUNT]
	inline int _getIdTileRank(const int& iTile) const { return iTile / PROTOCOL_NUMBER_PLACE % 10; }; //[1, MAX_RANK]

	void _startLogCommand(const string& sLogName);
	void _endLogCommand();
	void _logCommand(const string& sCommand);
	void _logCommand(const vector<string>& vCommands);
	void _logResponds(const string& sCommand);
	void _logResponds(const vector<string>& vCommands);

public:
	int m_iSeat; //[1, 4]
	int m_iPlayerId;//[0, 3], id will not change after shuffling seat

private:
	std::unique_ptr<BasePlayer> m_pPlayer;
	vector<int> m_vHandIdTile;//Tiles with format using 3 digits: PROTOCOL_SYMBOL_PLACE, PROTOCOL_NUMBER_PLACE, PROTOCOL_ID_PLACE
	vector<Tile> m_vThrownTiles;
	string m_sName;
	int m_iTableWind;									// [1, PLAYER]
	int m_iRound;											// [1, PLAYER]
	int m_iDealer;											// [1, PLAYER]
	fstream m_fCommandLog;

	static string g_sLogDirName;
};

