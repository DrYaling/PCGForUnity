using System.Runtime.InteropServices;
using UnityEngine;
using UnityEditor;

public class EngineEditorInitilizer : ScriptableObject
{
#if UNITY_IOS    
    private const string dllName = "__Internal";
#else
    private const string dllName = "cppLibs";
#endif
    [DllImport(dllName)]
    static extern void EngineEntryInitilize();

    [InitializeOnLoadMethod]
    static void EngineEditorInitilize()
    {
        //EngineEntryInitilize();
    }
}