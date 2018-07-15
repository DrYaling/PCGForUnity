using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

public class Bridge
{
    private const string dllName = "cppLibs";
    public delegate void CSUpdateCallback(int tick);
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
    public static void Init()
    {
        RegisterLog(CSLog);
        RegisterLogWarning(CSLogWarning);
        RegisterLogError(CSLogError);
        InitCppEngine();
        HandleSet(0, 666);
    }
    public static void Destroy()
    {
        DestroyCPP();
        //System.Runtime.InteropServices.
    }
    public static void CSUpdate(int time_diff)
    {
        UnityEngine.Profiling.Profiler.BeginSample("CppUpdate");
        //cost 4 ms (average) ms on my book
        Update(time_diff);
        UnityEngine.Profiling.Profiler.EndSample();
        UnityEngine.Profiling.Profiler.BeginSample("CSUpdate");
        //cost 95 ms (average) on my book
        long ret = 0;
        for (int i = 0; i < 10000000; i++)
        {
            ret += i;
        }
        CSLog("ret is " + ret);
        UnityEngine.Profiling.Profiler.EndSample();
    }
    private static void CSLog(string content)
    {
        UnityEngine.Debug.Log(content);
    }
    private static void CSLogWarning(string content)
    {
        UnityEngine.Debug.LogWarning(content);
    }
    private static void CSLogError(string content)
    {
        UnityEngine.Debug.LogError(content);
    }
}
