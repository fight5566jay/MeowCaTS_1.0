#include "Wall.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <ctime>
#include <random>
using std::stringstream;

Wall::Wall(const int & iSeed)
{
	//if (iSeed > 0)
	//	srand(iSeed);
	//else
	//	srand(time(NULL));

	m_vTile.clear();
	//suit
	for (int i = 1; i <= SUIT_COUNT; i++) 
		for (int j = 1; j <= MAX_SUIT_TILE_RANK; j++) 
			for (int k = 0; k < SAME_TILE_COUNT; k++) 
				m_vTile.push_back(i * 100 + j * 10 + k);
	//honor
	for (int i = SUIT_COUNT + 1; i <= TILE_TYPE_COUNT; i++)
		for (int j = 1; j <= MAX_HONOR_TILE_RANK; j++)
			for (int k = 0; k < SAME_TILE_COUNT; k++)
				m_vTile.push_back(i * 100 + j * 10 + k);

	//std::random_shuffle(m_vTile.begin(), m_vTile.end());//until c++11
	std::random_device rd;
	std::mt19937 gen(rd());
	if (iSeed > 0) {
		gen.seed(iSeed);
	}
	std::shuffle(m_vTile.begin(), m_vTile.end(), gen);
	m_iFrontIndex = 0;
	m_iRearIndex = TOTAL_TILE_COUNT;
	m_iUsedTileCountforGame = TOTAL_TILE_COUNT - NONPICKABLE_TILE_COUNT;
	//std::cerr << "[Wall]" << std::endl << getSgfString() << std::endl;
}

string Wall::getSgfString() const
{
	if (m_iFrontIndex != 0) {
		std::cerr << "[Wall::getSgfString]Only use before first get tile!!" << std::endl;
		exit(1);
	}

	stringstream ss;
	string str = "SQRWALL";
	for (int i = 0; i < TOTAL_TILE_COUNT; i++) {
		string sTile = std::to_string(m_vTile.at(i));
		str.append("[" + sTile + "]");
	}
	return str;
}

string Wall::toString() const
{
	if (m_iFrontIndex != 0) {
		std::cerr << "[Wall::toString]Only use before first get tile!!" << std::endl;
		exit(1);
	}

	stringstream ss;
	ss << "SQRWALL:";
	for (int i = 0; i < TOTAL_TILE_COUNT; i++) {
		ss << "[" << m_vTile.at(i) << "]";
	}
	return ss.str();
}
