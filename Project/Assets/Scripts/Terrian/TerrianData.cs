using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
namespace SkyDram
{
    [StructLayout(LayoutKind.Sequential)]
    class TerrianData
    {
#if UNITY_IOS
    private const string dllName = "__Internal";
#else
        private const string dllName = "cppLibs";
#endif
        [DllImport(dllName)]
        extern static void GetMeshTriangleData(int ins, [In, Out] int[] triangles, int size, int mesh, int lod);
        [DllImport(dllName)]
        extern static void GetMeshVerticeData(int ins, [In, Out] Vector3[] vertices, [In, Out] Vector3[] normals, int size, int mesh);
        [DllImport(dllName)]
        extern static void ReloadMeshNormalData(int ins, [In, Out] Vector3[] normals, int size, int mesh, int meshEdgePosition);
        [DllImport(dllName)]
        extern static void GetMeshUVData(int ins, [In, Out] Vector2[] uvs, int size, int mesh, int uv);
        [DllImport(dllName)]
        extern static void NeighborLodHasChanged(int ins, int neighbor);
        /// <summary>
        /// reset lod 
        /// </summary>
        /// <param name="instanceId"></param>
        /// <param name="lod"></param>
        [DllImport(dllName)]
        extern static void ResetLod(int instanceId, int lod);


        private bool _uvReadable;
        private bool _verticesReadable;
        private bool _trianglesReadable;
        private bool _useUV;
        private int _meshCount;
        private int _lodCount;
        Vector3[][] _vertices;
        Vector3[][] _normals;
        Vector2[][][] _uvs;
        //TODO 这里不仅仅是包含lod个数组,还有防缝隙优化的数组
        //实际情况是  当lod为0的时候,会有多个数组,每个数组的数据也不一致,比如
        //1.左边是lod1,这个时候左边边界会扣掉一些顶点,其它边也会存在这个情况
        //2.同理,当自身是lod1的时候,四个边沿可能会是lod2,又会有多个情况
        //3.所以这个三角形数组不是一个固定的数组,是动态变化的(固定数组会是比较大的一个数组
        //是一个组合数)
        int[][][] _triangles;

        public int meshCount
        {
            get
            {
                return _meshCount;
            }
        }
        public int lodCount
        {
            get
            {
                return _lodCount;
            }
        }
        public bool useUV
        {
            get
            {
                return _useUV;
            }
        }
        public bool readable
        {
            get
            {
                if (!useUV)
                    return _verticesReadable && _trianglesReadable;
                else
                    return _verticesReadable && _uvReadable && _trianglesReadable;
            }
        }
        public int owner { get; private set; }
        private TerrianData() { }
        public TerrianData(int I, int maxLod, bool uv, int ownerId)
        {
            _verticesReadable = false;
            _uvReadable = false;
            _trianglesReadable = false;
            owner = ownerId;
            _useUV = uv;
            _lodCount = maxLod;
        }
        private void InitVertices(int meshIndex, int verticesCount)
        {
            Debug.LogFormat("InitVertices mesh {0},size {1}", meshIndex, verticesCount);
            if (meshIndex >= 0 && meshIndex < meshCount)
            {
                _vertices[meshIndex] = new Vector3[verticesCount];
                _normals[meshIndex] = new Vector3[verticesCount];
                if (useUV)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        _uvs[meshIndex][i] = new Vector2[verticesCount];
                    }
                }
            }
            else
                Debug.LogErrorFormat("mesh vertices index out of bound {0}", meshIndex);
        }
        public Vector3[] GetVertices(int meshIndex)
        {
            if (meshIndex >= 0 && meshIndex < meshCount)
                return _vertices[meshIndex];
            else
            {
                Debug.LogErrorFormat("mesh vertices index out of bound {0}", meshIndex);
                return null;
            }
        }
        public Vector3[] GetNormal(int meshIndex)
        {
            if (meshIndex >= 0 && meshIndex < meshCount)
                return _normals[meshIndex];
            else
            {
                Debug.LogErrorFormat("mesh normal index out of bound {0}", meshIndex);
                return null;
            }
        }
        public Vector2[] GetUV(int meshIndex, int uvChannel)
        {
            if (uvChannel < 0 || uvChannel >= 4)
            {
                Debug.LogErrorFormat("invalid uvchannel {0}(4)", uvChannel);
                return null;
            }
            if (meshIndex >= 0 && meshIndex < meshCount)
                return _uvs[meshIndex][uvChannel];
            else
            {
                Debug.LogErrorFormat("mesh uv index out of bound {0}", meshIndex);
                return null;
            }
        }
        public void SetLod(int lod)
        {
            ResetLod(owner, lod);

        }
        private void InitLod(int mesh, int lod, int triangleSize)
        {
            //Debug.LogFormat("InitLod mesh {0},lod {1},size {2}", mesh, lod, triangleSize);
            if (lod < 0 || lod >= _lodCount)
            {
                Debug.LogFormat("Init Lod fail");
                return;
            }
            if (null == _triangles[mesh][lod] || _triangles[mesh][lod].Length != triangleSize)
            {
                _triangles[mesh][lod] = new int[triangleSize];
                //GetMeshTriangleData(owner, _triangles, _triangles.Length, mesh, lod);
            }

        }
        public int[] GetTriangles(int mesh, int lod)
        {
            if (mesh < 0 || mesh >= meshCount)
            {
                Debug.LogErrorFormat("GetTriangles fail,invalid mesh count {0},max {1}", mesh);
                return null;
            }
            if (null == _triangles || lod < 0 || lod >= _lodCount)
            {
                throw new Exception("invalid init lod");
            }
            return _triangles[mesh][lod];
        }
        public void MeshInitilizer(int type, int mesh, int lod, int size)
        {
            //Debug.LogFormat("MeshInitilizer type {0} mesh {1},lod {2},size {3}", type, mesh, lod, size);
            switch (type)
            {
                case TerrianConst.meshTopologyVertice:
                    InitVertices(mesh, size);
                    break;
                case TerrianConst.meshTopologyUV:
                    break;
                case TerrianConst.meshTopologyTriangle:
                    InitLod(mesh, lod, size);
                    break;
                case TerrianConst.meshTopologyMeshCount:
                    InitMeshInfo(mesh);
                    break;
                default:
                    break;
            }
        }

        private void InitMeshInfo(int mcount)
        {
            _meshCount = mcount;
            _vertices = new Vector3[mcount][];
            _normals = new Vector3[mcount][];
            _triangles = new int[mcount][][];
            if (useUV)
            {
                _uvs = new Vector2[mcount][][];
                for (int i = 0; i < mcount; i++)
                    _uvs[i] = new Vector2[4][];
            }
            for (int i = 0; i < mcount; i++)
            {
                _triangles[i] = new int[lodCount][];
            }
        }

        public void GeneratorNotifier(int type, int arg0, int arg1)
        {
            //Debug.LogFormat("GeneratorNotifier type {0} arg0 {1},arg1 {2}", type, arg0, arg1);
            switch (type)
            {
                case TerrianConst.meshTopologyVertice:
                    LoadVertices(arg0);
                    _verticesReadable = true;
                    break;
                case TerrianConst.meshTopologyNormal:
                    LoadNormal(arg0, arg1);
                    break;
                case TerrianConst.meshTopologyUV:
                    _uvReadable = true;
                    break;
                case TerrianConst.meshTopologyTriangle:
                    LoadTriangle(arg0, arg1);
                    _trianglesReadable = true;
                    break;
                default:
                    break;
            }
        }
        private void LoadVertices(int mesh)
        {
            GetMeshVerticeData(owner, _vertices[mesh], _normals[mesh], _vertices[mesh].Length, mesh);

        }
        private void LoadNormal(int mesh, int meshEdgePosition)
        {
            try
            {
                ReloadMeshNormalData(owner, _normals[mesh], _normals[mesh].Length, mesh, meshEdgePosition);
            }
            catch (System.Exception ex)
            {
                Debug.LogException(ex);
            }
        }
        private void LoadTriangle(int mesh, int lod)
        {
            GetMeshTriangleData(owner, _triangles[mesh][lod], _triangles[mesh][lod].Length, mesh, lod);
        }
        public void OnNeighborLODChanged(int neighbor)
        {
            NeighborLodHasChanged(owner, neighbor);
        }
    }
}