#ifndef Map_Generator_h
#define Map_Generator_h
#include "define.h"
#include <map>
#include "Terrain/Terrain.h"
#include "Generators/TerrainGenerator/Diamond_Square.h"
#include <mutex>
#include <functional>
#include <thread>
#include <memory>
#include "Utinities/LockedQueue.h"
#include "G3D/Vector4.h"
#include <atomic>
namespace generator
{
	typedef bool(__stdcall *TerrainGenerationCallBack)(uint32_t terrainId, uint32_t terrainWidth, G3D::Vector4 locate);
	class MapInstanceIdGenerator
	{
	public:
		uint32_t GetInstanceId();
		MapInstanceIdGenerator() = delete;

	};
	class MapGenerator
	{
	public:
		MapGenerator();
		~MapGenerator();
		void Init(MapGeneratorData data);
		void SetNativeDirectory(std::string dir) { m_sSaveDirectory = dir; }
		static MapGenerator* GetInstance();
		static void Destroy();
		void StartRun();
		void Stop();
		void UpdateInMainThread(int32_t diff);
		void SetCallBack(TerrainGenerationCallBack cb) { m_cbTerrainGenFinish = cb; }
		void InitTerrainInMainThread(uint32_t terrain, float* heightMap, int32_t heightMapSize, float* splatMap, int32_t splatWidth, int32_t splatCount);
		std::shared_ptr<Terrain> GetTerrain(uint32_t terr);
		void SaveTerrain(uint32_t terr);
		void WorkThreadEntry();
	private:
		void GenWorldMap();
		void AutoGenSplatMap();
		void GenerateTerrain();
		void WorkThread();
		uint32 InitilizeNext();
		void Generate(uint32);
		static inline bool GetNeighborHeight(int32_t x, int32_t y, NeighborType neighbor, uint32_t owner, float & p);
		void InitHeightMapBaseOnNeighbor(NeighborType position, std::shared_ptr<Terrain>);
		uint32_t GetNeighborID(NeighborType dir, uint32_t who);
		bool LoadFromNative(uint32_t terr);
	private:
		std::map<uint32_t, std::shared_ptr<Terrain>> m_mTerrainData;
		LockedQueue<uint32_t> m_finishQueue;
		std::shared_ptr<Terrain> m_pWorldMap;
		std::shared_ptr<Terrain> m_pCurrentMap;
		Diamond_Square* m_pGenerator;
		AutomaticPainter* m_pPainter;
		float* m_worldMapHeightMap;
		float* m_aHeightMap;
		float* m_aSplatMap;
		float* m_aHeightMapCopy;
		float* m_aSplatMapCopy;
		TerrainGenerationCallBack m_cbTerrainGenFinish;
		uint32_t m_nCurrentTerrain;
		std::mutex m_generatorMtx;
		std::thread* m_pThreadRunner;
		MapGeneratorData m_stData;
		std::atomic<bool> m_bRun;
		std::atomic<bool> m_bThreadExited;
		uint32_t m_nTotalMapCount;
		std::string m_sSaveDirectory;

	};
#define sMapGenerator MapGenerator::GetInstance()
}
#endif // !Map_Generator_h
