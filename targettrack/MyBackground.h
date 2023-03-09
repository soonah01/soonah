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
	void setCalcFrameNum(int calcFrameNum);//���ò�������֡��
	int  getTotalFrameNum();//��ȡ����������֡��

	void updateBackground(Mat frame);//����һ֡ͼ�񣬸���������
	void getBackgroundImage(Mat &bgframe);//�õ�����ͼ��

private:
	int m_calcFrameNum;//������㱳����֡��
	int m_totalFrameNum;//�ܹ�������֡��
	vector<Mat> m_calcFrameList;//��������ͼ���б�
	Mat m_bgframe;
};
