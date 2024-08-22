#pragma once

enum eTILE_STATUS
{
	TILE_PATH = 0,
	TILE_WALL,
	TILE_START,
	TILE_DEST,
	TILE_NODE_NORMAL,
	TILE_NODE_LL,
	TILE_NODE_LU,
	TILE_NODE_UU,
	TILE_NODE_RU,
	TILE_NODE_RR,
	TILE_NODE_RD,
	TILE_NODE_DD,
	TILE_NODE_LD,
};
enum eCurMode
{
	PATH_FIND_ASTAR = 0,
	PATH_FIND_JPS,
	PATH_FIND_COMPARE
};

constexpr unsigned int GRID_WIDTH = 80;
constexpr unsigned int GRID_HEIGHT = 40;
constexpr unsigned int GRID_SIZE = 16;

constexpr unsigned int G_WEIGHT = 1;
constexpr unsigned int H_WEIGHT = 1;