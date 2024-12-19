#pragma once
#include <string>
#include <array>
#include <memory>
#include <fstream>
#include "Wall.h"
#include "GamePlayer.h"
#include "../MJLibrary/Base/SGF.h"
using std::string;
using std::array;
using std::unique_ptr;
using std::ofstream;
typedef GamePlayer Player_t;
typedef array<unique_ptr<Player_t>, PLAYER_COUNT> PlayerList_t;

class GameServer_V2
{
public:
	GameServer_V2();
	~GameServer_V2();

	void init(PlayerList_t& vPlayers);
	void start(const Wall& oWall, const bool& bRandomParam = true);
	void printState() const;
	void switchSeat(const array<int, PLAYER_COUNT>& v_PlayerIds);
	void nextGame();

protected:
	void _initSeat();
	void _initGame();
	void _initHandTile();
	void _startGame();

	void _drawTile(const int& iDrawPlayer, const bool& bDrawFront);
	int _getNextPlayerIndex(const int& iCurrentPlayer);	//[0, PLAYER - 1]
	//int _getPreviousPlayerIndex(const int& iCurrentPlayer);	//[0, PLAYER - 1]
	//int _getNextPlayerIndex(const int& iCurrentPlayer, const int& iNextStep);
	void _updateScore(const array<int, PLAYER_COUNT>& vGameScores);
	void _initParameter();

protected:
	string m_sLog;
	PlayerList_t m_vPlayers;
	//array<shared_ptr<BasePlayer>, PLAYER_COUNT> m_vPlayers;
	int m_vPlayerScores[4];
	Wall m_oWall;
	//vector<Move> m_vAction;
	std::pair<Move, int> m_oLastMove;//To get info of target tile: use second; To other info (main player, move type), use first.
	int m_iDealer;											// [1, PLAYER]
	int m_iTableWind;									// [1, PLAYER]
	int m_iRound;											// [1, PLAYER]

	SGF m_oSgf;

	//collect some game data
	array<ofstream, PLAYER_COUNT> m_fouts;

	static string g_sLogDirName;
	static string g_sScoreFileName;
};

