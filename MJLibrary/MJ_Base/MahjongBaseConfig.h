//
//  Config.h
//  Mahjong
//
//  Created by nanj0178 on 2015/2/18.
//  Copyright (c) 2015 CGI. All rights reserved.
//

#pragma once
// System
#define WINDOWS
//#define LINUX 

//coding related
#define MJ_DEBUG_MODE //mainly use for assertion

// Feature
#define HEURISTIC_UPPER_BOUND 0
//#define USE_TT 1
#define DUMP_SGF 1
#if DUMP_SGF
    #define DUMP_TREE_SGF 1
#endif

// Global Game define

//protocol
#define PROTOCOL_ID_PLACE 1
#define PROTOCOL_NUMBER_PLACE 10
#define PROTOCOL_SYMBOL_PLACE 100

//Mahjong related
#define PLAYER_COUNT 4

#define SAME_TILE_COUNT 4
#define SUIT_COUNT 3
#define HONOR_COUNT 1
const int TILE_TYPE_COUNT = SUIT_COUNT + HONOR_COUNT;
#define MAX_SUIT_TILE_RANK 9
#define MAX_HONOR_TILE_RANK 7
const int MAX_RANK = (MAX_SUIT_TILE_RANK > MAX_HONOR_TILE_RANK) ? MAX_SUIT_TILE_RANK : MAX_HONOR_TILE_RANK;
const int MAX_DIFFERENT_TILE_COUNT = (SUIT_COUNT * MAX_SUIT_TILE_RANK + HONOR_COUNT * MAX_HONOR_TILE_RANK);
const int TOTAL_TILE_COUNT = (MAX_DIFFERENT_TILE_COUNT * SAME_TILE_COUNT);

#define NEED_GROUP 5
#define NEED_EYEs 1//don't modified this except it is necessary.
const int MAX_HANDTILE_COUNT = (NEED_GROUP * 3 + NEED_EYEs * 2);
#define MAX_MELD_COUNT NEED_GROUP
#define MAX_TILE_COUNT_IN_ONE_MELD 4
#define MAX_DRAW_COUNT 26//[Not general]how to calculate?
const int AVERAGE_MAX_DRAW_COUNT = (TOTAL_TILE_COUNT / PLAYER_COUNT - (MAX_HANDTILE_COUNT - 1));
const int MAX_DISCARDED_TILE_COUNT = (MAX_DRAW_COUNT - NEED_GROUP);
const int MAX_MINLACK = (NEED_GROUP * 2 + NEED_EYEs);
#define NONPICKABLE_TILE_COUNT 16

const int DIFF_SUIT_TILE_COUNT = (SUIT_COUNT * MAX_SUIT_TILE_RANK);

//feature, observation related
#define FEATURE_WIDTH MAX_DIFFERENT_TILE_COUNT
#define FEATURE_HEIGHT 1
const int FEATURE_SIZE = (FEATURE_WIDTH * FEATURE_HEIGHT);

// Other
#define LINUX_INI_PATH "./config.ini"
#define WINDOWS_INI_PATH "config.ini"

#undef _HAS_STD_BYTE