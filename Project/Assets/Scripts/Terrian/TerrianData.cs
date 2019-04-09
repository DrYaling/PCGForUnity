using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
namespace SkyDram
{
    static class TerrianConst
    {
        public const int maxVerticesPerMesh = 65000;
        public const int meshTopologyVertice = 0;
        public const int meshTopologyTriangle = 1;
        public const int meshTopologyUV = 2;
    }
    [StructLayout(LayoutKind.Sequential)]
    class TerrianData
    {
#if UNITY_IOS
    private const string dllName = "__Internal";
#else
        private const string dllName = "cppLibs";
#endif
        [DllImport(dllName)]
        extern static void InitMeshTriangleData(int ins, [In,Out] int[] triangles, int size, int mesh, int lod);
        private bool _isReadable;
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
                return _isReadable;
            }
        }
        public int owner { get; private set; }
        private TerrianData() { }
        public TerrianData(int I, int mcount, int maxLod, bool uv, int owner)
        {
            this.owner = owner;
            _useUV = uv;
            _meshCount = mcount;
            _lodCount = maxLod;
            _vertices = new Vector3[mcount][];
            _normals = new Vector3[mcount][];
            _triangles = new int[mcount][][];
            if (uv)
            {
                _uvs = new Vector2[mcount][][];
                for (int i = 0; i < mcount; i++)
                    _uvs[i] = new Vector2[4][];
            }
            for (int i = 0; i < mcount; i++)
            {
                _triangles[i] = new int[maxLod][];
            }
            int maxSize = (int)(Mathf.Pow(I, 2) + 1);
            int m_nMax = maxSize - 1;
            int startY = 0;
            int idx = 0;
            int outBoundY;
            int obY;
            outBoundY = obY = m_nMax / meshCount;
            //int32_t*** triangles;
            while (idx < meshCount)
            {
                //cb(meshTopologyVertice, idx, 0, );
                int nsize = maxSize * (outBoundY - startY + 1);
                _vertices[idx] = new Vector3[nsize];
                _normals[idx] = new Vector3[nsize];
                if (useUV)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        _uvs[idx][i] = new Vector2[nsize];
                    }
                }
                startY = outBoundY;//因为最上面和最右边一排不计算三角形，所以在交界处需要多计算一次
                outBoundY += obY;
                if (outBoundY > m_nMax)
                    outBoundY = m_nMax;
                idx++;
            }
        }
        private void InitVertices(int meshIndex, int verticesCount)
        {
            if (meshIndex >= 0 && meshIndex < _vertices.Length)
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

        public void InitLod(int mesh, int lod, int triangleSize)
        {
            if (lod < 0 || lod >= _lodCount)
            {
                throw new Exception("invalid init lod");
            }
            if (_triangles[mesh][lod].Length != triangleSize)
            {
                _triangles[mesh][lod] = new int[triangleSize];
                InitMeshTriangleData(owner, _triangles[mesh][lod], _triangles[mesh][lod].Length, mesh, lod);
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
        public void ResizeIndices(int type, int mesh, int lod, int size)
        {
            if (mesh < 0 || mesh >= meshCount)
            {
                Debug.LogErrorFormat("ResizeIndices fail,invalid mesh count {0},max {1}", mesh, meshCount);
                return;
            }
            switch (type)
            {
                case TerrianConst.meshTopologyTriangle:
                    InitLod(mesh, lod, size);
                    break;
                case TerrianConst.meshTopologyVertice:
                    InitVertices(mesh, size);
                    break;
            }
        }
    }
}