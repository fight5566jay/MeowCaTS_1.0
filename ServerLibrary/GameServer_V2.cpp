#include "GameServer_V2.h"
#include <fstream>
string GameServer_V2::g_sLogDirName = "Server_Log";
string GameServer_V2::g_sScoreFileName = "resultScore.txt";

GameServer_V2::GameServer_V2()
{
}


GameServer_V2::~GameServer_V2()
{
}

void GameServer_V2::init(PlayerList_t& vPlayers)
{
	//init member variables
	m_iDealer = 1;
	m_iTableWind = 1;
	m_iRound = 1;

	//init player id & score
	for (int i = 0; i < m_vPlayers.size(); i++) {
		m_vPlayers[i] = std::move(vPlayers[i]);
		m_vPlayers[i]->m_iPlayerId = i;
		m_vPlayerScores[i] = 0;
	}

	//init score file
	std::string sDirName = getCurrentPath() + g_sLogDirName;
	if (!isDirExists(sDirName)) {
		createFolder(sDirName);
	}
	ofstream fScoreFile(sDirName + "/" + g_sScoreFileName);
	for (int i = 0; i < PLAYER_COUNT; i++)
		fScoreFile << "S" << i + 1 << "\t";
	for (int i = 0; i < PLAYER_COUNT - 1; i++)
		fScoreFile << "TS" << i + 1 << "\t";
	fScoreFile << "TS" << PLAYER_COUNT << std::endl;
}

void GameServer_V2::start(const Wall & oWall, const bool & bRandomParam)
{
	m_oWall = oWall;
	if (bRandomParam)
		_initParameter();

	m_sLog = std::to_string(m_iDealer) + " " + std::to_string(m_iTableWind) + " " + std::to_string(m_iRound);


	_initSeat();
	_initGame();
	_initHandTile();
	//m_oWall.setTileCountUsedInGame(124);//60 + 16 * 4, only for experiment
	_startGame();
}

void GameServer_V2::printState() const
{
#ifdef WINDOWS
	system("cls");
#elif defined(LINUX)
	system("clear");
#endif
	std::vector<Tile> vThrownTiles;
	for (int i = 0; i < m_vPlayers.size(); ++i) {
		std::cout << i << " : ";
		if ((m_oLastMove.first.getMainPlayer() != i) || (m_oLastMove.first.getMoveType() == MoveType::Move_Init)) {
			std::cout << m_vPlayers[i]->getPlayerTile().toString() << " [MinLack: " << m_vPlayers[i]->getPlayerTile().getMinLack() << "]" << std::endl;
			std::cout << " >>";
			vThrownTiles = m_vPlayers[i]->getThrownTiles();
			for (int i = 0; i < vThrownTiles.size(); ++i)
				std::cout << " " << vThrownTiles[i].toString();
			std::cout << std::endl;
		}
		else {
			PlayerTile oPlayerTile;
			Tile oDrawTile;

			switch (m_oLastMove.first.getMoveType()) {
			case MoveType::Move_Draw:
				oPlayerTile = m_vPlayers[i]->getPlayerTile();
				oDrawTile = Tile::sgfIdToTile(m_oLastMove.second);
				oPlayerTile.popTileFromHandTile(oDrawTile);
				std::cout << oPlayerTile.toString() << " [" << oDrawTile.toString() << "] [MinLack: " << m_vPlayers[i]->getPlayerTile().getMinLack() << "]" << std::endl;
				std::cout << " >>";
				vThrownTiles = m_vPlayers[i]->getThrownTiles();
				for (int i = 0; i < vThrownTiles.size(); ++i)
					std::cout << " " << vThrownTiles[i].toString();
				std::cout << std::endl;
				break;
			case MoveType::Move_Throw:
				std::cout << m_vPlayers[i]->getPlayerTile().toString() << " [MinLack: " << m_vPlayers[i]->getPlayerTile().getMinLack() << "]" << std::endl;
				std::cout << " >>";
				vThrownTiles = m_vPlayers[i]->getThrownTiles();
				for (int i = 0; i < vThrownTiles.size() - 1; ++i)
					std::cout << " " << vThrownTiles[i].toString();
				std::cout << " [" << vThrownTiles.back().toString() << "]" << std::endl;
				break;
			}
		}
	}
}

void GameServer_V2::switchSeat(const array<int, PLAYER_COUNT>& v_PlayerIds)
{
	array<unique_ptr<Player_t>, PLAYER_COUNT> pPlayers;
	for (int i = 0; i < PLAYER_COUNT; i++) {
#ifdef MJ_DEBUG_MODE
		assert(v_PlayerIds.at(i) >= 1 && v_PlayerIds.at(i) <= PLAYER_COUNT);
#endif
		pPlayers[i] = std::move(m_vPlayers[i]);
	}

	for (int i = 0; i < m_vPlayers.size(); i++) {
		m_vPlayers[i] = std::move(pPlayers[v_PlayerIds.at(i) - 1]);
	}
}

void GameServer_V2::nextGame()
{
}

void GameServer_V2::_initSeat()
{
	for (int i = 0; i < PLAYER_COUNT; ++i) {
		m_vPlayers[i]->initSeat(i + 1);
	}
}

void GameServer_V2::_initGame()
{
	for (int i = 0; i < PLAYER_COUNT; ++i)
		m_vPlayers[i]->initGame(m_iTableWind, m_iRound, m_iDealer);

	//sgf
	//m_oSgf = SGF(g_sLogDirName + "/MJServerLog_" + getTime() + ".sgf", Type_GameSgf);
	m_oSgf = SGF(g_sLogDirName + "/MJServerLog_" + getTime(), Type_GameSgf);
	m_oSgf.CreateNewSGF();
	m_oSgf.addRootToSgf();
	const string sWind[5] = { "UnknownWind", "E", "S", "W", "N" };
	m_oSgf.addTag("DEALER", sWind[m_iDealer]);
	m_oSgf.addTag(m_oWall.getSgfString());
}

void GameServer_V2::_initHandTile()
{
	vector<int> vHandTile(MAX_HANDTILE_COUNT - 1);
	vHandTile.reserve(MAX_HANDTILE_COUNT);

	for (int i = 0; i < PLAYER_COUNT; ++i) {
		int iCurrentPlayer = (m_iDealer - 1 + i) % PLAYER_COUNT;
		for (int j = 0; j < MAX_HANDTILE_COUNT - 1; ++j) {
			vHandTile[j] = m_oWall.getTileFromFront();
			m_sLog += " " + Tile::sgfIdToTile(vHandTile[j]).toString();
		}

		if (iCurrentPlayer == m_iDealer - 1) {
			//dealer
			vHandTile.push_back(m_oWall.getTileFromFront());
			m_sLog += " " + Tile::sgfIdToTile(vHandTile[MAX_HANDTILE_COUNT - 1]).toString();
			m_vPlayers[iCurrentPlayer]->initHandTile(vHandTile);
			vHandTile.pop_back();
		}
		else {
			m_vPlayers[iCurrentPlayer]->initHandTile(vHandTile);
		}
	}
}

void GameServer_V2::_startGame()
{
	array<int, PLAYER_COUNT> vGameScore = { 0, 0, 0, 0 };
	m_oLastMove = std::pair<Move, int>(Move(MoveType::Move_Init), -1);
	int iNowPlayer = m_iDealer - 1;
	int iThrownTile;

	time_t startTime, thinkingTimeMs;
	int iPlayer;
	printState();
	bool bIsSomeoneWin = false, bIsSomeoneMeld = false, bIsSomeoneKong = false;
	const int ciPlayerCount = m_vPlayers.size();
	while (true) {
		//set
		m_vPlayers[iNowPlayer]->setLeftTileCount(m_oWall.getLeftDrawCount());//only for experiment

		if (bIsSomeoneKong || !bIsSomeoneMeld) {
			if (m_vPlayers[iNowPlayer]->canWin() && m_vPlayers[iNowPlayer]->askWin()) {
				//self-drawn
				for (int i = 0; i < PLAYER_COUNT; i++) {
					m_vPlayers[i]->winBySelfDraw(iNowPlayer + 1, m_oLastMove.second, m_vPlayers[iNowPlayer]->getHandIdTile());
				}
				m_sLog += " " + std::to_string(iNowPlayer + 1) + "W" + Tile::sgfIdToTile(m_oLastMove.second).toString();
				m_oSgf.addMoveToSgf(iNowPlayer + 1, "SM" + std::to_string(m_oLastMove.second));
				vGameScore = { -1, -1, -1, -1 };
				vGameScore[m_vPlayers[iNowPlayer]->m_iPlayerId] = 3;
				//writeTrajectoriesToFile(vGameScore);
				break;
			}
			if (m_oWall.isEnd()) {
				//draw game
				break;
			}
			if (m_vPlayers[iNowPlayer]->canDarkKongOrUpgradeKong()) {
				MoveType oMoveType;
				int iTargetTile;

				//ask dark kong or upgrade kong
				auto oResponds = m_vPlayers[iNowPlayer]->askDarkKongOrUpgradeKong();
				oMoveType = oResponds.first;
				iTargetTile = oResponds.second;
				//[Debug]
				if (oMoveType != MoveType::Move_Pass && iTargetTile == -1) {
					//error
					std::cerr << "[GameServer_V2::_startGame] Illegal move: " << Move(oMoveType, Tile::sgfIdToTile(iTargetTile)).toString() << std::endl;
					assert(oMoveType == MoveType::Move_Pass || iTargetTile > 0);
				}
				
				
				switch (oMoveType) {
				case MoveType::Move_DarkKong:
					//dark kong
					m_vPlayers[iNowPlayer]->darkKong(iTargetTile);
					for (int i = 0; i < ciPlayerCount; i++) {
						if (i != iNowPlayer) {
							m_vPlayers[i]->darkKong_otherPlayer(iNowPlayer + 1);
						}
					}
					m_sLog += " " + std::to_string(iNowPlayer + 1) + "K" + Tile::sgfIdToTile(iTargetTile).toString();
					m_oSgf.addMoveToSgf(iNowPlayer + 1, "HG" + std::to_string(iTargetTile));
					_drawTile(iNowPlayer, false);
					bIsSomeoneMeld = false;
					//bIsSomeoneKong = true;//not necessary
					printState();
					break;
				case MoveType::Move_UpgradeKong:
					//upgrade kong
					for (int i = 0; i < ciPlayerCount; i++) {
						m_vPlayers[i]->upgradeKong(iNowPlayer + 1, iTargetTile);
					}
					m_sLog += " " + std::to_string(iNowPlayer + 1) + "u" + Tile::sgfIdToTile(iTargetTile).toString();
					m_oSgf.addMoveToSgf(iNowPlayer + 1, "UG" + std::to_string(iTargetTile));
					_drawTile(iNowPlayer, false);
					bIsSomeoneMeld = false;
					//bIsSomeoneKong = true;//not necessary
					printState();
					break;
				case MoveType::Move_Pass:
					break;
				default:
					std::cerr << "[GameServer_V2::_startGame] Illegal move: " << Move(oMoveType, Tile::sgfIdToTile(iTargetTile)).toString() << std::endl;
					assert(oMoveType == MoveType::Move_DarkKong || oMoveType == MoveType::Move_UpgradeKong || oMoveType == MoveType::Move_Pass);
					break;
				}

				if(oMoveType != MoveType::Move_Pass)
					continue;
			}
		}
		
		//ask throw
		iThrownTile = m_vPlayers[iNowPlayer]->askThrow();

		//throw tile
		for (int i = 0; i < ciPlayerCount; i++) {
			m_vPlayers[i]->throwTile(iNowPlayer + 1, iThrownTile, bIsSomeoneKong || !bIsSomeoneMeld);
		}
		m_oLastMove = std::pair<Move, int>(Move(MoveType::Move_Throw, Tile::sgfIdToTile(iThrownTile), iNowPlayer, -1), iThrownTile);
		m_sLog += " " + std::to_string(iNowPlayer + 1) + "d" + Tile::sgfIdToTile(iThrownTile).toString();
		m_oSgf.addMoveToSgf(iNowPlayer + 1, "D" + std::to_string(iThrownTile));
		printState();
		//printThinkingTimeToFile(iNowPlayer, thinkingTimeMs);//experiment

		//check other players win
		bIsSomeoneWin = false;
		iPlayer = _getNextPlayerIndex(iNowPlayer);
		for (int i = 0; i < PLAYER_COUNT - 1; ++i) {
			if (m_vPlayers[iPlayer]->canWin(iThrownTile) && m_vPlayers[iPlayer]->askWin(iThrownTile)) {
				//iPlayer win
				for (int i = 0; i < PLAYER_COUNT; i++) {
					m_vPlayers[i]->winByChunk(iPlayer + 1, m_oLastMove.second, iNowPlayer + 1, m_vPlayers[iPlayer]->getHandIdTile());
				}
				m_sLog += " " + std::to_string(iPlayer + 1) + "w" + Tile::sgfIdToTile(iThrownTile).toString();
				m_oSgf.addMoveToSgf(iPlayer + 1, "H" + std::to_string(iThrownTile));
				vGameScore[m_vPlayers[iPlayer]->m_iPlayerId] = 1;
				vGameScore[m_vPlayers[iNowPlayer]->m_iPlayerId] = -1;
				//writeTrajectoriesToFile(vGameScore);
				bIsSomeoneWin = true;
				break;
			}
			iPlayer = _getNextPlayerIndex(iPlayer);
		}
		if (bIsSomeoneWin)
			break;

		//check other players pong or kong (first next player cannot kong)
		bIsSomeoneMeld = bIsSomeoneKong = false;
		iPlayer = _getNextPlayerIndex(iNowPlayer);
		for (int i = 0; i < PLAYER_COUNT - 1; ++i) {
			if (i != 0 && m_vPlayers[iPlayer]->canKong(iThrownTile) && m_vPlayers[iPlayer]->askKong(iThrownTile)) {
				
				//gen related tiles
				vector<int> vKongTiles(SAME_TILE_COUNT);
				int iKongTile = iThrownTile / PROTOCOL_NUMBER_PLACE * PROTOCOL_NUMBER_PLACE;
				for (int j = 0; j < SAME_TILE_COUNT; j++) {
					vKongTiles[j] = iKongTile + j;
				}

				//iPlayer kong
				for (int j = 0; j < ciPlayerCount; j++) {
					m_vPlayers[j]->kong(iPlayer + 1, vKongTiles);
				}
				m_sLog += " " + std::to_string(iPlayer + 1) + "k" + Tile::sgfIdToTile(iThrownTile).toString();
				m_oSgf.addMoveToSgf(iPlayer + 1, "G" + std::to_string(iThrownTile));
				_drawTile(iPlayer, false);
				iNowPlayer = iPlayer;
				bIsSomeoneMeld = true;
				bIsSomeoneKong = true;
				break;
			}
			if (m_vPlayers[iPlayer]->canPong(iThrownTile)) {
				vector<int> vTiles = m_vPlayers[iPlayer]->askPong(iThrownTile, i == 0);
				if (vTiles[0] != -1) {//pong
					//gen related tiles
					for (int j = 0; j < vTiles.size(); j++) {
						if (iThrownTile < vTiles[j]) {
							vTiles.insert(vTiles.begin() + j, iThrownTile);
							break;
						}
					}
					if (vTiles.size() == 2) {
						assert(iThrownTile > vTiles[0] && iThrownTile > vTiles[1]);
						vTiles.push_back(iThrownTile);
					}

					//iPlayer pong
					for (int i = 0; i < ciPlayerCount; i++) {
						m_vPlayers[i]->pong(iPlayer + 1, vTiles);
					}
					m_sLog += " " + std::to_string(iPlayer + 1) + "n" + Tile::sgfIdToTile(iThrownTile).toString();
					m_oSgf.addMoveToSgf(iPlayer + 1, "P" + std::to_string(iThrownTile));
					iNowPlayer = iPlayer;
					bIsSomeoneMeld = true;
					break;
				}
			}
			iPlayer = _getNextPlayerIndex(iPlayer);
		}
		if (bIsSomeoneMeld)
			continue;

		//check if next player eat
		iPlayer = _getNextPlayerIndex(iNowPlayer);
		if (m_vPlayers[iPlayer]->canChow(iThrownTile)) {
			vector<int> vTiles = m_vPlayers[iPlayer]->askChow(iThrownTile);
			if (vTiles[0] == -1) {
				//pass
				//do nothing...
			}
			else {
				//eat
				//gen releated tiles
				//put target tile at the middle of vTiles
				vTiles.insert(vTiles.begin() + 1, iThrownTile);

				if (iThrownTile < vTiles[0] && iThrownTile < vTiles[2]) {
					//eat left
					for (int i = 0; i < ciPlayerCount; i++) {
						m_vPlayers[i]->eatLeft(iPlayer + 1, vTiles);
					}
					m_oSgf.addMoveToSgf(iPlayer + 1, "EL" + std::to_string(iThrownTile));
				}
				else if (iThrownTile > vTiles[0] && iThrownTile > vTiles[2]) {
					//eat right
					for (int i = 0; i < ciPlayerCount; i++) {
						m_vPlayers[i]->eatRight(iPlayer + 1, vTiles);
					}
					m_oSgf.addMoveToSgf(iPlayer + 1, "ER" + std::to_string(iThrownTile));
					
				}
				else {
					//eat middle
					for (int i = 0; i < ciPlayerCount; i++) {
						m_vPlayers[i]->eatMiddle(iPlayer + 1, vTiles);
					}
					m_oSgf.addMoveToSgf(iPlayer + 1, "EM" + std::to_string(iThrownTile));
				}

				m_sLog += " " + std::to_string(iPlayer + 1) + "c";
				m_sLog += Tile::sgfIdToTile(vTiles[1]).toString();
				m_sLog += Tile::sgfIdToTile(vTiles[0]).toString();
				m_sLog += Tile::sgfIdToTile(vTiles[2]).toString();
				iNowPlayer = iPlayer;
				bIsSomeoneMeld = true;
			}
			
			if (bIsSomeoneMeld)
				continue;
		}

		//Nobody meld
		iNowPlayer = _getNextPlayerIndex(iNowPlayer);
		_drawTile(iNowPlayer, true);
		printState();
		//bIsSomeoneMeld = bIsSomeoneKong = false;
	}

	//Player_old::broadcast("/exit 0 0 0 0");
	_updateScore(vGameScore);
	//writeTrajectoriesToFile(vGameScore);
	//printThinkingTimeToFile_GameEnd();
	m_oSgf.finish();

	for (int i = 0; i < m_vPlayers.size(); i++) {
		m_vPlayers[i]->endGame();
	}

	//system("pause");
}

void GameServer_V2::_drawTile(const int & iDrawPlayer, const bool & bDrawFront)
{
	int iDrawTile = bDrawFront ? m_oWall.getTileFromFront() : m_oWall.getTileFromRear();
	m_vPlayers[iDrawPlayer]->drawTile(iDrawTile);
	m_oLastMove = std::pair<Move, int>(Move(MoveType::Move_Draw, Tile::sgfIdToTile(iDrawTile), iDrawPlayer, -1), iDrawTile);
	m_sLog += " " + std::to_string(iDrawPlayer + 1) + "g" + Tile::sgfIdToTile(iDrawTile).toString();
	m_oSgf.addMoveToSgf(iDrawPlayer + 1, "M" + std::to_string(iDrawTile));
	//printState();
}

int GameServer_V2::_getNextPlayerIndex(const int & iCurrentPlayer)
{
	if (iCurrentPlayer == PLAYER_COUNT - 1)
		return 0;
	return iCurrentPlayer + 1;
}

//int GameServer_V2::_getPreviousPlayerIndex(const int & iCurrentPlayer)
//{
//	if (iCurrentPlayer == 0)
//		return PLAYER_COUNT - 1;
//	return iCurrentPlayer - 1;
//}

void GameServer_V2::_updateScore(const array<int, PLAYER_COUNT>& vGameScores)
{
	assert(vGameScores.size() == PLAYER_COUNT);

	for (int i = 0; i < PLAYER_COUNT; i++) {
		m_vPlayerScores[i] += vGameScores[i];
	}

	//output to file
	std::fstream fScoreFile(getCurrentPath() + g_sLogDirName + "/" + g_sScoreFileName, std::ios::out | std::ios::app);
	for (int i = 0; i < PLAYER_COUNT; i++) {
		fScoreFile << vGameScores[i] << "\t";
	}
	for (int i = 0; i < PLAYER_COUNT - 1; i++) {
		fScoreFile << m_vPlayerScores[i] << "\t";
	}
	fScoreFile << m_vPlayerScores[PLAYER_COUNT - 1] << std::endl;
	fScoreFile.close();
}

void GameServer_V2::_initParameter()
{
	m_iDealer = rand() % PLAYER_COUNT + 1;
	m_iTableWind = rand() % PLAYER_COUNT + 1;
	m_iRound = rand() % PLAYER_COUNT + 1;
	//shuffleSeat();//off for experiment
}
