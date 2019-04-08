using System.Collections.Generic;
using UnityEngine;
namespace SkyDram
{
    class TerrianMesh
    {
        int meshInstanceId = EgineUtils.GetInstanceId();
        bool _loadFinish = false;
        public bool isLoaded
        {
            get
            {
                return _loadFinish;
            }
        }
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
        TerrianData _terrianData;
        TerrianMesh() { }
        public TerrianMesh(int verticeCount, int maxLod)
        {
            _terrianData = new TerrianData(verticeCount, maxLod);
            UnityCppBindings.ReisterBinding(meshInstanceId, this);
        }
        public void InitLodInfo(int lod, int triangleCount)
        {
            _terrianData.InitLod(lod, triangleCount);
        }
        public void LoadAsync()
        {
            //TODO load terrian mesh triangles and so on
        }
        private void Load()
        {
            _loadFinish = true;
        }
        public void Update(int time_diff)
        {

        }
    }
}