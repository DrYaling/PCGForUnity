using System.Runtime.InteropServices;
using UnityEngine;

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

        private uint terrainInstance = 0;
        private float[,] _heightMap;
        private float[,,] _splatMap;
        private const int splatMapSize = 512;
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
        public unsafe TerrainPiece(int heightMapWidth)
        {
            UnityCppBindings.RegistBinding(terrainInstance, this);
            _heightMap = new float[heightMapWidth, heightMapWidth];
            _splatMap = new float[splatMapSize, splatMapSize, splatCount];
            //StartGenerateOrLoad(meshInstanceId);

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
            _mapWidth = location.w;
            fixed (float* ptr_height = _heightMap)
            {
                fixed (float* ptr_splat = _splatMap)
                {
                    Debug.LogFormat("6");
                    WorldMapBindings_InitTerrain(instaneId, ptr_height, _heightMap.GetLength(0),
                        ptr_splat, _splatMap.GetLength(0), _splatMap.GetLength(2));
                }
                //传递高度图的首地址
            }
            OnHeightMapLoaded();
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
            splat0.texture = Resources.Load<Texture2D>("Realistic Terrain Collection/Other/Textures/ForestFloor2/forest_floor_2");
            splat0.normalMap = Resources.Load<Texture2D>("Realistic Terrain Collection/Other/Textures/ForestFloor2/forest_floor_2_normal");
            splat0.smoothness = 0.5f;

            var splat1 = new SplatPrototype();
            splat1.texture = Resources.Load<Texture2D>("Realistic Terrain Collection/Other/Textures/SolidRock2/solid_rock_2");
            splat1.normalMap = Resources.Load<Texture2D>("Realistic Terrain Collection/Other/Textures/SolidRock2/solid_rock_2_normal");
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

        }
        private void SetNeighbors()
        {
            for (TerrainNeighbor n = TerrainNeighbor.neighborPositionLeft; n < TerrainNeighbor.neighborPositionTop; n++)
            {
                Debug.LogFormat("7 "+n.ToString());
                uint terr = WorldMapBindings_GetNeighbor(instaneId, (int)n);
                var t = UnityCppBindings.GetTerrain(terr);
                SetNeighbor(t, n);
            }
            terrain.SetNeighbors(GetNeighbor(TerrainNeighbor.neighborPositionLeft),
                GetNeighbor(TerrainNeighbor.neighborPositionTop),
                GetNeighbor(TerrainNeighbor.neighborPositionRight),
                GetNeighbor(TerrainNeighbor.neighborPositionBottom));
            terrain.Flush();
        }
        internal void Release()
        {
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
