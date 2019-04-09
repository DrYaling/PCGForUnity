using System.Collections.Generic;
namespace SkyDram
{
    class MeshGroup
    {
        int _lod;
        List<TerrianMesh> _meshes;
        MeshGroup _leftNeighbor;
        MeshGroup _rightNeighbor;
        MeshGroup _bottomNeighbor;
        MeshGroup _topNeighbor;

    }
}