#include "stdafx.h"
#include "StateCount.h" 


StateCount::StateCount(void)
{
	m_totalcountnum = 0;
	m_calcnum = 100;//�������������Ĭ��Ϊ100
	m_startindex = 0;
}

StateCount::~StateCount(void) 
{
}


StateCount::StateCount(int calcnum)
{
	m_totalcountnum = 0;
	m_calcnum = calcnum;
	m_startindex = 0;
}

//���״̬
void StateCount::clear()
{
	m_totalcountnum = 0;
	m_startindex = 0;
	m_calcList.clear();
}
	
//���ò���ͳ�Ƽ����֡��
void StateCount::SetCalcNum(int calcnum)
{
	m_calcnum = calcnum;
}

//״̬�б����
void StateCount::updateState(int state)
{
	if(m_calcList.size() >= m_calcnum)//������ˣ���ȥ�滻��ǰ��Ҫ���ڵ�
	{
		m_calcList[m_startindex] = state;
		m_startindex++;
		//���������Ǳ���ֻͳ�����100֡��Ȼ��101֡��������֮ǰ�ĵ�һ֡״̬���滻��Ȼ��102֡��������֮ǰ�ĵڶ�֡״̬�滻
		//�����б���ʼ�ձ��ֽ��ڵ�100֡��¼
		if (m_startindex >= m_calcnum)
		{
			m_startindex = 0;
		}
	}
	else
	{
		m_calcList.push_back(state); 
	}
	
	m_totalcountnum++;//��֡����1
}

//��ȡ����ֵ
double StateCount::getRate()
{
	int detectnum = 0;//��⵽�ĸ���
	for(int i=0;i<m_calcList.size();i++)
	{
		if (m_calcList[i] == 1)
		{
			detectnum++;
		}
	}
	double rate = double(detectnum)/double(m_calcnum);//������ռ����
	return rate;
}