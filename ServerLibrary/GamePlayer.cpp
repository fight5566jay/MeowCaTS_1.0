#include "GamePlayer.h"
//#include "../DummyPlayer/DummyPlayer.h"
using std::cerr;
using std::endl;
string GamePlayer::g_sLogDirName = "Server_Log";

GamePlayer::GamePlayer()
{
	//m_pPlayer = std::make_unique<DummyPlayer>();
}

GamePlayer::GamePlayer(std::unique_ptr<BasePlayer>& pPlayer)
{
	m_pPlayer = std::move(pPlayer);
}


GamePlayer::~GamePlayer()
{
}

void GamePlayer::initSeat(const int & iSeat)
{
	m_iSeat = iSeat;

	//[LogCommand]
	string sCommandLogName(getCurrentPath() + g_sLogDirName + "/MJServerLog_" + getTime() + "_Player" + toStr(iSeat) + ".log");
	_startLogCommand(sCommandLogName);
	_logCommand("/start MJ " + std::to_string(iSeat) + " " + m_sName);
}

void GamePlayer::initGame(const int & iTableWind, const int & iRound, const int & iDealer)
{
	const static array<string, 5> vWind = { "Unknown",  "East", "South", "West", "North" };
	m_iTableWind = iTableWind;
	m_iRound = iRound;
	m_iDealer = iDealer;

	//[LogCommand]
	_logCommand({ "/initGame", vWind[iTableWind], vWind[iRound], std::to_string(iRound), std::to_string(iDealer), "0" });
}

void GamePlayer::initHandTile(const vector<int>& vHandTile)
{
	//reset
	m_vThrownTiles.clear();
	m_pPlayer->m_oPlayerTile.clear();
	m_pPlayer->m_oRemainTile.init(RemainTileType::Type_Playing);
	m_vHandIdTile.clear();

	//draw tiles
	const int sz = vHandTile.size();
	for (int i = 0; i < sz; ++i) {
		drawTile(vHandTile[i], true);
	}

	//[LogCommand]
	string sCommand = "/initCard";
	for (int i = 0; i < sz; ++i) {
		sCommand += " " + std::to_string(vHandTile[i]);
	}
	_logCommand(sCommand);
}

bool GamePlayer::askWin()
{
	_logCommand("/ask hu");
	bool bRespond = m_pPlayer->askWin();

	//[LogCommand]
	if (bRespond) {
		_logResponds("/hu");
	}
	else {
		_logResponds("/pass");
	}

	return bRespond;
}

bool GamePlayer::askWin(const int & iTile)
{
	_logCommand("/ask hu");
	bool bRespond = m_pPlayer->askWin(Tile::sgfIdToTile(iTile));

	//[LogCommand]
	if (bRespond) {
		_logResponds("/hu");
	}
	else {
		_logResponds("/pass");
	}

	return bRespond;
}

//return the id of tile that BasePlayer want to throw
int GamePlayer::askThrow()
{
	_logCommand("/ask throw");
	auto iThrownTile = _getTileIdFromHand(m_pPlayer->askThrow());

	//[LogCommand]
	_logResponds({"/throw", std::to_string(iThrownTile)});

	return iThrownTile;
}

vector<int> GamePlayer::askChow(const int & iTile)
{
	_logCommand("/ask eat");
	Tile oTargetTile = Tile::sgfIdToTile(iTile);
	auto oResponds = m_pPlayer->askEat(oTargetTile);

	vector<int> vRelatedTiles = { -1, -1 };
	switch (oResponds) {
	case MoveType::Move_EatLeft:
		vRelatedTiles[0] = _getTileIdFromHand(oTargetTile + 1);
		vRelatedTiles[1] = _getTileIdFromHand(oTargetTile + 2);
		_logResponds({"/eat", std::to_string(vRelatedTiles[0]), std::to_string(vRelatedTiles[1]) });
		break;
	case MoveType::Move_EatMiddle:
		vRelatedTiles[0] = _getTileIdFromHand(oTargetTile - 1);
		vRelatedTiles[1] = _getTileIdFromHand(oTargetTile + 1);
		_logResponds({ "/eat", std::to_string(vRelatedTiles[0]), std::to_string(vRelatedTiles[1]) });
		break;
	case MoveType::Move_EatRight:
		vRelatedTiles[0] = _getTileIdFromHand(oTargetTile - 2);
		vRelatedTiles[1] = _getTileIdFromHand(oTargetTile - 1);
		_logResponds({ "/eat", std::to_string(vRelatedTiles[0]), std::to_string(vRelatedTiles[1]) }); 
		break;
	case MoveType::Move_Pass:
		//vRelatedTiles[0] = vRelatedTiles[1] = -1;
		_logResponds("/pass");
		break;
	default:
		cerr << "[GamePlayer::askChow(int)] Illegal move type: " << Move(oResponds, Tile::sgfIdToTile(iTile)).toString() << endl;
		assert(0);
		break;
	}

	assert(vRelatedTiles.size() == 2);
	return vRelatedTiles;
}

vector<int> GamePlayer::askPong(const int & iTile, const bool& bCanChow)
{
	_logCommand("/ask pong");
	Tile oTargetTile = Tile::sgfIdToTile(iTile);
	bool bRespond = m_pPlayer->askPong(oTargetTile, bCanChow, false);

	//[LogCommand]
	vector<int> vRelatedTiles = { -1, -1 };
	if (bRespond) {
		vector<int> vTiles = _getTileIdsFromHand(oTargetTile, 2);
		assert(vTiles.size() == 2);
		vRelatedTiles[0] = vTiles[0];
		vRelatedTiles[1] = vTiles[1];
		_logResponds({ "/pong", std::to_string(vRelatedTiles[0]), std::to_string(vRelatedTiles[1]) });
	}
	else {
		_logResponds("/pass");
	}

	assert(vRelatedTiles.size() == 2);
	return vRelatedTiles;
}

bool GamePlayer::askKong(const int & iTile)
{
	_logCommand("/ask gong");
	Tile oTargetTile = Tile::sgfIdToTile(iTile);
	bool bRespond = m_pPlayer->askKong(Tile::sgfIdToTile(iTile));

	//[LogCommand]
	if (bRespond) {
		vector<int> vRelatedTile = _getTileIdsFromHand(oTargetTile, 3);
		assert(vRelatedTile.size() == 3);
		_logResponds({ "/gong", "4", std::to_string(vRelatedTile[0]),  std::to_string(vRelatedTile[1]),  std::to_string(vRelatedTile[2]) });
	}
	else {
		_logResponds("/pass");
	}

	return bRespond;
}

std::pair<MoveType, int> GamePlayer::askDarkKongOrUpgradeKong()
{
	_logCommand("/ask gong");
	auto oResponds = m_pPlayer->askDarkKongOrUpgradeKong();

	//[LogCommand]
	vector<int> vRelatedTile;
	switch (oResponds.first) {
	case MoveType::Move_DarkKong:
		vRelatedTile = _getTileIdsFromHand(oResponds.second, 4);
		assert(vRelatedTile.size() == 4);
		_logResponds({ "/gong", "0", std::to_string(vRelatedTile[0]), std::to_string(vRelatedTile[1]), std::to_string(vRelatedTile[2]), std::to_string(vRelatedTile[3]) });
		return std::pair<MoveType, int>(MoveType::Move_DarkKong, vRelatedTile[0]);
		break;
	case MoveType::Move_UpgradeKong:
		vRelatedTile.push_back(_getTileIdFromHand(oResponds.second));
		_logResponds({ "/gong", "1", std::to_string(vRelatedTile[0]) });
		return std::pair<MoveType, int>(MoveType::Move_UpgradeKong, vRelatedTile[0]);
		break;
	case MoveType::Move_Pass:
		_logResponds("/pass");
		return std::pair<MoveType, int>(MoveType::Move_Pass, -1);
		break;
	default:
		cerr << "[GamePlayer::askDarkKongOrUpgradeKong] Illegal move type: " << Move(oResponds.first, oResponds.second).toString() << endl;
		assert(0);
		break;
	}

	return std::pair<MoveType, int>();
}

//This function will update both BasePlayer's PlayerTile and BasePlayer's RemainTile.
//If you want to simply put a tile into hand (GamePlayer's and BasePlayer's), please use GamePlayer::_putTileToHand(const int &).
void GamePlayer::drawTile(const int & iTile, const bool& bInitializingHand)
{
	if(!bInitializingHand){
		assert(m_vHandIdTile.size() % 3 == 1);
		//[Log Command]
		_logCommand({ "/draw", std::to_string(iTile) });
	}

	m_pPlayer->drawTile(Tile::sgfIdToTile(iTile));
	m_vHandIdTile.push_back(iTile);
}

int GamePlayer::throwTile(const int& iSeat, const int& iTile, const bool& bPreviousMoveIsDraw)
{ 
	//[Log Command]
	_logCommand({"/throw", std::to_string(iSeat), std::to_string(iTile)});

	//update state
	Tile oTile = Tile::sgfIdToTile(iTile);

	if (iSeat == m_iSeat) {
		assert(m_vHandIdTile.size() % 3 == 2);

		//update BasePlayer	
		m_pPlayer->throwTile(oTile);

		//update m_vThrownTiles
		m_vThrownTiles.push_back(oTile);

		//update m_vHandIdTile
		return _popTileFromHandIdTile_id(iTile);
	}
	else {
		m_pPlayer->m_oRemainTile.popTile(oTile, 1, bPreviousMoveIsDraw);
	}
	
	return 0;
}

/*int GamePlayer::throwTile_Tile(const Tile& oTargetTile)
{ 
	assert(m_vHandIdTile.size() % 3 == 2);
	//update BasePlayer
	m_pPlayer->throwTile(oTargetTile); 

	//update m_vHandIdTile
	return _popTileFromHandIdTile_Tile(oTargetTile); 
}*/

//The target tile is put in vTiles[1]
int GamePlayer::eatRight(const int& iSeat, const vector<int>& vTiles)
{
	assert(vTiles.size() == 3);

	Tile oTargetTile = Tile::sgfIdToTile(vTiles[1]);
	assert(oTargetTile.isSuit() && oTargetTile.getRank() > 2 && oTargetTile.getRank() <= MAX_RANK);

	//[Log Command]
	_logCommand({ "/eat", std::to_string(iSeat), std::to_string(vTiles[0]), std::to_string(vTiles[1]), std::to_string(vTiles[2]) });

	if (iSeat == m_iSeat) {
		assert(m_vHandIdTile.size() % 3 == 1);
		
		//update BasePlayer
		m_pPlayer->doAction(Move(MoveType::Move_EatRight, oTargetTile));

		//update m_vHandIdTile
		return (_popTileFromHandIdTile_id(vTiles[0]) != -1 && _popTileFromHandIdTile_id(vTiles[2]) != -1) ? 0 : -1;
	}
	else {
		//update state
		m_pPlayer->m_oRemainTile.popTile(Tile::sgfIdToTile(vTiles[0]), 1, false);
		m_pPlayer->m_oRemainTile.popTile(Tile::sgfIdToTile(vTiles[2]), 1, false);
	}

	return 0;
}

int GamePlayer::eatMiddle(const int& iSeat, const vector<int>& vTiles)
{
	assert(vTiles.size() == 3);

	Tile oTargetTile = Tile::sgfIdToTile(vTiles[1]);
	assert(oTargetTile.isSuit() && oTargetTile.getRank() > 1 && oTargetTile.getRank() < MAX_RANK);

	//[Log Command]
	_logCommand({ "/eat", std::to_string(iSeat), std::to_string(vTiles[0]), std::to_string(vTiles[1]), std::to_string(vTiles[2]) });

	if (iSeat == m_iSeat) {
		assert(m_vHandIdTile.size() % 3 == 1);

		//update BasePlayer
		m_pPlayer->doAction(Move(MoveType::Move_EatMiddle, oTargetTile));

		//update m_vHandIdTile
		return (_popTileFromHandIdTile_id(vTiles[0]) != -1 && _popTileFromHandIdTile_id(vTiles[2]) != -1) ? 0 : -1;
	}
	else {
		//update state
		m_pPlayer->m_oRemainTile.popTile(Tile::sgfIdToTile(vTiles[0]), 1, false);
		m_pPlayer->m_oRemainTile.popTile(Tile::sgfIdToTile(vTiles[2]), 1, false);
	}

	return 0;
}

int GamePlayer::eatLeft(const int& iSeat, const vector<int>& vTiles)
{
	assert(vTiles.size() == 3);

	Tile oTargetTile = Tile::sgfIdToTile(vTiles[1]);
	assert(oTargetTile.isSuit() && oTargetTile.getRank() > 0 && oTargetTile.getRank() < MAX_RANK - 1);

	//[Log Command]
	_logCommand({ "/eat", std::to_string(iSeat), std::to_string(vTiles[0]), std::to_string(vTiles[1]), std::to_string(vTiles[2]) });

	if (iSeat == m_iSeat) {
		assert(m_vHandIdTile.size() % 3 == 1);

		//update BasePlayer
		m_pPlayer->doAction(Move(MoveType::Move_EatLeft, oTargetTile));

		//update m_vHandIdTile
		return (_popTileFromHandIdTile_id(vTiles[0]) != -1 && _popTileFromHandIdTile_id(vTiles[2]) != -1) ? 0 : -1;
	}
	else {
		//update state
		m_pPlayer->m_oRemainTile.popTile(Tile::sgfIdToTile(vTiles[0]), 1, false);
		m_pPlayer->m_oRemainTile.popTile(Tile::sgfIdToTile(vTiles[2]), 1, false);
	}

	return 0;
}

int GamePlayer::pong(const int& iSeat, const vector<int>& vTiles)
{
	assert(vTiles.size() == 3);
	Tile oTile = Tile::sgfIdToTile(vTiles[0]);

	//[Log Command]
	_logCommand({ "/pong", std::to_string(iSeat), std::to_string(vTiles[0]), std::to_string(vTiles[1]), std::to_string(vTiles[2]) });

	if (iSeat == m_iSeat) {
		assert(m_vHandIdTile.size() % 3 == 1);

		//update BasePlayer
		m_pPlayer->doAction(Move(MoveType::Move_Pong, oTile));

		//update m_vHandIdTile
		//expect pop should success (0) twice and fail (-1) once.
		int iPopResult = 0;
		for (int i = 0; i < vTiles.size(); i++) {
			iPopResult += _popTileFromHandIdTile_id(vTiles[i], false);
		}
		return iPopResult >= -1;
	}
	else {
		//update state
		m_pPlayer->m_oRemainTile.popTile(oTile, 2, false);
	}

	return 0;
}

int GamePlayer::kong(const int& iSeat, const vector<int>& vTiles)
{
	assert(vTiles.size() == 4);
	Tile oTile = Tile::sgfIdToTile(vTiles[0]);

	//[Log Command]
	_logCommand({ "/gong", std::to_string(iSeat), "4", std::to_string(vTiles[0]), std::to_string(vTiles[1]), std::to_string(vTiles[2]), std::to_string(vTiles[3]) });

	if (iSeat == m_iSeat) {
		assert(m_vHandIdTile.size() % 3 == 1);

		//update BasePlayer
		m_pPlayer->doAction(Move(MoveType::Move_Kong, oTile));

		//update m_vHandIdTile
		//expect pop should success (0) twice and fail (-1) once.
		int iPopResult = 0;
		for (int i = 0; i < vTiles.size(); i++) {
			iPopResult += _popTileFromHandIdTile_id(vTiles[i], false);
		}
		return iPopResult >= -1;
	}
	else {
		//update state
		m_pPlayer->m_oRemainTile.popTile(oTile, 3, false);
	}

	return 0;
}

int GamePlayer::darkKong(const int & iTile)
{
	assert(m_vHandIdTile.size() % 3 == 2);
	Tile oTile = Tile::sgfIdToTile(iTile);

	//[Log Command]
	int iTmpTile = (iTile / PROTOCOL_NUMBER_PLACE) * PROTOCOL_NUMBER_PLACE;
	_logCommand({ "/gong"
		, std::to_string(m_iSeat)
		, "0"
		, std::to_string(iTmpTile)
		, std::to_string(iTmpTile + 1 * PROTOCOL_ID_PLACE)
		, std::to_string(iTmpTile + 2 * PROTOCOL_ID_PLACE)
		, std::to_string(iTmpTile + 3 * PROTOCOL_ID_PLACE) });

	//update BasePlayer
	m_pPlayer->doAction(Move(MoveType::Move_DarkKong, oTile));

	//update m_vHandIdTile
	return _popTileFromHandIdTile_Tile(oTile, 4) ? 0 : -1;
}

int GamePlayer::darkKong_otherPlayer(const int & iSeat)
{
	_logCommand({ "/gong", std::to_string(iSeat), "0" });
	m_pPlayer->m_oRemainTile.decreaseRemainDrawCount(1);//there is a draw before dark kong
	return 0;
}

int GamePlayer::upgradeKong(const int& iSeat, const int & iTile)
{
	//[Log Command]
	_logCommand({ "/gong", std::to_string(iSeat), "1", std::to_string(iTile) });

	Tile oTile = Tile::sgfIdToTile(iTile);
	if (iSeat == m_iSeat) {
		assert(m_vHandIdTile.size() % 3 == 2);

		//update BasePlayer
		m_pPlayer->doAction(Move(MoveType::Move_UpgradeKong, oTile));

		//update m_vHandIdTile
		return _popTileFromHandIdTile_id(iTile);
	}
	else {
		//update state
		m_pPlayer->m_oRemainTile.popTile(oTile, 1, true);
	}

	return 0;
}

int GamePlayer::winByChunk(const int & iWinnerSeat, const int & iTile, const int & iChunkerSeat, vector<int> vWinningHandTile)
{
	//[Log Command]
	vector<string> vCommands({ "/hu", std::to_string(iWinnerSeat), std::to_string(iTile) });
	std::sort(vWinningHandTile.begin(), vWinningHandTile.end());
	const int sz = vWinningHandTile.size();
	for (int i = 0; i < sz; i++) {
		if (vWinningHandTile[i] != iTile)
			vCommands.push_back(std::to_string(vWinningHandTile[i]));
	}
	_logCommand(vCommands);
	return 0;
}

int GamePlayer::winBySelfDraw(const int & iWinnerSeat, const int & iTile, vector<int> vWinningHandTile)
{
	//[Log Command]
	vector<string> vCommands({ "/hu", std::to_string(iWinnerSeat), std::to_string(iTile) });
	std::sort(vWinningHandTile.begin(), vWinningHandTile.end());
	const int sz = vWinningHandTile.size();
	for (int i = 0; i < sz; i++) {
		if(vWinningHandTile[i] != iTile)
			vCommands.push_back(std::to_string(vWinningHandTile[i]));
	}
	_logCommand(vCommands);
	return 0;
}

int GamePlayer::endGame()
{
	_endLogCommand();
	return 0;
}

void GamePlayer::setLeftTileCount(const int & iTileCount)
{
	//do nothing now...
}


//This function only put the tile into hand (No update to BasePlayer::m_oRemainTile).
//If you want to apply draw move to GamePlayer, please use GamePlayer::draw(const int&).
void GamePlayer::_putTileToHand(const int & iTile)
{
	//update BasePlayer
	m_pPlayer->m_oPlayerTile.putTileToHandTile(Tile::sgfIdToTile(iTile));

	//update m_vHandIdTile
	m_vHandIdTile.push_back(iTile);
}

int GamePlayer::_popTileFromHandIdTile_id(const int & iTile, const bool& bAlert)
{
	auto sz = m_vHandIdTile.size();
	for (int i = 0; i < sz; i++) {
		if (m_vHandIdTile[i] == iTile) {
			m_vHandIdTile.erase(m_vHandIdTile.begin() + i);
			return iTile;//success
		}
	}

	//Error: cannot find iTile in hand
	if (bAlert) {
		cerr << "[GamePlayer::_popTileFromHandIdTile_id] Failed to pop iTile: " << iTile;
		cerr << "[GamePlayer::_popTileFromHandIdTile_id] m_vHandIdTile:";
		for (auto itr = m_vHandIdTile.begin(); itr != m_vHandIdTile.end(); itr++) {
			cerr << " " << *itr;
		}
		cerr << endl;
		assert(0);
	}
	return -1;//failed
}

int GamePlayer::_popTileFromHandIdTile_Tile(const Tile & oTile, const int& iCount)
{
	int iPopTileId;
	auto sz = m_vHandIdTile.size();
	int iRemainCount = iCount;
	for (int i = 0; i < sz; i++) {
		if (Tile::sgfIdToTile(m_vHandIdTile[i]) == oTile) {
			iPopTileId = m_vHandIdTile[i];
			m_vHandIdTile.erase(m_vHandIdTile.begin() + i);
			iRemainCount--;
			if (iRemainCount == 0)
				return iPopTileId;//success
			sz = m_vHandIdTile.size();
			i--;
		}
	}

	//Error: cannot find iTile in hand
	cerr << "[GamePlayer::_popTileFromHandIdTile_Tile] Failed to pop oTargetTile: " << oTile.toString();
	cerr << "[GamePlayer::_popTileFromHandIdTile_Tile] m_vHandIdTile:";
	for (auto itr = m_vHandIdTile.begin(); itr != m_vHandIdTile.end(); itr++) {
		cerr << " " << *itr;
	}
	cerr << endl;
	assert(0);
	return -1;//failed
}

//return the id of tile in hand
int GamePlayer::_getTileIdFromHand(const Tile & oTile) const
{
	const auto sz = m_vHandIdTile.size();
	for (int i = 0; i < sz; i++) {
		if (Tile::sgfIdToTile(m_vHandIdTile[i]) == oTile) {
			return m_vHandIdTile[i];
		}
	}

	//Error: cannot find iTile in hand
	cerr << "[GamePlayer::_getTileIdFromHand] Failed to find oTargetTile: " << oTile.toString();
	cerr << "[GamePlayer::_getTileIdFromHand] m_vHandIdTile:";
	for (auto itr = m_vHandIdTile.begin(); itr != m_vHandIdTile.end(); itr++) {
		cerr << " " << *itr;
	}
	cerr << endl;
	//assert(0);
	return -1;//failed
}

vector<int> GamePlayer::_getTileIdsFromHand(const Tile & oTile, const int & iCount) const
{
	vector<int> vAns;
	vAns.reserve(iCount);

	const auto sz = m_vHandIdTile.size();
	for (int i = 0; i < sz; i++) {
		if (Tile::sgfIdToTile(m_vHandIdTile[i]) == oTile) {
			vAns.push_back(m_vHandIdTile[i]);
			if (vAns.size() == iCount) {
				break;
			}
		}
	}

	return vAns;
}

void GamePlayer::_startLogCommand(const string& sLogName)
{
	assert(!m_fCommandLog.is_open());
	m_fCommandLog.open(sLogName, std::ios::out);
	if (!m_fCommandLog.is_open()) {
		cerr << "[GamePlayer::initSeat] Cannot open file: " << sLogName << endl;
#ifdef WINDOWS
		char errmsg[1000];
		strerror_s<1000>(errmsg, errno);
		cerr << errmsg << endl;
#endif
		assert(m_fCommandLog.is_open());
	}
}

void GamePlayer::_endLogCommand()
{
	m_fCommandLog.close();
}

void GamePlayer::_logCommand(const string & sCommand)
{
	m_fCommandLog << sCommand << endl;
}

void GamePlayer::_logCommand(const vector<string>& vCommands)
{
	if (vCommands.empty())
		return;
	
	auto sz = vCommands.size();
	m_fCommandLog << vCommands[0];
	for (int i = 1; i < sz; i++) {
		m_fCommandLog << " " << vCommands[i];
	}
	m_fCommandLog << endl;
}

void GamePlayer::_logResponds(const string & sCommand)
{
	m_fCommandLog << "getCommand: [" << sCommand << "]" << endl;
}

void GamePlayer::_logResponds(const vector<string>& vCommands)
{
	m_fCommandLog << "getCommand: [";
	if (vCommands.empty()) {
		m_fCommandLog << "]" << endl;
		return;
	}

	auto sz = vCommands.size();
	m_fCommandLog << vCommands[0];
	for (int i = 1; i < sz; i++) {
		m_fCommandLog << " " << vCommands[i];
	}
	m_fCommandLog << "]" << endl;
}
