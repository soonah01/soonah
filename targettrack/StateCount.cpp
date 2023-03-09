#include "stdafx.h"
#include "StateCount.h" 


StateCount::StateCount(void)
{
	m_totalcountnum = 0;
	m_calcnum = 100;//如果不传参数，默认为100
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

//清空状态
void StateCount::clear()
{
	m_totalcountnum = 0;
	m_startindex = 0;
	m_calcList.clear();
}
	
//设置参与统计计算的帧数
void StateCount::SetCalcNum(int calcnum)
{
	m_calcnum = calcnum;
}

//状态列表更新
void StateCount::updateState(int state)
{
	if(m_calcList.size() >= m_calcnum)//如果多了，就去替换以前的要过期的
	{
		m_calcList[m_startindex] = state;
		m_startindex++;
		//这样做就是比如只统计最近100帧，然后101帧过来，把之前的第一帧状态给替换，然后102帧过来，把之前的第二帧状态替换
		//这样列表内始终保持近期的100帧记录
		if (m_startindex >= m_calcnum)
		{
			m_startindex = 0;
		}
	}
	else
	{
		m_calcList.push_back(state); 
	}
	
	m_totalcountnum++;//总帧数加1
}

//获取比例值
double StateCount::getRate()
{
	int detectnum = 0;//检测到的个数
	for(int i=0;i<m_calcList.size();i++)
	{
		if (m_calcList[i] == 1)
		{
			detectnum++;
		}
	}
	double rate = double(detectnum)/double(m_calcnum);//计算所占比例
	return rate;
}