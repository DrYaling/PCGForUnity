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
    }
    internal static class TerrainConst
    {
        public const float MaxTerrainHeight = 1000;
        public const int maxVerticesPerMesh = 65000;
        public const int meshTopologyVertice = 0;
        public const int meshTopologyTriangle = 1;
        public const int meshTopologyUV = 2;
        public const int meshTopologyMeshCount = 3;
        public const int meshTopologyNormal = 4;
        public const float lod_1_Distance = 30f;/*meters*/
        public const float lod_2_Distance = 60f;/*meters*/
        public const float lod_3_Distance = 90f;/*meters*/
        /* public const int mesh_arg_seed = 0;
         public const int mesh_arg_lod = 1;
         public const int mesh_arg_I = 2;
         public const int mesh_arg_H = 3;
         public const int mesh_arg_mapWidth = 4;
         public const int mesh_arg_h0 = 5;
         public const int mesh_arg_h1 = 6;
         public const int mesh_arg_h2 = 7;
         public const int mesh_arg_h3 = 8;
         public const int mesh_arg_h4 = 9;
         public const int mesh_arg_useuv = 10;*/

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
        private static extern void WorldMapBindings_InitilizeWorldMap(MapGeneratorData data);
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
        public void Init()
        {
            Debug.LogFormat("1");
            MapGeneratorData data = new MapGeneratorData()
            {
                seed = Random.Range(0, 100),
                H = 30,
                I = 5,
                singleMapSize = 0,
                worldMapSize = 8,
                splatWidth = 512,
                splatCount = 2,
                height0 = 500,
                height1 = 300,
                height2 = 200,
                height3 = 240
            };
            Debug.LogFormat("2");
            WorldMapBindings_InitilizeWorldMap(data);
            Debug.LogFormat("3");
            WorldMapBindings_SetGenerateCallBack(OnMapGenerateSuccess);
            /*ThreadStart start = new ThreadStart(Runner);
            workThread = new Thread(start);
            workThread.Start();*/
            //WorldMapBindings_WorkThreadRunner();
            Runner();
        }

        private bool _runner_exited = false;

        private void Runner()
        {
            _runner_exited = false;
            WorldMapBindings_WorkThreadRunner();
            Debug.LogFormat("WorldMapBindings_WorkThreadRunner exit");
            _runner_exited = true;
            WorldMapBindings_Destroy();
        }
        ~Terrain()
        {
            WorldMapBindings_StopGeneration();
        }
        private static bool OnMapGenerateSuccess(uint terrain, uint width, Vector4 location)
        {
            Debug.LogFormat("4");
            var terr = UnityCppBindings.GetTerrain(terrain);
            if (null != terr)
            {
                return terr.TerrainInitilizer(width, location);
            }
            return false;
        }
        public void Update(int time_diff)
        {
            //WorldMapBindings_UpdateInMainThread(time_diff);
        }
    }
}