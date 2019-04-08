Shader "Custom/TerrianTiles"
{
	Properties
	{
		_heightMapTex("HeightMap", 2D) = "white" {}
		_grassTex("Grass", 2D) = "white" {}
		_grassLow("GrassLow",float) = 0
		_grassOptimize("GrassOptimize",float) = 0
		_grassHigh("GrassHigh",float) = 0
		_rockTex("Rock", 2D) = "white" {}
		_rockLow("RockLow",float) = 0
		_rockOptimize("RockOptimize",float) = 0
		_rockHigh("RockHigh",float) = 0
		_LightColor0("_LightColor0",Color) = (1,1,1,1)
	}
		SubShader
		{
			Tags { "LightModel" = "ForwardBase" }
			LOD 100

			Pass
			{
				CGPROGRAM
				#pragma vertex vert
				#pragma fragment frag
				// make fog work
				#pragma multi_compile_fog
				#pragma multi_compile_fwdbase 
				#include "UnityCG.cginc"
				#include "AutoLight.cginc"
				struct appdata
				{
					float4 vertex : POSITION;
					float2 uv : TEXCOORD0;
					float3 normal : NORMAL;
				};

				struct v2f
				{
					float2 uv : TEXCOORD0;
					//UNITY_FOG_COORDS(2)
					float3 normal : NORMAL;
					float4 pos : SV_POSITION;
					UNITY_SHADOW_COORDS(1)
				};
				sampler2D _heightMapTex;
				sampler2D _grassTex;
				sampler2D _rockTex;
				float4 _LightColor0;
				float _grassOptimize;
				float _grassLow;
				float _grassHigh;
				float _rockOptimize;
				float _rockLow;
				float _rockHigh;

				v2f vert(appdata v)
				{
					v2f o;
					o.uv = v.uv;
					//UNITY_TRANSFER_FOG(o, o.vertex);
					o.pos = UnityObjectToClipPos(v.vertex);
					o.normal = v.normal;
					TRANSFER_SHADOW(o);
					return o;
				}

				fixed4 frag(v2f i) : SV_Target
				{
					// sample the texture
					fixed4 col = fixed4(1,1,1,1);
					fixed4 colgrass = tex2D(_grassTex, i.uv);
					fixed4 colrock = tex2D(_rockTex, i.uv);
					fixed3 col1 = colgrass.rgb;
					fixed4 heightColor = tex2D(_heightMapTex, i.uv);
					if (heightColor.a > _grassLow && heightColor.a <= _grassOptimize)
					{
						col1 = (heightColor.a - _grassLow)*col1;
					}
					else if (heightColor.a < _grassHigh && heightColor.a > _grassOptimize)
					{
						col1 = (heightColor.a - _grassOptimize)*col1;
					}
					else
					{
						col1 = fixed3(0, 0, 0);
					}
					fixed3 col2 = colrock.rgb;
					if (heightColor.a > _rockLow && heightColor.a <= _rockOptimize)
					{
						col2 = (heightColor.a - _rockLow)*col2;
					}
					else if (heightColor.a < _grassHigh && heightColor.a > _grassOptimize)
					{
						col2 = (heightColor.a - _rockOptimize)*col2;
					}
					else
					{
						col2 = fixed3(0, 0, 0);
					}
					col = fixed4(col1 + col2, 1);
					fixed shadow = SHADOW_ATTENUATION(i);

					// 使用内置宏同时计算光照衰减和阴影。自动声明atten变量。
					UNITY_LIGHT_ATTENUATION(atten, i, i.pos);
					// 环境光
					fixed3 ambient = UNITY_LIGHTMODEL_AMBIENT.rgb;
					// 光照方向。
					fixed3 lightDir = normalize(_WorldSpaceLightPos0.xyz);; // 切线空间下的光照方向
					// 获取法线世界坐标系中的方向,并进行归一化处理
					fixed3 world_normal = UnityObjectToWorldNormal(i.normal);// normalize(mul(i.normal, (float3x3)unity_WorldToObject));
					// 兰伯特漫反射计算公式为:光照颜色和强度变量(_LightColor0) * 漫反射系数(_Diffuse) * 光源方向(_WorldSpaceLightPos0)与法线方向(normal)的非负值点积
					fixed3 diffuse = _LightColor0.rgb * col.rgb * saturate(dot(world_normal, lightDir));
					// 最终颜色 = 漫反射 + 环境光 
					fixed3 tempColor = diffuse + ambient * col; // 让环境光也跟纹理颜色做融合，防止环境光使得纹理效果看起来朦胧
					col = fixed4(tempColor*atten*shadow, 1);
					// apply fog
					UNITY_APPLY_FOG(i.fogCoord, col);
					return col;
				}
				ENDCG
			}
			//Additional Pass,渲染其他光源
					Pass
				{
					//指明光照模式为前向渲染模式
					Tags{ "LightMode" = "ForwardAdd" }

					//开启混合模式，将计算结果与之前的光照结果进行叠加
					Blend One One

					CGPROGRAM
#pragma vertex vert  
#pragma fragment frag  
					//确保光照衰减等光照变量可以被正确赋值
#pragma multi_compile_fwdadd

					//包含引用的内置文件  
#include "Lighting.cginc"  

					//声明properties中定义的属性  
				fixed4 _Diffuse;
				fixed4 _Specular;
				float _Gloss;

				//定义输入与输出的结构体  
				struct a2v
				{
					float4 vertex : POSITION;
					float3 normal : NORMAL;
				};

				struct v2f
				{
					float4 pos : SV_POSITION;
					//存储世界坐标下的法线方向和顶点坐标  
					float3 worldNormal : TEXCOORD0;
					float3 worldPos : TEXCOORD1;
				};

				//在顶点着色器中，计算世界坐标下的法线方向和顶点坐标，并传递给片元着色器  
				v2f vert(a2v v)
				{
					v2f o;
					//转换顶点坐标到裁剪空间  
					o.pos = UnityObjectToClipPos(v.vertex);
					//转换法线坐标到世界空间，直接使用_Object2World转换法线，不能保证转换后法线依然与模型垂直  
					o.worldNormal = mul(v.normal, (float3x3)unity_WorldToObject);
					//转换顶点坐标到世界空间  
					o.worldPos = mul(unity_ObjectToWorld, v.vertex).xyz;
					return o;
				}

				//在片元着色器中计算光照模型  
				fixed4 frag(v2f i) : SV_Target
				{
					fixed3 worldNormal = normalize(i.worldNormal);

				//计算不同的光源方向
#ifdef USING_DIRECTIONAL_LIGHT
				fixed3 worldLightDir = normalize(_WorldSpaceLightPos0.xyz);
#else
				fixed3 worldLightDir = normalize(_WorldSpaceLightPos0.xyz - i.worldPos.xyz);
#endif


				//计算漫反射光照  
				fixed3 diffuse = _LightColor0.rgb * _Diffuse.rgb * saturate(dot(worldNormal,worldLightDir));

				//获取视角方向 = 摄像机的世界坐标 - 顶点的世界坐标  
				fixed3 viewDir = normalize(_WorldSpaceCameraPos.xyz - i.worldPos.xyz);
				//计算新矢量h  
				fixed3 halfDir = normalize(viewDir + worldLightDir);
				//计算高光光照  
				fixed3 specular = _LightColor0.rgb * _Specular.rgb * pow(saturate(dot(worldNormal,halfDir)),_Gloss);

				//处理不同的光源衰减
#ifdef USING_DIRECTIONAL_LIGHT
				fixed atten = 1.0;
#else
				/*float3 lightCoord = mul(unity_WorldToLight, float4(i.worldPos, 1)).xyz;
				fixed atten = tex2D(_LightTexture0, dot(lightCoord, lightCoord).rr).UNITY_ATTEN_CHANNEL;*/
				float distance = length(_WorldSpaceLightPos0.xyz - i.worldPos.xyz);
				//线性衰减
				fixed atten = 1.0 / distance;
#endif

				return fixed4((diffuse + specular) * atten,1.0);

			}
				ENDCG
			}
		}
			Fallback"Specular"
}
