using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
namespace SkyDram
{
    public class TerrianMesh
    {
#if UNITY_IOS
    private const string dllName = "__Internal";
#else
        private const string dllName = "cppLibs";
#endif
        public delegate void MeshInitilizer(int target, int type, int mesh, int lod, int size);
        public delegate void GeneratorNotifier(int target, int type, int arg0, int arg1);

        [DllImport(dllName)]
        extern static void RegisterTerrianMeshBindings(int ins);
        [DllImport(dllName)]
        extern static void InitTerrianMesh(int ins, [In]int[] args, int argsize, MeshInitilizer cb, GeneratorNotifier notifier);
        [DllImport(dllName)]
        extern static void SetMeshNeighbor(int ins, int neighbor, int dir, bool reloadNormalIfLoaded);
        [DllImport(dllName)]
        extern static void StartGenerateOrLoad(int ins);
        [DllImport(dllName)]
        extern static void ReleaseMeshGenerator(int ins);
        [DllImport(dllName)]
        extern static void FlushMeshGenerator(int ins);


        int meshInstanceId = EgineUtils.GetInstanceId();
        bool _initilized = false;
        private Vector3 _meshMiddle = Vector3.zero;
        public bool isLoaded { get; private set; }
        Camera _camera;
        Camera camera
        {
            get
            {
                return _camera;
            }
            set
            {
                _camera = value;
            }
        }
        public int instaneId
        {
            get
            {
                return meshInstanceId;
            }
        }
        GameObject _gameObject;
        List<Mesh> _meshes = new List<Mesh>();
        TerrianData _terrianData;
        TerrianMesh _neighborLeft;
        TerrianMesh _neighborRight;
        TerrianMesh _neighborBottom;
        TerrianMesh _neighborTop;
        bool _terrianDataReadOnly = false;
        TerrianMesh() { }
        public TerrianMesh(int mapSize, int maxLod = 3, bool useUV = false)
        {
            Debug.LogFormat("mesh map size {0} ,maxLod {1}", mapSize, maxLod);
            if (mapSize > 4)
            {
                Debug.LogWarningFormat("with a size of {0},map take huge cost(with {1} vertices),pay attantion that if it is needed to continue do this.", mapSize,(int)Mathf.Pow((Mathf.Pow(2,2*mapSize)+1),2));
            }
            _terrianData = new TerrianData(mapSize, maxLod, useUV, meshInstanceId);
            UnityCppBindings.RegistBinding(meshInstanceId, this);
            RegisterTerrianMeshBindings(meshInstanceId);
            int[] args = GetInitArgs(UnityEngine.Random.Range(0, 5), maxLod, mapSize, 33, 100, 80, 100, 40, 200);
            InitTerrianMesh(meshInstanceId, args, args.Length, MeshInitilize, MeshNotifier);
            //StartGenerateOrLoad(meshInstanceId);

        }
        ~TerrianMesh()
        {
            Release();
        }
        private static void MeshInitilize(int target, int type, int mesh, int lod, int size)
        {
            TerrianMesh tmesh = UnityCppBindings.GetMesh(target);
            if (null != tmesh)
            {
                tmesh._terrianData.MeshInitilizer(type, mesh, lod, size);
            }
            else
            {
                Debug.LogErrorFormat("mesh target {0} does not exist", target);
            }
        }
        private static void MeshNotifier(int target, int type, int arg0, int arg1)
        {
            TerrianMesh tmesh = UnityCppBindings.GetMesh(target);
            if (null != tmesh)
            {
                tmesh._terrianData.GeneratorNotifier(type, arg0, arg1);
            }
            else
            {
                Debug.LogErrorFormat("mesh target {0} does not exist", target);
            }
        }
        private int[] GetInitArgs(int seed, int maxLod, int I, int H, int mapWidth, int h0, int h1, int h2, int h3, bool useuv = false)
        {
            return new int[] { seed, maxLod, I, H, mapWidth, h0, h1, h2, h3, useuv ? 1 : 0 };
        }
        public void SetMeshRoot(GameObject root)
        {
            _gameObject = root;
        }
        public void Loadsync()
        {
            //TODO load terrian mesh triangles and so on
            //_terrianDataReadOnly = true;
            StartGenerateOrLoad(meshInstanceId);
            FlushMeshGenerator(meshInstanceId);
        }
        /// <summary>
        /// if neighbor was not loaded,this method got no use ,
        /// if this mesh was loaded,then reloadNormalIfLoaded = true
        /// </summary>
        /// <param name="neighbor"></param>
        /// <param name="position"></param>
        /// <param name="reloadNormalIfLoaded"></param>
        public void SetNeighbor(TerrianMesh neighbor, int position, bool reloadNormalIfLoaded = false)
        {
            switch (position)
            {
                case TerrianConst.neighborPositionLeft:
                    _neighborLeft = neighbor;
                    break;
                case TerrianConst.neighborPositionRight:
                    _neighborRight = neighbor;
                    break;
                case TerrianConst.neighborPositionBottom:
                    _neighborBottom = neighbor;
                    break;
                case TerrianConst.neighborPositionTop:
                    _neighborTop = neighbor;
                    break;
            }
            SetMeshNeighbor(meshInstanceId, neighbor.meshInstanceId, position, reloadNormalIfLoaded);
        }
        private void SetLOD(int lod)
        {
            if (lod >= 0 && lod < _terrianData.lodCount)
            {
                _terrianData.SetLod(lod);
                if (null != _neighborLeft)
                    _neighborLeft.OnNeighborLODChanged(this);
                if (null != _neighborRight)
                    _neighborRight.OnNeighborLODChanged(this);
                if (null != _neighborBottom)
                    _neighborBottom.OnNeighborLODChanged(this);
                if (null != _neighborTop)
                    _neighborTop.OnNeighborLODChanged(this);
            }
        }
        protected void OnNeighborLODChanged(TerrianMesh neighbor)
        {
            _terrianData.OnNeighborLODChanged(neighbor.instaneId);
        }
        private void Load()
        {
            isLoaded = true;
            _terrianDataReadOnly = false;
        }
        void OnLoadFinish(int lod)
        {
            if (_terrianDataReadOnly)
                return;
            Debug.LogFormat("Load TerrianMesh {0}", meshInstanceId);
            for (int i = 0; i < _terrianData.meshCount; i++)
            {
                Mesh mesh = new Mesh();
                _meshes.Add(mesh);
                mesh.vertices = _terrianData.GetVertices(i);
                mesh.normals = _terrianData.GetNormal(i);
                mesh.triangles = _terrianData.GetTriangles(i, lod);
                if (_terrianData.useUV)
                {
                    mesh.uv = _terrianData.GetUV(i, 0);
                    mesh.uv2 = _terrianData.GetUV(i, 1);
                    mesh.uv3 = _terrianData.GetUV(i, 2);
                    mesh.uv4 = _terrianData.GetUV(i, 3);
                }
                GameObject go = new GameObject("mesh" + i);
                go.AddComponent<MeshFilter>().mesh = mesh;
                go.AddComponent<MeshRenderer>().material = Resources.Load<Material>("Terrian");
                if (null != _gameObject)
                    go.transform.SetParent(_gameObject.transform, false);
            }
        }
        public void Update(int time_diff)
        {
            if (isLoaded && !_initilized)
            {
                if (_terrianDataReadOnly)
                    return;
                _initilized = true;
                OnLoadFinish(_terrianData.lodCount - 1);
            }
            else if (_terrianData.readable)
            {
                isLoaded = true;
            }
        }
        internal void Release()
        {
            Debug.LogFormat("Release mesh {0}",instaneId);
            ReleaseMeshGenerator(meshInstanceId);
            UnityCppBindings.UnResistBinding(meshInstanceId);
            meshInstanceId = 0;
        }

    }
}