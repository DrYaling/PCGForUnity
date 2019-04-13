using System.Collections.Generic;
using UnityEngine;
using System.Threading;
namespace SkyDram
{
    static class TerrainConst
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
        Dictionary<int, SkyDram.TerrainPiece> m_mapTerrains = new Dictionary<int, TerrainPiece>();
        public void Init()
        {
            Thread t = new Thread(new ThreadStart(() => {
            }));
            int size = 5;
            int lod = 3;
            var terrain0 = new TerrainPiece(size, lod);
            var terrain1 = new TerrainPiece(size, lod);
            var terrain2 = new TerrainPiece(size, lod);
            var terrain3 = new TerrainPiece(size, lod);
            /**/
            /*
             *      mesh0   mesh1
             *      mesh2   mesh3
             */
            /**/
            m_mapTerrains.Add(terrain0.instaneId, terrain0);
            m_mapTerrains.Add(terrain1.instaneId, terrain1);
            m_mapTerrains.Add(terrain2.instaneId, terrain2);
            m_mapTerrains.Add(terrain3.instaneId, terrain3);
            terrain0.Load();

            terrain1.SetNeighbor(terrain0, TerrainConst.neighborPositionLeft);
            terrain1.Load();
            terrain1.SetPosition(new Vector3(100, 0, 0));

            terrain2.SetNeighbor(terrain0, TerrainConst.neighborPositionTop);
            terrain2.Load();
            terrain2.SetPosition(new Vector3(0, 0, -100));

            terrain3.SetNeighbor(terrain1, TerrainConst.neighborPositionTop);
            terrain3.SetNeighbor(terrain2, TerrainConst.neighborPositionLeft);
            terrain3.Load();
            terrain3.SetPosition(new Vector3(100, 0, -100));

            terrain0.SetNeighbor(terrain1, TerrainConst.neighborPositionRight);
            terrain0.SetNeighbor(terrain2, TerrainConst.neighborPositionBottom);
            terrain1.SetNeighbor(terrain3, TerrainConst.neighborPositionBottom);
            terrain2.SetNeighbor(terrain3, TerrainConst.neighborPositionRight);
        }
        public void Update(int time_diff)
        {
            foreach (var mesh in m_mapTerrains)
            {
                mesh.Value.Update(time_diff);
            }
        }
        public void Release()
        {
            /*foreach (var mesh in m_mapMeshes)
            {
                mesh.Value.Release();
            }*/
            m_mapTerrains.Clear();
        }
    }
}