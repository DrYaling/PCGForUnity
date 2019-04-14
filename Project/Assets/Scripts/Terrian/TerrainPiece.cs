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


        int terrainInstance = EgineUtils.GetInstanceId();
        float[,] _heightMap;
        public int instaneId
        {
            get
            {
                return terrainInstance;
            }
        }
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
            int heightMapWidth = (int)(Mathf.Pow(2, 2 * mapSize) + 1);
            int heightMapSize = heightMapWidth * heightMapWidth;
            if (mapSize > 4)
            {
                Debug.LogWarningFormat("with a size of {0},map take huge cost(with {1} vertices),pay attantion that if it is needed to continue do this.", mapSize, heightMapSize);
            }
            _heightMap = new float[heightMapWidth, heightMapWidth];
            _maxLod = maxLod;
            UnityCppBindings.RegistBinding(terrainInstance, this);
            RegisterTerrianMeshBindings(terrainInstance);
            int[] args = GetInitArgs(Random.Range(0, 5), maxLod, mapSize, 33, 100, 380, 500, 240, 700);

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
            return _heightMap.GetLength(0);
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
            Vector3 terrainSize = new Vector3(size, 1000, size); //地形的大小，最高、最低点的差值
            Debug.LogFormat("Terrain {0} Size {1}",terrainInstance, terrainSize);
            terrainData.size = terrainSize;
            terrainData.heightmapResolution = size + 1;
            /* float[,] hxy = new float[size, size];
             for (int x = 0; x < _heightMap.GetLength(0); x++)
             {
                 for (int y = 0; y < _heightMap.GetLength(1); y++)
                 {
                     //Debug.LogErrorFormat("height 0 {0}", _heightMap[x, y]);
                     hxy[x,y] = _heightMap[x, y] / 1000.0f;
                 }
             }*/
            terrainData.SetHeights(0, 0, _heightMap); //一切都是为了这个方法...
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
