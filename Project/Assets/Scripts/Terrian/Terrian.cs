using System.Collections.Generic;
using UnityEngine;
namespace SkyDram
{
    class Terrian
    {
        Dictionary<int, SkyDram.TerrianMesh> m_mapMeshes = new Dictionary<int, TerrianMesh>();
        public void Init()
        {
            var mesh = new TerrianMesh(3, 1);
            m_mapMeshes.Add(0,mesh);
            mesh.LoadAsync();
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