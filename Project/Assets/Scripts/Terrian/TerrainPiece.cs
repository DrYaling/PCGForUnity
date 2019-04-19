using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

namespace SkyDram
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
        public delegate void MeshInitilizer(int target, int type, int arg0, int arg1, int ar2);

        [DllImport(dllName)]
        extern static void RegisterTerrianMeshBindings(int ins);
        //if mapSize > 0 use perlin noise
        [DllImport(dllName)]
        extern static unsafe void InitTerrianMesh(int ins, [In]int[] args, int argsize, [In, Out]float* heightMap, int heightMapSize, MeshInitilizer cb);
        [DllImport(dllName)]
        extern static void SetMeshNeighbor(int ins, int neighbor, int dir);
        [DllImport(dllName)]
        extern static void StartGenerateOrLoad(int ins);
        [DllImport(dllName)]
        extern static void ReleaseMeshGenerator(int ins);
        [DllImport(dllName)]
        extern static void FlushMeshGenerator(int ins);
        [DllImport(dllName)]
        extern static unsafe void GetTerraniHeightMap(int instanceId, [Out]float* heightMap, int sizeX, int sizey);
        [DllImport(dllName)]
        extern static unsafe void InitTerrainPainter(int instanceId, float* alphaMap, int sizeXy, int splatCount);
        int terrainInstance = EgineUtils.GetInstanceId();
        float[,] _heightMap;
        float[,,] _splatMap;
        const int splatMapSize = 512;
        const int splatCount = 2;
        public int instaneId
        {
            get
            {
                return terrainInstance;
            }
        }
        private int MapWidth = 400;
        TerrainPiece _neighborLeft;
        TerrainPiece _neighborRight;
        TerrainPiece _neighborBottom;
        TerrainPiece _neighborTop;
        Terrain terrain;
        TerrainData terrainData;
        int _maxLod;
        TerrainPiece() { }
        public unsafe TerrainPiece(int mapSize, int maxLod = 3, bool useUV = false)
        {
            //Debug.LogFormat("mesh map size {0} ,maxLod {1}", mapSize, maxLod);
            //perfect low resolution mesh size
            switch(mapSize)
            {
                case 1:
                    MapWidth = 16;
                    break;
                case 2:
                    mapSize = 32;
                    break;
                case 3:
                    MapWidth = 64;
                    break;
                case 4:
                    MapWidth = 128;
                    break;
                case 5:
                    MapWidth = 512;
                    break;
                default:
                    Debug.LogErrorFormat("Error MapSize {0} not supported",mapSize);
                    return;
            }
            int heightMapWidth = (int)(Mathf.Pow(2, 2 * mapSize) + 1);
            int heightMapSize = heightMapWidth * heightMapWidth;
            _maxLod = maxLod;
            UnityCppBindings.RegistBinding(terrainInstance, this);
            RegisterTerrianMeshBindings(terrainInstance);
            int[] args = GetInitArgs(Random.Range(0, 5), maxLod, mapSize, 33, -1, 380, 500, 240, 700);
            _heightMap = new float[heightMapWidth, heightMapWidth];
            /* MapWidth = _heightMap.GetLength(0) ;
             if (MapWidth % 2 != 0)
                 MapWidth++;*/
            fixed (float* ptr = _heightMap)
            {
                //传递高度图的首地址
                InitTerrianMesh(terrainInstance, args, args.Length, ptr, heightMapSize, TerrainInitilize);
            }
            //StartGenerateOrLoad(meshInstanceId);

        }
        ~TerrainPiece()
        {
            Release();
        }
        public int GetSize()
        {
            return MapWidth;
        }
        public void Load()
        {
            StartGenerateOrLoad(terrainInstance);
            FlushMeshGenerator(terrainInstance);
        }
        public void SetNeighbor(TerrainPiece neighbor, int position)
        {
            switch (position)
            {
                case TerrainConst.neighborPositionLeft:
                    _neighborLeft = neighbor;
                    break;
                case TerrainConst.neighborPositionRight:
                    _neighborRight = neighbor;
                    break;
                case TerrainConst.neighborPositionBottom:
                    _neighborBottom = neighbor;
                    break;
                case TerrainConst.neighborPositionTop:
                    _neighborTop = neighbor;
                    break;
            }
            SetMeshNeighbor(terrainInstance, neighbor.terrainInstance, position);
            if (null != terrain)//已经创建过地形
            {
                SetNeighbors();

            }
        }
        private Terrain GetNeighbor(int neighbor)
        {
            switch (neighbor)
            {
                case TerrainConst.neighborPositionLeft:
                    if (null != _neighborLeft)
                        return _neighborLeft.terrain;
                    break;
                case TerrainConst.neighborPositionRight:
                    if (null != _neighborRight)
                        return _neighborRight.terrain;
                    break;
                case TerrainConst.neighborPositionBottom:
                    if (null != _neighborBottom)
                        return _neighborBottom.terrain;
                    break;
                case TerrainConst.neighborPositionTop:
                    if (null != _neighborTop)
                        return _neighborTop.terrain;
                    break;
                default:
                    break;
            }
            return null;
        }
        private int[] GetInitArgs(int seed, int maxLod, int I, int H, int mapWidth, int h0, int h1, int h2, int h3, bool useuv = false)
        {
            return new int[] { seed, maxLod, I, H, mapWidth, h0, h1, h2, h3, useuv ? 1 : 0 };
        }
        private static void TerrainInitilize(int target, int type, int arg0, int arg1, int arg2)
        {
            TerrainPiece tmesh = UnityCppBindings.GetMesh(target);
            if (null != tmesh)
            {
                tmesh.TerrainInitilizer((TerrainInitType)type, arg0, arg1, arg2);
            }
            else
            {
                Debug.LogErrorFormat("mesh target {0} does not exist", target);
            }
        }
        public unsafe void TerrainInitilizer(TerrainInitType type, int arg0, int arg1, int arg2)
        {
            //Debug.LogFormat("TerrainInitilizer type {0} arg0 {1},arg1 {2},arg2 {3}", type, arg0, arg1, arg2);
            switch (type)
            {
                case TerrainInitType.HeightMap:
                    /*_heightMap = new float[arg0, arg1];
                    int row = _heightMap.GetUpperBound(0) + 1;
                    int col = _heightMap.GetUpperBound(1) + 1;
                    fixed (float* ptr = _heightMap)
                    {
                        //传递高度图的首地址
                        GetTerraniHeightMap(terrainInstance, ptr, arg0, arg1);
                    }*/
                    _splatMap = new float[splatMapSize, splatMapSize, splatCount];
                    Debug.LogFormat("TerrainInitilizer splat of terrain {0},{1},{2},{3}", instaneId, _splatMap.GetLength(0), _splatMap.GetLength(1), _splatMap.GetLength(2));
                    fixed (float* ptr = _splatMap)
                    {
                        InitTerrainPainter(instaneId, ptr, splatMapSize, splatCount);
                    }
                    Debug.LogFormat("TerrainInitilizer splat of terrain {0},{1},{2},{3}", instaneId, _splatMap.GetLength(0), _splatMap.GetLength(1), _splatMap.GetLength(2));
                    OnHeightMapLoaded();
                    break;
                default:
                    break;
            }
        }
        private void OnHeightMapLoaded()
        {
            terrainData = new TerrainData();
            //地形设置
            int size = _heightMap.GetLength(0);
            Vector3 terrainSize = new Vector3(MapWidth, TerrainConst.MaxTerrainHeight, MapWidth); //地形的大小，最高、最低点的差值
            //Debug.LogFormat("Terrain {0} Size {1}", terrainInstance, terrainSize);
            terrainData.size = terrainSize;
            terrainData.heightmapResolution = size + 1;
            terrainData.SetHeights(0, 0, _heightMap); //一切都是为了这个方法...
            var tmp = new SplatPrototype[splatCount];
            var splat0 = new SplatPrototype();
            splat0.texture = Resources.Load<Texture2D>("Realistic Terrain Collection/Other/Textures/ForestFloor2/forest_floor_2");
            splat0.normalMap = Resources.Load<Texture2D>("Realistic Terrain Collection/Other/Textures/ForestFloor2/forest_floor_2_normal");

            var splat1 = new SplatPrototype();
            splat1.texture = Resources.Load<Texture2D>("Realistic Terrain Collection/Other/Textures/SolidRock2/solid_rock_2");
            splat1.normalMap = Resources.Load<Texture2D>("Realistic Terrain Collection/Other/Textures/SolidRock2/solid_rock_2_normal");
            tmp[0] = splat0;
            tmp[1] = splat1;
            terrainData.splatPrototypes = tmp;
            Debug.LogFormat("splat of terrain {0},{1},{2},{3}",instaneId,_splatMap.GetLength(0),_splatMap.GetLength(1),_splatMap.GetLength(2));
            terrainData.SetAlphamaps(0, 0, _splatMap);
           /* int xmin = 0;
            int xmax = 0;
            int ymin = 0;
            int ymax = 0;
            if (instaneId == 0)
            {
                xmin = 1110;
                xmax = 200;
                ymin = 500;
                ymax = 511;
            }
            for (int x = xmin; x <= xmax; x++)
            {
                for (int y = ymin; y <= ymax; y++)
                {
                    //Debug.LogErrorFormat("height of {0} is at x {1},y {2} is  {3}-normalized {4}", instaneId, x, y, terrainData.GetHeight(x, y), _heightMap[y, x]);
                    for(int i = 0;i<2;i++)
                    {
                        Debug.LogFormat("splat x {0},y {1},alpha {2} is {3}",x,y,i,_splatMap[x,y,i]);
                    }
                }
            }*/
            var t = Terrain.CreateTerrainGameObject(terrainData);
            t.name = terrainInstance.ToString();
            terrain = t.GetComponent<Terrain>();
            terrain.heightmapMaximumLOD = _maxLod;
            SetNeighbors();
            terrain.terrainData.size = terrainSize;
            terrain.Flush();

        }
        private void SetNeighbors()
        {
            terrain.SetNeighbors(GetNeighbor(TerrainConst.neighborPositionLeft), GetNeighbor(TerrainConst.neighborPositionTop), GetNeighbor(TerrainConst.neighborPositionRight), GetNeighbor(TerrainConst.neighborPositionBottom));
            terrain.Flush();
        }
        internal void Release()
        {
            //Debug.LogFormat("Release mesh {0}", instaneId);
            ReleaseMeshGenerator(terrainInstance);
            UnityCppBindings.UnResistBinding(terrainInstance);
            terrainInstance = 0;
        }
        public void SetPosition(Vector3 position)
        {
            if (null != terrain)
                terrain.transform.position = position;
            else
                Debug.LogErrorFormat("Terrain {0} is not loaded yet", terrainInstance);
        }
        public void Update(int time_diff)
        {

        }
    }
}
