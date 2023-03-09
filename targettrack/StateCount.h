#pragma once
#include <vector>

using namespace std;
class StateCount
{
public:
	StateCount(void);
	StateCount(int calcnum); 
	~StateCount(void);
	void updateState(int state);//状态列表更新 
	double getRate();//获取比例值
	void clear();//清空状态
	void SetCalcNum(int calcnum);//设置计算的帧数
private:
	int m_calcnum;//要真正统计的数目
	int m_totalcountnum;//合计统计的数目
	vector<int> m_calcList;//参与计算的数据列表
	int m_startindex;//如果列表满了后，新值替换的索引 
};
