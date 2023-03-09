#include "StdAfx.h"
#include "MyBackground.h"

MyBackground::MyBackground(void)
{
	m_calcFrameNum = 25;
	m_totalFrameNum = 0;
}

MyBackground::~MyBackground(void) 
{

}


MyBackground::MyBackground(int calcFrameNum) 
{
	m_calcFrameNum = calcFrameNum;
	m_totalFrameNum = 0;
}

void MyBackground::setCalcFrameNum(int calcFrameNum)
{
	m_calcFrameNum = calcFrameNum;
}

//获取传进来的总帧数
int MyBackground::getTotalFrameNum()
{
	return m_totalFrameNum;//传进来的总帧数
}

//传来一帧图像，更新下数据
void MyBackground::updateBackground(Mat frame)
{
	Mat temp = frame.clone();
	m_calcFrameList.push_back(temp);//插入列表内

	if(m_calcFrameList.size() > m_calcFrameNum)//如果多了，就释放最开始的,这样列表内永远只有m_calcFrameNum个来计算均值
	{
		m_calcFrameList.erase(m_calcFrameList.begin());
	}

	m_totalFrameNum++;//总帧数加1

	if(m_calcFrameList.size() == 0)
	{
		return;
	}

	int framenum = m_calcFrameList.size();//此时列表内的帧数，不一定等于m_calcFrameNum
	Mat addimg;//所有的相加
	for (int i=0;i<framenum;i++)
	{
		Mat img = m_calcFrameList[i];//取出一帧图像
		Mat doubleimg;//为了数据计算，必须转为double类型，不然每个元素只是0-255整数不能有小数
		img.convertTo(doubleimg,CV_64F,1,0);//转为double类型
		if(i==0)
		{
			addimg =  doubleimg.clone();//第一次等于这个图像
		}
		else
		{
			addimg = addimg + doubleimg;//以后相加
		}
	}


	//int c = addimg.channels();
	//for (int i=0;i<addimg.rows;i++)
	//{
	//	for (int j=0;j<addimg.cols;j++)
	//	{
	//		Vec3d value = addimg.at<Vec3d>(i,j);//取出一个像素
	//		double d1 = value[0];
	//		double d2 = value[1];
	//		double d3 = value[2];
	//	}
	//}

	Mat meanimg = addimg/framenum;//均值图像
	meanimg.convertTo(m_bgframe,CV_8U,1,0);//转为char类型，普通类型
}

//得到背景图像
void MyBackground::getBackgroundImage(Mat &bgframe)
{
	
	bgframe = m_bgframe;
}