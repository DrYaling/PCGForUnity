using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
public class test : MonoBehaviour
{
    MeshGenerator mg = new MeshGenerator();
    // Use this for initialization
    IEnumerator Start()
    {
        yield return new WaitForSeconds(1);
        Bridge.Init();
        DateTime dt = DateTime.Now;
        long tick0 = dt.Ticks;
        mg.Init(1, 4, true);
        long tick1 = dt.Ticks;
        var mesh = mg.Generate();
        if (null != mesh)
        {
            for (int i = 0; i < mesh.Length; i++)
            {
                if (null == mesh[i])
                    continue;
                GameObject go = new GameObject();
                go.AddComponent<MeshFilter>().mesh = mesh[i];
                var mat = Resources.Load<Material>("Terrian");
                int texSize = UnityEngine.Mathf.FloorToInt((float)Math.Sqrt(mesh[i].vertexCount));
               /* Texture2D heightMap = new Texture2D(texSize, texSize, TextureFormat.Alpha8, true);
                Color[] colors = new Color[mesh[i].vertexCount];
                for (int idx = 0; idx < mesh[i].vertexCount; idx++)
                {
                    colors[idx] = new Color(1, 1, 1, mesh[i].vertices[idx].y / 10f);
                }
                heightMap.SetPixels(colors);
                heightMap.EncodeToPNG();
                heightMap.Apply();
                GameObject obj = new GameObject("spriet");

                var sr = obj.AddComponent<SpriteRenderer>();

                Sprite pic = Sprite.Create(heightMap, new Rect(0, 0, texSize, texSize), new Vector2(0.5f, 0.5f));
                sr.sprite = pic;

                mat.SetTexture("HeightMap", heightMap);*/
                go.AddComponent<MeshRenderer>().material = mat;
                tick1 = dt.Ticks;
                Debug.LogFormat("GameObject cost time {0} milleseconds", (tick1 - tick0) / TimeSpan.TicksPerMillisecond);
                dt = DateTime.Now;
                //重置法线
                //mesh[i].RecalculateNormals();
                tick1 = dt.Ticks;
                Debug.LogFormat("RecalculateNormals cost time {0} milleseconds", (tick1 - tick0) / TimeSpan.TicksPerMillisecond);
                //重置范围
                //mesh[i].RecalculateBounds();
            }
        }
        tick1 = dt.Ticks;
        Debug.LogFormat("total cost time {0} milleseconds", (tick1 - tick0) / TimeSpan.TicksPerMillisecond);
    }

    // Update is called once per frame
    void Update()
    {
        Bridge.CSUpdate((int)(Time.deltaTime * 1000));
    }
    private void OnApplicationQuit()
    {
        Debug.LogFormat("OnApplicationQuit");
        Bridge.Destroy();
        mg.Destroy();
        mg = null;
    }
}
