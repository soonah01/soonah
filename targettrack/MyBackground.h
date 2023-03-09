#pragma once

#include "opencv2\opencv.hpp"
#include <vector> 

using namespace  cv;
using namespace  std;

class MyBackground
{
public:
	MyBackground(void);
	~MyBackground(void);
	MyBackground(int calcFrameNum); 
	void setCalcFrameNum(int calcFrameNum);//设置参与计算的帧数
	int  getTotalFrameNum();//获取传进来的总帧数

	void updateBackground(Mat frame);//传来一帧图像，更新下数据
	void getBackgroundImage(Mat &bgframe);//得到背景图像

private:
	int m_calcFrameNum;//参与计算背景的帧数
	int m_totalFrameNum;//总共传来的帧数
	vector<Mat> m_calcFrameList;//参与计算的图像列表
	Mat m_bgframe;
};
