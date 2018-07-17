using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
using System.IO;
using AOT;

public class Bridge
{
    private const string dllName = "cppLibs";
#if UNITY_EDITOR
    // pointer handle to the C++ DLL
    private static IntPtr libarayHandle;

#if UNITY_EDITOR_OSX
    const string LIB_PATH = "/Plugins/cppLibs.bundle/Contents/MacOS/cppLibs";
#elif UNITY_EDITOR_LINUX
    const string LIB_PATH = "/cppLibs.so";
#elif UNITY_EDITOR_WIN
    const string LIB_PATH = "/Plugins/x86_x64/cppLibs.dll";
#endif
#if UNITY_EDITOR_OSX || UNITY_EDITOR_LINUX
    //OSX 和Linux下的导入
    [DLLImport("__Internal")]
    public static extern IntPtr dlopen(string path, int flag);
    [DllImport("__Internal")]
    public static extern IntPtr dlsym(IntPtr handle, string symbolName);
    [DllImport("__Internal")]
    public static extern int dlclose(IntPtr handle);

    public static IntPtr OpenLibrary(string path)
    {
        IntPtr handle = dlopen(path, 0);
        if(handle == IntPtr.Zero)
        {
           throw new Exception("Couldn't open native library: "+ path);
        }
        return handle;
    }
    
    public static void CloseLibrary(IntPtr libraryHandle)
    {
         dlclose(libraryHandle);
    }
    
#elif UNITY_EDITOR_WIN
    // win 编辑器下
    [DllImport("kernel32")]
    public static extern IntPtr LoadLibrary(string path);

    [DllImport("kernel32")]
    public static extern IntPtr GetProcAddress(IntPtr libraryHandle,
    string symbolName);

    [DllImport("kernel32")]
    public static extern bool FreeLibrary(IntPtr libraryHandle);

    public static IntPtr OpenLibrary(string path)
    {
        IntPtr handle = LoadLibrary(path);
        if (handle == IntPtr.Zero)
        {
            throw new Exception("Couldn't open native library: " + path);
        }
        return handle;
    }

    public static void CloseLibrary(IntPtr libraryHandle)
    {
        FreeLibrary(libraryHandle);
    }
#endif
#endif
    public delegate void Log(string content);
    public delegate void LogWarning(string content);
    public delegate void LogError(string content);
    [DllImport(dllName)]
    extern static void InitCppEngine();
    [DllImport(dllName)]
    extern static void HandleSet(int key, int val);
    [DllImport(dllName)]
    extern static void Update(int time_diff);
    [DllImport(dllName)]
    extern static void RegisterLog(Log callback);
    [DllImport(dllName)]
    extern static void RegisterLogWarning(LogWarning callback);
    [DllImport(dllName)]
    extern static void RegisterLogError(LogError callback);
    [DllImport(dllName)]
    extern static void DestroyCPP();
    [DllImport(dllName)]
    extern static void SetObjtest(vector3 v);
    [DllImport(dllName)]
    extern static void SetIntTest(int input);
    [DllImport(dllName)]
    extern static int GetIntTest();
    [DllImport(dllName)]
    extern static vector3 GetObjTest();
    public static void Init()
    {
#if UNITY_EDITOR
        libarayHandle = OpenLibrary(Application.dataPath+LIB_PATH);
#endif
        RegisterLog(CSLog);
        RegisterLogWarning(CSLogWarning);
        RegisterLogError(CSLogError);
        InitCppEngine();
        HandleSet(0, 666);
        int time_diff = 1111;
        int count = 100000;
        UnityEngine.Profiling.Profiler.BeginSample("CppUpdate");
        for (int i = 0; i < count; i++)
        {
            Update(time_diff);
        }
        UnityEngine.Profiling.Profiler.EndSample();
        UnityEngine.Profiling.Profiler.BeginSample("CSUpdate");
        for (int i = 0; i < count; i++)
        {
            CSUpdateTest(time_diff);
        }
        UnityEngine.Profiling.Profiler.EndSample();
        UnityEngine.Profiling.Profiler.BeginSample("CSGetInt");
        for (int i = 0; i < count; i++)
        {
            CSGetIntTest();
        }
        UnityEngine.Profiling.Profiler.EndSample();
        UnityEngine.Profiling.Profiler.BeginSample("CPPGetInt");
        for (int i = 0; i < count; i++)
        {
            GetIntTest();
        }
        UnityEngine.Profiling.Profiler.EndSample();
        UnityEngine.Profiling.Profiler.BeginSample("CSGetObj");
        for (int i = 0; i < count; i++)
        {
            CSGetObjTest();
        }
        UnityEngine.Profiling.Profiler.EndSample();
        UnityEngine.Profiling.Profiler.BeginSample("CPPGetObj");
        for (int i = 0; i < count; i++)
        {
            //GetObjTest();
        }
        UnityEngine.Profiling.Profiler.EndSample();
        UnityEngine.Profiling.Profiler.BeginSample("CSSetInt");
        for (int i = 0; i < count; i++)
        {
            CSSetIntTest(111);
        }
        UnityEngine.Profiling.Profiler.EndSample();
        UnityEngine.Profiling.Profiler.BeginSample("CPPSetInt");
        for (int i = 0; i < count; i++)
        {
            SetIntTest(111);
        }
        UnityEngine.Profiling.Profiler.EndSample();
        UnityEngine.Profiling.Profiler.BeginSample("CSSetObj");
        for (int i = 0; i < 1; i++)
        {
            CSSetObjtest(v);
        }
        UnityEngine.Profiling.Profiler.EndSample();
        UnityEngine.Profiling.Profiler.BeginSample("CPPSetObj");
        for (int i = 0; i < 1; i++)
        {
            SetObjtest(v);
        }
        UnityEngine.Profiling.Profiler.EndSample();
    }
    public static void Destroy()
    {
        DestroyCPP();
#if UNITY_EDITOR
        CloseLibrary(libarayHandle);
        libarayHandle = IntPtr.Zero;
        //UnityEditor.AssetDatabase.ImportAsset("Assets"+LIB_PATH, UnityEditor.ImportAssetOptions.ForceSynchronousImport);
#endif
        //System.Runtime.InteropServices.
    }
    public static void CSUpdate(int time_diff)
    {
        UnityEngine.Profiling.Profiler.BeginSample("CppUpdate");
        Update(time_diff);
        UnityEngine.Profiling.Profiler.EndSample();
    }
    static void CSUpdateTest(int time_diff)
    {
    }
    [MonoPInvokeCallback(typeof(Log))]
    private static void CSLog(string content)
    {
        UnityEngine.Debug.Log(content);
    }
    [MonoPInvokeCallback(typeof(LogWarning))]
    private static void CSLogWarning(string content)
    {
        UnityEngine.Debug.LogWarning(content);
    }
    [MonoPInvokeCallback(typeof(LogError))]
    private static void CSLogError(string content)
    {
        UnityEngine.Debug.LogError(content);
    }
    [StructLayout(LayoutKind.Sequential)]
    class vector3
    {
        public int x, y, z;
    };
    static vector3 v = new vector3() { x = 1, y = 2, z = 3 };
    static void CSSetIntTest(int inut)
    {

    }
    static void CSSetObjtest(vector3 v)
    {

    }
    static int CSGetIntTest()
    {
        return 0;
    }
    static vector3 CSGetObjTest()
    {
        return v;
    }
}
