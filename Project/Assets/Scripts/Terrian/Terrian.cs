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
        public const float lod_1_Distance = 30f;/*meters*/
        public const float lod_2_Distance = 60f;/*meters*/
        public const float lod_3_Distance = 90f;/*meters*/

    }
    class Terrian
    {
        Dictionary<int, SkyDram.TerrianMesh> m_mapMeshes = new Dictionary<int, TerrianMesh>();
        public void Init()
        {
            var mesh = new TerrianMesh(3, 1);
            m_mapMeshes.Add(0,mesh);
            mesh.Loadsync();
        }
        public void Update(int time_diff)
        {
            foreach(var mesh in m_mapMeshes)
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