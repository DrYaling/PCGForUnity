#include "MapManager.h"
namespace server
{
	static MapManager instance;
	MapManager * MapManager::GetInstance()
	{
		return &instance;
	}

	MapManager::MapManager()
	{
	}

	MapManager::~MapManager()
	{
	}

	void MapManager::Init()
	{
	}

	void MapManager::Update(uint32_t diff)
	{
	}

}