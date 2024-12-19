#include "GameBridge.h"
#define NOMINMAX
#include "../MJLibrary/Base/Ini.h"

string GameBridge::g_sLogName = "";

GameBridge::GameBridge() : m_bUseSocket(false) 
{
	std::cerr << "[GameBridge::GameBridge] Do not use this constructor!!" << std::endl;
	assert(0);
}

GameBridge::GameBridge(GamePlayer* pPlayer) : m_bUseSocket(false), m_pPlayer(pPlayer)
{

}

GameBridge::GameBridge(GamePlayer* pPlayer, const string& sIp, const int& iPort) : m_bUseSocket(true), m_pPlayer(pPlayer)
{
	m_oSocket.initSocket(sIp, iPort);
}

void GameBridge::setStartGame(const Command & oCommand)
{
	m_pPlayer->m_iSeat = toInt(oCommand[2]);
	g_sLogName = getTime();

	//log commands
	Ini &oIni = Ini::getInstance();
	m_bLogCommand = oIni.getIntIni("Log.LogGameCommand") > 0;
	string sLogDir = oIni.getStringIni("Log.LogPath");
	if (m_bLogCommand) {
		clearFile(getCurrentPath() + sLogDir + "\\" + g_sLogName + "_" + oCommand[2] + ".txt");
		m_fLog.open(getCurrentPath() + sLogDir + "\\" + g_sLogName + "_" + oCommand[2] + ".txt", std::ios::out | std::ios::app);
	}

	//log sgf
	m_bLogSgf = oIni.getIntIni("Sgf.LogGameSgf");
	if (m_bLogSgf) {
		m_oSgf = SGF(g_sLogName + "_" + oCommand[2], Type_GameSgf);
		m_oSgf.CreateNewSGF();
		m_oSgf.addRootToSgf();
	}
}

void GameBridge::setInitHandTile(const Command & oCommand)
{
	for (int i = 1; i < oCommand.size(); ++i) { 
		addTile(oCommand[i]); 
		//log sgf
		if(m_bLogSgf)
			m_oSgf.addMoveToSgf(m_pPlayer->m_iSeat, "M" + oCommand[i]);
	}
	//log sgf
	m_bPreviousMoveIsMo = m_pPlayer->m_oPlayerTile.getHandTileNumber() == 17;
}

vector<string> GameBridge::getCommand() const
{
	string sCommand;
	if (m_bUseSocket) {
		//std::cerr << "[GameBridge::getCommand] Try to recvData..." << std::endl;
		m_oSocket.recvData(sCommand);
		//std::cout << "[ << ] " << sCommand;
	} else {
		char vSrcCmd[256];
		cin.getline(vSrcCmd, 256);
		sCommand = string(vSrcCmd);
	}
	std::cerr << "[ << ] " << sCommand << std::endl;

	if (sCommand.back() == '\n') { sCommand.pop_back(); }
	if (sCommand.back() == '\r') { sCommand.pop_back(); }
	vector<string> vCommand = splitWith(sCommand, ' ');

	return vCommand;
}

void GameBridge::sendCommand(string sCommand) const
{
	if (sCommand.length() > 0) {
		if (sCommand.back() != '\n') { sCommand += '\n'; }

		if (m_bUseSocket) {
			HANDLE hConsole;
			hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, 12);

			//std::cerr << "[GameBridge::getCommand] Try to sendData: " << sCommand << std::endl;
			m_oSocket << sCommand;
			//std::cout << "[ >> ] " << sCommand;
			//cout.flush();

			SetConsoleTextAttribute(hConsole, 15);
		} else {
			std::cout << sCommand;
			cout.flush();
		}
		std::cerr << "[ >> ] " << sCommand;
		cerr.flush();
	}
}

bool GameBridge::handleCommand(const Command& oCommand)
{
	if (oCommand.empty())
		return true;
	string sInstruction = oCommand[0];
	bool bGameContinue = true;

	if (sInstruction == "")					{ return true; }
	else if (sInstruction == "/start")		{ setStartGame(oCommand); }
	//else if (sInstruction == "/setLeftTileCount") { setLeftTileCount(oCommand); }
	else if (sInstruction == "/initGame")	{ setInitGame(oCommand);}
	else if (sInstruction == "/initCard")	{ setInitHandTile(oCommand); }
	else if (sInstruction == "/mo")			{ drawTile(oCommand); }
	else if (sInstruction == "/ask")		{ ask(oCommand); }
	else if (sInstruction == "/throw")		{ throwTile(oCommand); }
	else if (sInstruction == "/eat")		{ chow(oCommand); }
	else if (sInstruction == "/pong")		{ pong(oCommand); }
	else if (sInstruction == "/gong")		{ kong(oCommand); }
	else if (sInstruction == "/hu")			{ win(oCommand); }
	else if (sInstruction == "/exit")		{
		//m_pPlayer->m_oTfCommunication.sendString("end game");
		//m_pPlayer->m_oTfCommunication.receiveLine(); 
		bGameContinue = false;
		if(m_bLogSgf)
			m_oSgf.finish();
	}
	else if (sInstruction == "/close")		{ bGameContinue = false; }
	else if (sInstruction == "/ready") { sendCommand("/ready"); }//for ntpu server

	//write to log
	if (m_bLogCommand && m_fLog.is_open()) {
		for (int i = 0; i < oCommand.size(); i++) {
			if (i > 0)
				m_fLog << " ";
			m_fLog << oCommand.at(i);
		}
		m_fLog << std::endl;
		if (!bGameContinue)
			m_fLog.close();
	}

	return bGameContinue;
}

void GameBridge::setInitGame(const Command& oCommand)
{
	m_pPlayer->m_oPlayerTile.clear();
	m_pPlayer->m_oRemainTile.init(RemainTileType::Type_Playing);
	m_oLastThrowTile = Tile();
	m_iLastThrowSeat = -1;
	m_vSourceHandTile.clear();
	m_bPreviousActionIsThrow = true;

	//log sgf
	int iDealer = atoi(oCommand.at(4).c_str());
	if(m_bLogSgf)
		m_oSgf.addTag("DEALER", SGF::getWindString(iDealer));
}

void GameBridge::throwTile(const Command& oCommand)
{
	m_oLastThrowTile = toTile(oCommand[2]);
	m_iLastThrowSeat = toInt(oCommand[1]);
	if (m_iLastThrowSeat == m_pPlayer->m_iSeat) {
		m_pPlayer->throwTile(m_oLastThrowTile);
		popTile(oCommand[2]);
	} else {
		m_pPlayer->m_oRemainTile.popTile(m_oLastThrowTile, 1, false);
		if (m_bPreviousActionIsThrow) { m_pPlayer->m_oRemainTile.emptyPopTile(); }
	}
	m_bPreviousActionIsThrow = true;

	//log sgf
	if(m_bLogSgf)
		m_oSgf.addMoveToSgf(m_iLastThrowSeat, "D" + oCommand[2]);
	m_bPreviousMoveIsMo = false;

	CERR(FormatString("RemainTile:\n%s\n", m_pPlayer->m_oRemainTile.getReadableString().c_str()));
	CERR(FormatString("RemainTile.remainDrawCount = %d\n", m_pPlayer->m_oRemainTile.getRemainDrawNumber()));
}

void GameBridge::ask(Command oCommand)
{
	string sResponse = "";
	if (oCommand[1] == "throw")		{ sResponse = askThrow(); }
	else if (oCommand[1] == "eat")	{ sResponse = askChow(); }
	else if (oCommand[1] == "pong")	{ sResponse = askPong(); }
	else if (oCommand[1] == "gong")	{ sResponse = askKong(); }
	else if (oCommand[1] == "hu")	{ sResponse = askWin(); }
	sendCommand(sResponse);
	m_pPlayer->reset();
}

void GameBridge::chow(const Command& oCommand)
{
	int iSeat = toInt(oCommand[1]);
	Tile oChowTile = toTile(oCommand[3]);
	vector<Tile> vRelatedTile = { toTile(oCommand[2]), toTile(oCommand[4]) };

	if (iSeat == m_pPlayer->m_iSeat) {
		m_pPlayer->m_oPlayerTile.eat({ oChowTile, vRelatedTile[0], vRelatedTile[1] });
		popTile(oCommand[2]);
		popTile(oCommand[4]);
	} else {
		m_pPlayer->m_oRemainTile.popTile(vRelatedTile[0], 1, false);
		m_pPlayer->m_oRemainTile.popTile(vRelatedTile[1], 1, false);
		m_bPreviousActionIsThrow = false;
	}

	//log sgf
	string sEatType;
	if (oChowTile > vRelatedTile[0] && oChowTile > vRelatedTile[1]) {//Eat right
		sEatType = "ER";
	}
	else if (oChowTile < vRelatedTile[0] && oChowTile < vRelatedTile[1]) {//Eat left
		sEatType = "EL";
	}
	else {//Eat middle
		sEatType = "EM";
	}
	if (m_bLogSgf)
		m_oSgf.addMoveToSgf(iSeat, sEatType + oCommand[3]);
	m_bPreviousMoveIsMo = false;
}

void GameBridge::pong(const Command& oCommand)
{
	int iSeat = toInt(oCommand[1]);
	Tile oPongTile = toTile(oCommand[3]);

	if (iSeat == m_pPlayer->m_iSeat) {
		m_pPlayer->m_oPlayerTile.pong(oPongTile);
		popTile(oCommand[2]);
		popTile(oCommand[3]);
		popTile(oCommand[4]);
	} else {
		m_pPlayer->m_oRemainTile.popTile(oPongTile, 2, false);
		m_bPreviousActionIsThrow = false;
	}

	//log sgf
	if (m_bLogSgf)
		m_oSgf.addMoveToSgf(iSeat, "P" + oCommand[3]);
	m_bPreviousMoveIsMo = false;
}

void GameBridge::kong(const Command& oCommand)
{
	int iSeat = toInt(oCommand[1]);
	int iKongType = toInt(oCommand[2]);
	m_bPreviousActionIsThrow = false;

	if (iKongType == 1) {
		Tile oKongTile = toTile(oCommand[3]);
		if (iSeat == m_pPlayer->m_iSeat) {
			m_pPlayer->m_oPlayerTile.upgradeKong(oKongTile);
			popTile(oCommand[3]);
		} else {
			m_pPlayer->m_oRemainTile.popTile(oKongTile, 1, false);
			m_pPlayer->m_oRemainTile.emptyPopTile();
		}
	} else if (iKongType == 4) {
		Tile oKongTile = toTile(oCommand[3]);
		if (iSeat == m_pPlayer->m_iSeat) {
			m_pPlayer->m_oPlayerTile.kong(oKongTile);
			popTile(oCommand[3]);
			popTile(oCommand[4]);
			popTile(oCommand[5]);
			popTile(oCommand[6]);
		} else {
			m_pPlayer->m_oRemainTile.popTile(oKongTile, 3, false);
			m_pPlayer->m_oRemainTile.emptyPopTile();
		}
	} else if (iKongType == 0) {
		if (iSeat == m_pPlayer->m_iSeat) {
			Tile oKongTile = toTile(oCommand[3]);
			m_pPlayer->m_oPlayerTile.darkKong(oKongTile);
			popTile(oCommand[3]);
			popTile(oCommand[4]);
			popTile(oCommand[5]);
			popTile(oCommand[6]);
		}
		else {
			m_pPlayer->m_oRemainTile.emptyPopTile();
		}
	}

	//log sgf
	string sKongType;
	if (iKongType == 1) {
		sKongType = "UG";
	}
	else if (iKongType == 0) {
		sKongType = "HG";
	}
	else if (iKongType == 4) {
		sKongType = "G";
	}

	if (m_bLogSgf) {
		if (iKongType == 0 && iSeat != m_pPlayer->m_iSeat)//other player darkkong
			m_oSgf.addMoveToSgf(iSeat, sKongType);//format correct?
		else
			m_oSgf.addMoveToSgf(iSeat, sKongType + oCommand[3]);
	}
	
	m_bPreviousMoveIsMo = false;
}

string GameBridge::askThrow()
{
	//std::cout << m_pPlayer->m_oPlayerTile.toString() << std::endl;
	Tile oTile = m_pPlayer->askThrow();
	return "/throw " + std::to_string(getTile(oTile));
}

string GameBridge::askChow()
{
	//[Probably bug] using getAndPopTile() will pop the tile immediately.
	//If the chow is not applied by server, m_vSourceHandTile will be incorrect. 
	MoveType oChow = m_pPlayer->askEat(m_oLastThrowTile);
	if (oChow == MoveType::Move_EatRight)
		return "/eat " + std::to_string(getAndPopTile(m_oLastThrowTile - 1)) + " " + std::to_string(getAndPopTile(m_oLastThrowTile - 2));
	else if (oChow == MoveType::Move_EatMiddle)
		return "/eat " + std::to_string(getAndPopTile(m_oLastThrowTile - 1)) + " " + std::to_string(getAndPopTile(m_oLastThrowTile + 1));
	else if (oChow == MoveType::Move_EatLeft)
		return "/eat " + std::to_string(getAndPopTile(m_oLastThrowTile + 1)) + " " + std::to_string(getAndPopTile(m_oLastThrowTile + 2));
	else
		return "/pass";
}

string GameBridge::askPong()
{
	if (m_pPlayer->askPong(m_oLastThrowTile, canChow(), canKong()))
		return "/pong " + std::to_string(getAndPopTile(m_oLastThrowTile)) + " " + std::to_string(getAndPopTile(m_oLastThrowTile));
	else
		return "/pass";
}

string GameBridge::askKong()
{
	if (m_vSourceHandTile.size() % 3 == 2) {
		std::pair<MoveType, Tile> oKong = m_pPlayer->askDarkKongOrUpgradeKong();
		if (oKong.first == MoveType::Move_DarkKong) {
			return "/gong 0 " +
				std::to_string(getAndPopTile(oKong.second)) + " " +
				std::to_string(getAndPopTile(oKong.second)) + " " +
				std::to_string(getAndPopTile(oKong.second)) + " " +
				std::to_string(getAndPopTile(oKong.second));
		} else if (oKong.first == MoveType::Move_UpgradeKong) {
			return "/gong 1 " + std::to_string(getTile(oKong.second));
		}
	} else {
		if (m_pPlayer->askKong(m_oLastThrowTile)) {
			return "/gong 4 " +
				std::to_string(getAndPopTile(m_oLastThrowTile)) + " " +
				std::to_string(getAndPopTile(m_oLastThrowTile)) + " " +
				std::to_string(getAndPopTile(m_oLastThrowTile));
		}
	}
	return "/pass";
}

///private

int GameBridge::getAndPopTile(const Tile& oTile)
{
	for (int i = 0; i < m_vSourceHandTile.size(); ++i) {
		if (oTile == toTile(m_vSourceHandTile[i])) {
			int iTile = m_vSourceHandTile[i];
			m_vSourceHandTile.erase(m_vSourceHandTile.begin() + i);
			return iTile;
		}
	}
}

int GameBridge::getTile(const Tile& oTile)
{
	for (int i = 0; i < m_vSourceHandTile.size(); ++i) {
		if (oTile == toTile(m_vSourceHandTile[i]))
			return m_vSourceHandTile[i];
	}
}

void GameBridge::popTile(const int& iTile)
{
	for (int i = 0; i < m_vSourceHandTile.size(); ++i) {
		if (iTile == m_vSourceHandTile[i]) {
			m_vSourceHandTile.erase(m_vSourceHandTile.begin() + i);
			break;
		}
	}
}

void GameBridge::addTile(const int& iTile)
{
	m_vSourceHandTile.push_back(iTile);
	m_pPlayer->drawTile(toTile(iTile));
}

Tile GameBridge::toTile(const int& iTile) const
{
	//[Not gerenal] Cannot handle if max rank > 9
	int iType = iTile / PROTOCOL_SYMBOL_PLACE - 1;
	int iRank = (iTile / (PROTOCOL_SYMBOL_PLACE / PROTOCOL_NUMBER_PLACE)) % PROTOCOL_NUMBER_PLACE;
	return Tile(iType, iRank);
}