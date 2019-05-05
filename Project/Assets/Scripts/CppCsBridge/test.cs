using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using UnityEngine.UI;
public class test : MonoBehaviour
{
    //MeshGenerator mg = new MeshGenerator();
    SkyDream.Terrain terrian;
    [SerializeField]
    Button lod0;
    [SerializeField]
    Button lod1;
    [SerializeField]
    Button lod2;
    [SerializeField]
    Button lod3;
    bool[] lodBtnState = new bool[4] { false,false,false,false};
    private void Awake()
    {
        lod0.onClick.AddListener(OnClickLod0);
        lod1.onClick.AddListener(OnClickLod1);
        lod2.onClick.AddListener(OnClickLod2);
        lod3.onClick.AddListener(OnClickLod3);
        UnityEngine.TerrainData data = new TerrainData();
    }

    private void OnClickLod0()
    {
        lodBtnState[0] = !lodBtnState[0];
    }

    private void OnClickLod1()
    {
        lodBtnState[1] = !lodBtnState[1];
    }

    private void OnClickLod2()
    {
        lodBtnState[2] = !lodBtnState[2];
    }

    private void OnClickLod3()
    {
        lodBtnState[3] = !lodBtnState[3];
    }

    // Use this for initialization
    void Start()
    {
        Bridge.Init();
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
        Bridge.Destroy();
        //mg.Destroy();
    }
}
