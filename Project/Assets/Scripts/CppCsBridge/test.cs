using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using UnityEngine.UI;
public class test : MonoBehaviour
{
    //MeshGenerator mg = new MeshGenerator();
    SkyDream.Terrain terrian;
    private Server _server;
    // Use this for initialization
    void Start()
    {
        Bridge.Init();
        _server = new Server();
        DateTime dt = DateTime.Now;
        terrian = new SkyDream.Terrain();
        terrian.Init();
        Application.targetFrameRate = 60;
    }

    // Update is called once per frame
    void Update()
    {
        Bridge.CSUpdate((int)(Time.deltaTime * 1000));
        if (null != terrian)
            terrian.Update((int)(Time.deltaTime * 1000));
    }
    private void OnApplicationQuit()
    {
        Debug.LogFormat("OnApplicationQuit");
        if (null != terrian)
        {
            terrian.Destroy();
            terrian = null;
        }
        if (null != _server)
        {
            _server.Destroy();
            _server = null;
        }
        Bridge.Destroy();
        //mg.Destroy();
    }
}
