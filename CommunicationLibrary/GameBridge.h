#pragma once
#include <iostream>
#include <vector>
#include "BasePlayer.h"
#include "../MJLibrary/Base/Tools.h"
#include "../MJLibrary/Base/ClientSocket.h"
#include "../MJLibrary/Base/SGF.h"

using std::vector;
using std::string;
using std::cin;
using std::cout;
using std::fstream;

typedef vector<string> Command;
typedef BasePlayer GamePlayer;

class GameBridge
{
public:
	static string g_sLogName;

public:
	GameBridge();
	GameBridge(GamePlayer* pPlayer);
	GameBridge(GamePlayer* pPlayer, const string& sIp, const int& iPort);
	
public:
	//void setLeftTileCount(const Command & oCommand);
	void setStartGame(const Command& oCommand);
	void setInitHandTile(const Command& oCommand);
	inline void drawTile(const Command& oCommand) { 
		addTile(oCommand[1]); 
		//log sgf
		m_oSgf.addMoveToSgf(m_pPlayer->m_iSeat, "M" + oCommand[1]);
		m_bPreviousMoveIsMo = true;
	};
	inline void win(const Command& oCommand) {
		if (toInt(oCommand[1]) == m_pPlayer->m_iSeat && !m_bPreviousMoveIsMo) {
			m_pPlayer->m_oPlayerTile.putTileToHandTile(toTile(oCommand[2]));
		}

		//log sgf
		string sWinType = m_bPreviousMoveIsMo? "SM" : "H";
		m_oSgf.addMoveToSgf(toInt(oCommand[1]), sWinType + oCommand[2]);
	};
	inline string askWin() { return "/hu"; };

	vector<string> getCommand() const;
	void sendCommand(string sCommand) const;
	bool handleCommand(const Command& oCommand);
	void setInitGame(const Command& oCommand);
	
	void throwTile(const Command& oCommand);
	void ask(Command oCommand);
	void chow(const Command& oCommand);
	void pong(const Command& oCommand);
	void kong(const Command& oCommand);
	string askThrow();
	string askChow();
	string askPong();
	string askKong();

private:
	inline bool canChow() { return(m_pPlayer->m_iSeat - m_iLastThrowSeat) % PLAYER_COUNT == 1; };
	inline bool canKong() { return(m_pPlayer->m_iSeat - m_iLastThrowSeat) % PLAYER_COUNT != 1; };
	inline Tile toTile(const string& sTile) const { return toTile(toInt(sTile)); };
	inline void popTile(const string& sTile) { popTile(toInt(sTile)); };
	inline void addTile(const string& sTile) { addTile(toInt(sTile)); };

	int getAndPopTile(const Tile& oTile);
	int getTile(const Tile& oTile);
	void popTile(const int& iTile);
	void addTile(const int& iTile);
	Tile toTile(const int& iTile) const;

private:
	GamePlayer* m_pPlayer;
	Tile m_oLastThrowTile;
	int m_iLastThrowSeat;
	vector<int> m_vSourceHandTile;
	bool m_bPreviousActionIsThrow;
	ClientSocket m_oSocket;
	bool m_bUseSocket;

	//log
	bool m_bLogCommand;
	bool m_bLogSgf;
	fstream m_fLog;
	SGF m_oSgf;
	bool m_bPreviousMoveIsMo;
};