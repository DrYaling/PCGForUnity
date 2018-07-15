using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class test : MonoBehaviour {

	// Use this for initialization
	IEnumerator Start () {
        Bridge.Init();
        yield return new WaitForSeconds(3);
	}
	
	// Update is called once per frame
	void Update () {
        Bridge.CSUpdate((int)(Time.deltaTime * 1000));
	}
    private void OnApplicationQuit()
    {
        Debug.LogFormat("OnApplicationQuit");
        Bridge.Destroy();
    }
}
