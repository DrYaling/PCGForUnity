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
            var mesh0 = new TerrianMesh(3, 1);
            var mesh1 = new TerrianMesh(3, 1);
            var mesh2 = new TerrianMesh(3, 1);
            var mesh3 = new TerrianMesh(3, 1);
            /**/
            /*
             *      mesh0   mesh1
             *      mesh2   mesh3
             */
            /**/
            m_mapMeshes.Add(mesh0.instaneId, mesh0);
            m_mapMeshes.Add(mesh1.instaneId, mesh1);
            m_mapMeshes.Add(mesh2.instaneId, mesh2);
            m_mapMeshes.Add(mesh3.instaneId, mesh3);
            GameObject go1 = new GameObject("first mesh");
            mesh0.SetMeshRoot(go1);
            mesh0.Loadsync();
            GameObject go2 = new GameObject("second mesh");
            go2.transform.position = go1.transform.position + new Vector3(1000, 0, 0);
            mesh1.SetNeighbor(mesh0, TerrianConst.neighborPositionLeft);
            mesh1.SetMeshRoot(go2);
            mesh1.Loadsync();
            GameObject go3 = new GameObject("third mesh");
            go3.transform.position = go1.transform.position + new Vector3(0, 0, -1000);
            mesh2.SetNeighbor(mesh0, TerrianConst.neighborPositionTop);
            mesh2.SetMeshRoot(go3);
            mesh2.Loadsync();

            GameObject go4 = new GameObject("fourth mesh");
            go4.transform.position = go1.transform.position + new Vector3(1000, 0, -1000);
            mesh3.SetNeighbor(mesh1, TerrianConst.neighborPositionTop);
            mesh3.SetNeighbor(mesh2, TerrianConst.neighborPositionLeft);
            mesh3.SetMeshRoot(go4);
            mesh3.Loadsync();


            mesh0.SetNeighbor(mesh1, TerrianConst.neighborPositionRight, true);
            mesh0.SetNeighbor(mesh2, TerrianConst.neighborPositionBottom, true);
            mesh1.SetNeighbor(mesh3, TerrianConst.neighborPositionBottom, true);
            mesh2.SetNeighbor(mesh3, TerrianConst.neighborPositionRight, true);
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