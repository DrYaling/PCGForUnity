using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
class MeshGenerator
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    struct v3
    {
        public float x;
        public float y;
        public float z;
    }
#if UNITY_IOS
    private const string dllName = "__Internal";
#else
    private const string dllName = "cppLibs";
#endif
    [DllImport(dllName)]
    extern static void ReleaseMeshGenerator();
    [DllImport(dllName)]
    extern static void InitMeshGenerator(int seed, [In]int[] args, int argSize, bool optimalize);
    [DllImport(dllName)]
    extern static void GenerateMesh(int type, [In, Out] int[] v3Size);
    [DllImport(dllName)]
    extern static void GetMeshData([In, Out]Vector3[] v3List, [In, Out]int[] pIdx, int maxV3);
    [DllImport(dllName)]
    extern static int GetMeshVerticesData([In, Out]Vector3[] v3List, [In, Out]Vector3[] normalList, int maxV3);
    [DllImport(dllName)]
    extern static void GetMeshTrianglesData([In, Out]int[] pIdx, int maxV3);
    int _seed;
    int _size;
    public void Init(int seed, int size, bool usePerlin)
    {
        _seed = seed;
        _size = size;
        //InitMeshGenerator(2, 4, 1000/*mm*/, 100000,new Vector3(100,50,100), false);
        int[] conor = new int[] { 4/*I*/,40/*rough *100*/, 10000/*map coord size*/, 50, 2500, 5000, 850 };
        InitMeshGenerator(0, conor, conor.Length, false);
    }
    public Mesh[] Generate()
    {
        int[] vsize = new int[25];
        int meshCount = 0;
        GenerateMesh(1, vsize);
        for (int sizeIdx = 0; sizeIdx < 25; sizeIdx++)
        {
            if (vsize[sizeIdx] > 0)
                meshCount++;
            else
                break; 
        }
        Mesh[] mesh = null;
        if (meshCount > 0)
            mesh = new Mesh[meshCount];
        for (int sizeIdx = 0; sizeIdx < meshCount; sizeIdx++)
        {
            int sizeV = vsize[sizeIdx];
            Vector3[] vertexes = new Vector3[sizeV];
            Vector3[] normalList = new Vector3[sizeV];
            int sizeI = GetMeshVerticesData(vertexes,normalList, sizeIdx);
            Debug.LogFormat("sizeV {0},sizeI {1}", sizeV, sizeI);
            if (sizeI > 0)
            {
                int[] indexes = new int[sizeI];
                for (int i = 0; i < sizeV; i++)
                {
                    //Debug.LogFormat(" {0},y {1}", i, vertexes[i]);
                    vertexes[i] /= 100f;
                    /*GameObject go = new GameObject(string.Format(" {0},y {1}", i, vertexes[i]));
                    go.transform.position = vertexes[i];*/
                }
                GetMeshTrianglesData(indexes, sizeIdx);  
                mesh[sizeIdx] = new Mesh();
                mesh[sizeIdx].vertices = vertexes;
                mesh[sizeIdx].triangles = indexes;
                mesh[sizeIdx].normals = normalList;
            }
        }
        return mesh;
    }
    public void Destroy()
    {
        ReleaseMeshGenerator();
    }
}