#ifndef DIAMOND_SQUARE_H
#define DIAMOND_SQUARE_H
#include "../generator.h"
#include <vector>
NS_GNRT_START
/************************************************************************/
/*							����-���������ɵ���                         */
/*				������Ϊ x=0-x = max Ϊ0-max,y�����ϵ���				*/
/************************************************************************/
class Diamond_Square :public TerrianGenerator
{
public:
	Diamond_Square() {}
	Diamond_Square(int32_t seed, int32_t I, float H);
	virtual ~Diamond_Square();
	void SetProcessHandler(std::function<void(int32_t)> handler) { m_cbProcessHandler = handler; }
	void Start(const float* corner, const int32_t size = 4);
	//���ݴ�����������maxCoord�����ͼ
	void GenerateTerrian(std::vector<int32_t>* triangles, std::vector<G3D::Vector3>* v3, std::vector<G3D::Vector3>* normal, float maxCoord);
	bool IsFinished() { return m_bIsFinished; }
private:
	void WorkThread();
	inline void Diamond(int x, int y, int size, float h);
	inline void Square(int x, int y, int size, float h);
	inline float Randomize(float h);
	inline bool IsEdge(int x, int y) { return x == 0 || y == 0 || x == m_nI || y == m_nI; }
	//ƽ�����أ�ʹ֮���Ժ�������ͼƴ��
	void AddEdge(const G3D::Vector3* edge, int32_t size, int32_t edgeType/*x or y*/);
	inline void SetAtXY(int32_t x, int32_t y, float val) { m_vHeightMap[x + y * m_nSize] = val; }
	inline float GetAtXY(int x, int y) { return m_vHeightMap[x + y * m_nSize]; }
	float GetExtendHeight(int x, int y)
	{
		if (x<0)
		{
			x = 0;
		}
		else if (x > m_nMax)
		{
			x = m_nMax;
		}
		if (y < 0)
		{
			y = 0;
		}
		else if (y > m_nMax)
		{
			y = m_nMax;
		}
		return GetAtXY(x, y);
	}
	inline size_t GetSize() { return m_vHeightMap.size(); }
	inline void	 SetExtendedPoint(int x, int y, float fx, float fy, float fz)/*x y from -1~m_nSize+1*/
	{
		m_vExtendPoints[x + 1 + (y + 1) * (m_nSize + 2)] = G3D::Vector3(fx, fy, fz);
	}
	inline const G3D::Vector3& GetExtendedPoint(int x, int y) const/*x y from -1~m_nSize+1*/
	{
		return m_vExtendPoints[x + 1 + (y + 1) * (m_nSize + 2)];
	}
private:
	std::function<void(int32_t)> m_cbProcessHandler;
	std::vector<float> m_vHeightMap;
	std::vector<G3D::Vector3> m_vExtendPoints;/*x = -1,y = -1,x = m_nSize,y = m_nSize*/
	float m_aPointBuffer[5];
	int32_t m_nSize;//���� 2^(2*i)+1
	float m_nH;//�ֲڶ�
	int32_t	m_nI;//����
	int32_t m_nMax;
	bool m_bIsFinished;
	bool m_bEdgeExtended;
};

NS_GNRT_END
#endif
