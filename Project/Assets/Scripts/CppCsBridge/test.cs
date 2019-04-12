using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
public class test : MonoBehaviour
{
    //MeshGenerator mg = new MeshGenerator();
    SkyDram.Terrian terrian;
    // Use this for initialization
    IEnumerator Start()
    {
        yield return new WaitForSeconds(1);
        Bridge.Init();
        DateTime dt = DateTime.Now;
        terrian = new SkyDram.Terrian();
        terrian.Init();
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
            terrian.Release();
            terrian = null;
        }
        Bridge.Destroy();
        //mg.Destroy();
    }
}
