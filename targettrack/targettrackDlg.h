// targettrackDlg.h : 头文件
//

#pragma once
#include "opencv2\opencv.hpp"

#pragma comment(lib, "opencv_video249d.lib")
#pragma comment(lib, "opencv_core249d.lib") 
#pragma comment(lib, "opencv_highgui249d.lib")
#pragma comment(lib, "opencv_objdetect249d.lib")
#pragma comment(lib, "opencv_imgproc249d.lib")

using namespace cv;
using namespace std;

typedef struct PERSONINFO
{
	CString idno;//身份证号
	CString name;//姓名 
	CString address;//地址
	CString phone;//电话
	
}PERSONINFO,*PPERSONINFO;

// CtargettrackDlg 对话框
class CtargettrackDlg : public CDialog
{
// 构造
public:
	CtargettrackDlg(CWnd* pParent = NULL);	// 标准构造函数

	BOOL m_bKillThread;//是否关闭视频
	CWinThread *m_pThread;//线程句柄
	BOOL m_bPause;//当前是否暂停了
	
	int m_FrameCount;//当前播放的帧数
	int m_FrameAll;//视频总帧数
	CvCapture *m_capture;// 视频句柄
	
	int m_width;//帧宽度
	int m_height;//帧高度

	void show_pic(IplImage *t,int nID);//在控件中显示图片
	void ProcessForeground(Mat &frame,Mat &fgimg);

	Mat m_faceimg;//检测到的人脸
	CascadeClassifier cascade;//人脸检测器

	bool detectOneFace(Mat& img, Rect &facer);//人脸检测函数

	//提取样本特征库
	void mytrain();
	//识别
	int gettype(Mat faceimg);

		//数据库访问
	char m_szHost[20];//数据库地址
	char m_szUser[20];//用户名
	char m_szPwd[20];//密码
	char m_szDef[20];//连接数据库

	_ConnectionPtr m_Connection ;//数据库链接句柄

	BOOL InitDataBase();//初始链接数据库

	void ReadIni();

	void GetAllPerson();

	vector<PERSONINFO> m_personlist;//人员列表
// 对话框数据
	enum { IDD = IDD_TARGETTRACK_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedButtonPause();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedOk();
	afx_msg void OnClose();
};
