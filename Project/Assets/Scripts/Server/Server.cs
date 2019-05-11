using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

public class Server
{
#if UNITY_IOS    
    private const string dllName = "__Internal";
#else
    private const string dllName = "cppLibs";
#endif
    [DllImport(dllName)]
    static extern void StartUpServer(string ip,int port);
    [DllImport(dllName)]
    static extern void StopServer();
    [DllImport(dllName)]
    static extern void StartUpClientTest();

    public Server()
    {
        StartUpServer("127.0.0.1",8081);
        StartUpClientTest();
    }

    public void Destroy()
    {
        StopServer();
    }
}
