using System.Collections.Generic;
public static class UnityCppBindings
{
    private static Dictionary<uint, SkyDream.TerrainPiece> _terrainDic = new Dictionary<uint, SkyDream.TerrainPiece>();
    public static void RegistBinding(uint instanceId, SkyDream.TerrainPiece obj)
    {
        if (!_terrainDic.ContainsKey(instanceId))
        {
            _terrainDic.Add(instanceId, obj);
        }
        else
        {
            UnityEngine.Debug.LogErrorFormat("mesh {0} already binded", instanceId);
        }
    }
    public static void UnResistBinding(uint instanceId)
    {
        if (_terrainDic.ContainsKey(instanceId))
        {
            _terrainDic.Remove(instanceId);
        }
        else
        {
            //UnityEngine.Debug.LogErrorFormat("mesh {0} was not binded", instanceId);
        }
    }
    public static SkyDream.TerrainPiece GetTerrain(uint instanceId)
    {
        if (_terrainDic.ContainsKey(instanceId))
        {
            return _terrainDic[instanceId];
        }

        return null;
    }
    public static void Clear()
    {
        _terrainDic.Clear();
    }
    public static int size()
    {
        return _terrainDic.Count;
    }
}
