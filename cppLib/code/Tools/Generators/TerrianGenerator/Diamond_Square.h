#ifndef DIAMOND_SQUARE_H
#define DIAMOND_SQUARE_H
#include "../generator.h"
#include <vector>
NS_GNRT_START
/************************************************************************/
/*							菱形-正方形生成地形                         */
/*				点的序号为 x=0-x = max 为0-max,y轴向上递增				*/
/************************************************************************/
class Diamond_Square :public TerrianGenerator
{
public:
	Diamond_Square() {}
	Diamond_Square(int32_t seed, int32_t I, float H);
	virtual ~Diamond_Square();
	void SetProcessHandler(std::function<void(int32_t)> handler) { m_cbProcessHandler = handler; }
	void Start(const float* corner, const int32_t size = 4);
	//根据传入的最大坐标maxCoord计算地图
	void GenerateTerrian(std::vector<int32_t>* triangles, std::vector<G3D::Vector3>* v3, float maxCoord);
	bool IsFinished() { return m_bIsFinished; }
private:
	void WorkThread();
	inline void Diamond(int x, int y, int size, float h);
	inline void Square(int x, int y, int size, float h);
	inline float Randomize(float h);
	inline bool IsEdge(int x, int y) { return x == 0 || y == 0 || x == m_nI || y == m_nI; }
	//平滑边沿，使之可以和其他地图拼接
	void AddEdge(const G3D::Vector3* edge, int32_t size, int32_t coord/*x or y*/);
	inline void SetAtXY(int32_t x, int32_t y, float val) { m_vHeightMap[x + y * m_nSize] = val; }
	inline float GetAtXY(int x, int y) { return m_vHeightMap[x + y * m_nSize]; }
	inline size_t GetSize() { return m_vHeightMap.size(); }
private:
	std::function<void(int32_t)> m_cbProcessHandler;
	std::vector<float> m_vHeightMap;
	int32_t m_nSize;//总数 2^(2*i)+1
	float m_nH;//粗糙度
	int32_t	m_nI;//级数
	int32_t m_nMax;
	bool m_bIsFinished;
};

NS_GNRT_END
#endif
