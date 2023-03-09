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

//��ȡ����������֡��
int MyBackground::getTotalFrameNum()
{
	return m_totalFrameNum;//����������֡��
}

//����һ֡ͼ�񣬸���������
void MyBackground::updateBackground(Mat frame)
{
	Mat temp = frame.clone();
	m_calcFrameList.push_back(temp);//�����б���

	if(m_calcFrameList.size() > m_calcFrameNum)//������ˣ����ͷ��ʼ��,�����б�����Զֻ��m_calcFrameNum���������ֵ
	{
		m_calcFrameList.erase(m_calcFrameList.begin());
	}

	m_totalFrameNum++;//��֡����1

	if(m_calcFrameList.size() == 0)
	{
		return;
	}

	int framenum = m_calcFrameList.size();//��ʱ�б��ڵ�֡������һ������m_calcFrameNum
	Mat addimg;//���е����
	for (int i=0;i<framenum;i++)
	{
		Mat img = m_calcFrameList[i];//ȡ��һ֡ͼ��
		Mat doubleimg;//Ϊ�����ݼ��㣬����תΪdouble���ͣ���Ȼÿ��Ԫ��ֻ��0-255����������С��
		img.convertTo(doubleimg,CV_64F,1,0);//תΪdouble����
		if(i==0)
		{
			addimg =  doubleimg.clone();//��һ�ε������ͼ��
		}
		else
		{
			addimg = addimg + doubleimg;//�Ժ����
		}
	}


	//int c = addimg.channels();
	//for (int i=0;i<addimg.rows;i++)
	//{
	//	for (int j=0;j<addimg.cols;j++)
	//	{
	//		Vec3d value = addimg.at<Vec3d>(i,j);//ȡ��һ������
	//		double d1 = value[0];
	//		double d2 = value[1];
	//		double d3 = value[2];
	//	}
	//}

	Mat meanimg = addimg/framenum;//��ֵͼ��
	meanimg.convertTo(m_bgframe,CV_8U,1,0);//תΪchar���ͣ���ͨ����
}

//�õ�����ͼ��
void MyBackground::getBackgroundImage(Mat &bgframe)
{
	
	bgframe = m_bgframe;
}