#ifndef Map_manager_h
#define Map_manager_h
#include "define.h"
namespace server
{
	class MapManager 
	{
	public:
		static MapManager* GetInstance();
		MapManager();
		~MapManager();
		void Init();
		void Update(uint32_t diff);

	};
#define sMapMgr MapManager::GetInstance()
}
#endif // !Map_manager_h
