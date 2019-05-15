using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Threading;
using UnityEngine;
namespace SkyDream
{
    internal enum TerrainNeighbor
    {
        neighborPositionLeft = 0,
        neighborPositionBottom = 1,
        neighborPositionRight = 2,
        neighborPositionTop = 3,
        neighborTotal,

    }
    internal static class TerrainConst
    {
        public const float MaxTerrainHeight = 2000;

    }
    [StructLayout(LayoutKind.Sequential)]
    internal struct MapGeneratorData
    {
        public int seed;
        public int H;
        public int I;
        ///map count at each world map edge
        public uint worldMapSize;
        public uint singleMapSize;
        public uint splatWidth;
        public uint splatCount;
        //first terrain param
        public int height0;
        public int height1;
        public int height2;
        public int height3;
        public int flags;// bit 0 & 1 - use native thread (0-native thread(c++),1-c# thread,2-c# main thread(single thread),3-not supported,bit 2 -use berlin noise

    };

    internal class Terrain
    {
        private const int mapSize = 100;
#if UNITY_IOS
    private const string dllName = "__Internal";
#else
        private const string dllName = "cppLibs";
#endif
        public delegate bool TerrainGenerationCallBack(uint terrain, uint width, Vector4 location);

        /// <summary>
        /// initilize map generator
        /// </summary>
        /// <param name="data"></param>
        [DllImport(dllName)]
        private static extern void WorldMapBindings_InitilizeWorldMap(MapGeneratorData data, string mapDir);
        /// <summary>
        /// stop generate
        /// </summary>
        [DllImport(dllName)]
        private static extern void WorldMapBindings_StopGeneration();
        /// <summary>
        /// destroy generator
        /// </summary>
        [DllImport(dllName)]
        private static extern void WorldMapBindings_Destroy();
        /// <summary>
        /// set callback (when map generate success)
        /// </summary>
        /// <param name="cb"></param>
        [DllImport(dllName)]
        private static extern void WorldMapBindings_SetGenerateCallBack(TerrainGenerationCallBack cb);
        [DllImport(dllName)]
        private static extern uint WorldMapBindings_GetNeighbor(uint who, int dir);
        [DllImport(dllName)]
        private static extern void WorldMapBindings_UpdateInMainThread(int diff);
        [DllImport(dllName)]
        private static extern void WorldMapBindings_WorkThreadRunner();

        private Dictionary<int, SkyDream.TerrainPiece> m_mapTerrains = new Dictionary<int, TerrainPiece>();
        public static int maxLod = 3;
        private Thread workThread;
        MapGeneratorData _data;
        public void Init()
        {
            UnityCppBindings.Init(32);
            MapGeneratorData data = new MapGeneratorData()
            {
                seed = Random.Range(0, 100),
                H = 10,
                I = 5,
                singleMapSize = 65,/*64+1,1 for same edge line*/
                worldMapSize = 1,//only support 1
                splatWidth = 65,
                splatCount = 2,
                height0 = 500,
                height1 = 300,
                height2 = 800,
                height3 = 240,
                flags = 0
            };
            _data = data;
            WorldMapBindings_InitilizeWorldMap(data, Application.dataPath + "/../Map");
            WorldMapBindings_SetGenerateCallBack(OnMapGenerateSuccess);
            /* if ((data.flags & 0x3) == 1)
             {
                 ThreadStart start = new ThreadStart(Runner);
                 workThread = new Thread(start);
                 workThread.Start();
             }
             else if ((data.flags & 0x3) == 2)
                 WorldMapBindings_WorkThreadRunner();*/
            // Debug.LogFormat("WorldMapBindings_WorkThreadRunner exit");
            //Runner();
        }

        private bool _runner_exited = false;

        private void Runner()
        {
            _runner_exited = false;
            WorldMapBindings_WorkThreadRunner();
            Debug.LogFormat("WorldMapBindings_WorkThreadRunner exit");
            _runner_exited = true;
            //WorldMapBindings_Destroy();
        }
        ~Terrain()
        {
        }
        private static bool OnMapGenerateSuccess(uint terrain, uint width, Vector4 location)
        {
            Debug.LogFormat("OnMapGenerateSuccess {0} {1},exist {2}", terrain, location, UnityCppBindings.GetTerrain(terrain) != null);
            var terr = UnityCppBindings.GetTerrain(terrain);
            if (null != terr)
            {
                return false;
            }
            else
            {
                terr = new TerrainPiece(terrain, (int)width);
                return terr.TerrainInitilizer(width, location);
            }
        }

        private int tick = 0;
        public void Update(int time_diff)
        {
            tick++;
            if (tick > 20)
            {
                tick = 0;
                WorldMapBindings_UpdateInMainThread(time_diff);
            }
        }
        public void Destroy()
        {
            Debug.LogFormat("WorldMapBindings_StopGeneration");
            WorldMapBindings_StopGeneration();
            Debug.LogFormat("WorldMapBindings_Destroy");
            WorldMapBindings_Destroy();
            UnityCppBindings.Clear();
            Debug.LogFormat("over");
        }
    }
}