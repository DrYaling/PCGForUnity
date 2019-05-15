using System.Collections.Generic;
public static class UnityCppBindings
{
    private static List<float[,]> _heightMapCache = new List<float[,]>();
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
        _heightMapCache.Clear();
    }
    public static int size()
    {
        return _terrainDic.Count;
    }

    public static void Init(int cacheCount)
    {
        for (int i = 0; i < cacheCount; i++)
        {
            _heightMapCache.Add(new float[65, 65]);
        }
    }

    public static float[,] GetHeightMapCache()
    {
        if (_heightMapCache.Count > 0)
        {
            var r = _heightMapCache[0];
            _heightMapCache.RemoveAt(0);
            return r;
        }
        return new float[65, 65];
    }

    public static void RecycleCache(float[,] heightMap)
    {
        _heightMapCache.Add(heightMap);
    }
}
