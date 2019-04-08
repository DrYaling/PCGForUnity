using System;
using System.Collections.Generic;
using UnityEngine;
namespace SkyDram
{
    class TerrianData
    {
        public int verticeCount
        {
            get
            {
                if (null != _vertices)
                    return _vertices.Length;
                else
                    return 0;
            }
        }
        Vector3[] _vertices;
        Vector3[] vertices { get { return _vertices; } }
        //TODO 这里不仅仅是包含lod个数组,还有防缝隙优化的数组
        //实际情况是  当lod为0的时候,会有多个数组,每个数组的数据也不一致,比如
        //1.左边是lod1,这个时候左边边界会扣掉一些顶点,其它边也会存在这个情况
        //2.同理,当自身是lod1的时候,四个边沿可能会是lod2,又会有多个情况
        //3.所以这个三角形数组不是一个固定的数组,是动态变化的(固定数组会是比较大的一个数组
        //是一个组合数)
        int[][] _triangles;

        private TerrianData() { }
        public TerrianData(int verticeCount, int maxLod)
        {
            _vertices = new Vector3[verticeCount];
            _triangles = new int[maxLod][];
        }
        public void InitLod(int lod, int triangleSize)
        {
            if (null == _triangles || lod <0)
            {
                throw new Exception("invalid init lod");
            }
            if(_triangles.Length > lod)
            {
                _triangles[lod] = new int[triangleSize];
            }
        }
        public int[] GetTriangles(int lod)
        {
            if (null == _triangles || lod < 0 || lod >= _triangles.Length)
            {
                throw new Exception("invalid init lod");
            }
            return _triangles[lod];
        }
    }
}