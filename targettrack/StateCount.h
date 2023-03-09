#pragma once
#include <vector>

using namespace std;
class StateCount
{
public:
	StateCount(void);
	StateCount(int calcnum); 
	~StateCount(void);
	void updateState(int state);//״̬�б���� 
	double getRate();//��ȡ����ֵ
	void clear();//���״̬
	void SetCalcNum(int calcnum);//���ü����֡��
private:
	int m_calcnum;//Ҫ����ͳ�Ƶ���Ŀ
	int m_totalcountnum;//�ϼ�ͳ�Ƶ���Ŀ
	vector<int> m_calcList;//�������������б�
	int m_startindex;//����б����˺���ֵ�滻������ 
};
