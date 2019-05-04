using System;
using System.Collections.Generic;
#if UNITY_EDITOR
using UnityEditor;
#endif
namespace SkyDream
{
    class TerrianGenerator
    {
#if UNITY_EDITOR
        [MenuItem("Tools/CppLibs/Reload")]
        static void ReloadDLL()
        {
            UnityEditor.AssetDatabase.Refresh(ImportAssetOptions.ForceSynchronousImport);
        }
#endif
    }
}
