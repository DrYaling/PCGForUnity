using System.Collections.Generic;
using UnityEngine;
namespace SkyDram
{
    static class TerrianConst
    {
        public const int maxVerticesPerMesh = 65000;
        public const int meshTopologyVertice = 0;
        public const int meshTopologyTriangle = 1;
        public const int meshTopologyUV = 2;
        public const int meshTopologyMeshCount = 3;
        public const int meshTopologyNormal = 4;
        public const float lod_1_Distance = 30f;/*meters*/
        public const float lod_2_Distance = 60f;/*meters*/
        public const float lod_3_Distance = 90f;/*meters*/
        public const int neighborPositionLeft = 0;
        public const int neighborPositionBottom = 1;
        public const int neighborPositionRight = 2;
        public const int neighborPositionTop = 3;
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
    class Terrian
    {
        Dictionary<int, SkyDram.TerrianMesh> m_mapMeshes = new Dictionary<int, TerrianMesh>();
        public void Init()
        {
            var mesh0 = new TerrianMesh(1, 1);
            var mesh1 = new TerrianMesh(1, 1);
            m_mapMeshes.Add(0, mesh0);
            GameObject go1 = new GameObject("first mesh");
            mesh0.SetMeshRoot(go1);
            mesh0.Loadsync();
            m_mapMeshes.Add(1, mesh1);
            GameObject go2 = new GameObject("second mesh");
            mesh1.SetNeighbor(mesh0, TerrianConst.neighborPositionLeft);
            mesh1.SetMeshRoot(go2);
            mesh1.Loadsync();
            mesh0.SetNeighbor(mesh1, TerrianConst.neighborPositionRight, true);
            go2.transform.position = go1.transform.position + new Vector3(1000, 0, 0);
        }
        public void Update(int time_diff)
        {
            foreach (var mesh in m_mapMeshes)
            {
                mesh.Value.Update(time_diff);
            }
        }
        public void Release()
        {
            foreach (var mesh in m_mapMeshes)
            {
                mesh.Value.Release();
            }
            m_mapMeshes.Clear();
        }
    }
}