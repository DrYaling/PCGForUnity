using System.Collections.Generic;
public static class UnityCppBindings
{
    static Dictionary<int, SkyDram.TerrianMesh> _meshDic = new Dictionary<int, SkyDram.TerrianMesh>();
    public static void RegistBinding(int instanceId, SkyDram.TerrianMesh obj)
    {
        if (!_meshDic.ContainsKey(instanceId))
            _meshDic.Add(instanceId, obj);
        else
            UnityEngine.Debug.LogErrorFormat("mesh {0} already binded", instanceId);
    }
    public static void UnResistBinding(int instanceId)
    {
        if (_meshDic.ContainsKey(instanceId))
        {
            _meshDic.Remove(instanceId);
        }
        else
        {
            UnityEngine.Debug.LogErrorFormat("mesh {0} was not binded", instanceId);
        }
    }
    public static SkyDram.TerrianMesh GetMesh(int instanceId)
    {
        if (_meshDic.ContainsKey(instanceId))
            return _meshDic[instanceId];
        return null;
    }
}
