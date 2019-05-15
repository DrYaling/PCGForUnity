using System.Diagnostics;
using System.Runtime.InteropServices;
using UnityEngine;
using Debug = UnityEngine.Debug;

namespace SkyDream
{
    public enum TerrainInitType
    {
        HeightMap = 0,
    }
    public class TerrainPiece
    {
        private const int mapSize = 100;
#if UNITY_IOS
    private const string dllName = "__Internal";
#else
        private const string dllName = "cppLibs";
#endif
        [DllImport(dllName)]
        private static extern uint WorldMapBindings_GetNeighbor(uint who, int dir);
        [DllImport(dllName)]
        private static extern unsafe void WorldMapBindings_InitTerrain(uint who, float* heightMap, int heightMapSize, float* splatMap, int splatWidth, int splatCount);

        private Texture2D[] texture2s = new Texture2D[4];
        private uint terrainInstance = 0;
        private float[,] _heightMap;
        private float[,,] _splatMap;
        private const int splatMapSize = 65;
        private const int splatCount = 2;
        private float _mapWidth;
        public uint instaneId
        {
            get
            {
                return terrainInstance;
            }
        }

        // private int MapWidth = 400;
        private TerrainPiece _neighborLeft;
        private TerrainPiece _neighborRight;
        private TerrainPiece _neighborBottom;
        private TerrainPiece _neighborTop;
        private UnityEngine.Terrain terrain;
        private TerrainData terrainData;

        private TerrainPiece() { }
        public unsafe TerrainPiece(uint ins, int heightMapWidth)
        {
            terrainInstance = ins;
            UnityCppBindings.RegistBinding(terrainInstance, this);
            _heightMap = UnityCppBindings.GetHeightMapCache();
            _splatMap = new float[splatMapSize, splatMapSize, splatCount];
            //StartGenerateOrLoad(meshInstanceId);
            if (null == texture2s[0])
            {

                texture2s[0] = Resources.Load<Texture2D>("Realistic Terrain Collection/Other/Textures/ForestFloor2/forest_floor_2");
                texture2s[1] = Resources.Load<Texture2D>("Realistic Terrain Collection/Other/Textures/ForestFloor2/forest_floor_2_normal");
                texture2s[2] = Resources.Load<Texture2D>("Realistic Terrain Collection/Other/Textures/SolidRock2/solid_rock_2");
                texture2s[3] = Resources.Load<Texture2D>("Realistic Terrain Collection/Other/Textures/SolidRock2/solid_rock_2_normal");
            }
        }
        ~TerrainPiece()
        {
            Release();
        }
        internal void SetNeighbor(TerrainPiece neighbor, TerrainNeighbor position)
        {
            switch (position)
            {
                case TerrainNeighbor.neighborPositionLeft:
                    _neighborLeft = neighbor;
                    break;
                case TerrainNeighbor.neighborPositionRight:
                    _neighborRight = neighbor;
                    break;
                case TerrainNeighbor.neighborPositionBottom:
                    _neighborBottom = neighbor;
                    break;
                case TerrainNeighbor.neighborPositionTop:
                    _neighborTop = neighbor;
                    break;
            }
        }
        private UnityEngine.Terrain GetNeighbor(TerrainNeighbor neighbor)
        {
            switch (neighbor)
            {
                case TerrainNeighbor.neighborPositionLeft:
                    if (null != _neighborLeft)
                    {
                        return _neighborLeft.terrain;
                    }
                    break;
                case TerrainNeighbor.neighborPositionRight:
                    if (null != _neighborRight)
                    {
                        return _neighborRight.terrain;
                    }
                    break;
                case TerrainNeighbor.neighborPositionBottom:
                    if (null != _neighborBottom)
                    {
                        return _neighborBottom.terrain;
                    }
                    break;
                case TerrainNeighbor.neighborPositionTop:
                    if (null != _neighborTop)
                    {
                        return _neighborTop.terrain;
                    }
                    break;
                default:
                    break;
            }
            return null;
        }
        public unsafe bool TerrainInitilizer(uint width, Vector4 location)
        {
            Stopwatch watch = new Stopwatch(); ;
            watch.Start(); ;
            _mapWidth = location.w;
            Debug.LogFormat("TerrainInitilizer wdith {0},location {1}", width, location);
            fixed (float* ptr_height = _heightMap)
            {
                fixed (float* ptr_splat = _splatMap)
                {
                    WorldMapBindings_InitTerrain(instaneId, ptr_height, _heightMap.GetLength(0),
                        ptr_splat, _splatMap.GetLength(0), _splatMap.GetLength(2));
                }
                //传递高度图的首地址
            }
            watch.Stop();
            Debug.LogFormat("init terrain data cost {0} ms", watch.ElapsedMilliseconds);
            watch.Reset();
            watch.Start();
            OnHeightMapLoaded();
            watch.Stop();
            Debug.LogFormat("init terrain cost {0} ms", watch.ElapsedMilliseconds);

            SetPosition(new Vector3(location.x, location.y, location.z));
            return true;
        }
        private void OnHeightMapLoaded()
        {
            terrainData = new TerrainData();
            //地形设置
            int size = _heightMap.GetLength(0);
            Vector3 terrainSize = new Vector3(_mapWidth, TerrainConst.MaxTerrainHeight, _mapWidth); //地形的大小，最高、最低点的差值
            //Debug.LogFormat("Terrain {0} Size {1}", terrainInstance, terrainSize);
            terrainData.size = terrainSize;
            terrainData.heightmapResolution = size + 1;
            terrainData.SetHeights(0, 0, _heightMap); //一切都是为了这个方法...
            var tmp = new SplatPrototype[splatCount];
            var splat0 = new SplatPrototype();
            splat0.texture = texture2s[0];
            splat0.normalMap = texture2s[1];
            splat0.smoothness = 0.5f;

            var splat1 = new SplatPrototype();
            splat1.texture = texture2s[2];
            splat1.normalMap = texture2s[3];
            splat1.smoothness = 0.5f;
            tmp[0] = splat0;
            tmp[1] = splat1;
            terrainData.splatPrototypes = tmp;
            Debug.LogFormat("splat of terrain {0},{1},{2},{3}", instaneId, _splatMap.GetLength(0), _splatMap.GetLength(1), _splatMap.GetLength(2));
            terrainData.SetAlphamaps(0, 0, _splatMap);
            var t = UnityEngine.Terrain.CreateTerrainGameObject(terrainData);
            t.name = terrainInstance.ToString();
            terrain = t.GetComponent<UnityEngine.Terrain>();
            terrain.heightmapMaximumLOD = SkyDream.Terrain.maxLod;
            SetNeighbors();
            terrain.terrainData.size = terrainSize;
            terrain.Flush();
            Debug.LogFormat("terrain {0} create success ", instaneId);

        }
        private void SetNeighbors(TerrainNeighbor ignoreNeighbor = TerrainNeighbor.neighborTotal)
        {
            for (TerrainNeighbor n = TerrainNeighbor.neighborPositionLeft; n <= TerrainNeighbor.neighborPositionTop; n++)
            {
                uint terr = WorldMapBindings_GetNeighbor(instaneId, (int)n);
                var t = UnityCppBindings.GetTerrain(terr);
                SetNeighbor(t, n);
                //prevent dead loop
                if (null != t && n != ignoreNeighbor)
                {
                    t.SetNeighbors((TerrainNeighbor)((int)(n + 2) % (int)TerrainNeighbor.neighborTotal));
                }
            }
            terrain.SetNeighbors(GetNeighbor(TerrainNeighbor.neighborPositionLeft),
                GetNeighbor(TerrainNeighbor.neighborPositionTop),
                GetNeighbor(TerrainNeighbor.neighborPositionRight),
                GetNeighbor(TerrainNeighbor.neighborPositionBottom));
            terrain.Flush();
        }
        internal void Release()
        {
            //UnityCppBindings.RecycleCache(_heightMap);
            UnityCppBindings.UnResistBinding(terrainInstance);
            terrainInstance = 0;
        }
        public void SetPosition(Vector3 position)
        {
            if (null != terrain)
            {
                terrain.transform.position = position;
            }
            else
            {
                Debug.LogErrorFormat("Terrain {0} is not loaded yet", terrainInstance);
            }
        }
        public void Update(int time_diff)
        {

        }
    }
}
