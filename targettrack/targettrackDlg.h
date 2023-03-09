// targettrackDlg.h : ͷ�ļ�
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
	CString idno;//���֤��
	CString name;//���� 
	CString address;//��ַ
	CString phone;//�绰
	
}PERSONINFO,*PPERSONINFO;

// CtargettrackDlg �Ի���
class CtargettrackDlg : public CDialog
{
// ����
public:
	CtargettrackDlg(CWnd* pParent = NULL);	// ��׼���캯��

	BOOL m_bKillThread;//�Ƿ�ر���Ƶ
	CWinThread *m_pThread;//�߳̾��
	BOOL m_bPause;//��ǰ�Ƿ���ͣ��
	
	int m_FrameCount;//��ǰ���ŵ�֡��
	int m_FrameAll;//��Ƶ��֡��
	CvCapture *m_capture;// ��Ƶ���
	
	int m_width;//֡���
	int m_height;//֡�߶�

	void show_pic(IplImage *t,int nID);//�ڿؼ�����ʾͼƬ
	void ProcessForeground(Mat &frame,Mat &fgimg);

	Mat m_faceimg;//��⵽������
	CascadeClassifier cascade;//���������

	bool detectOneFace(Mat& img, Rect &facer);//������⺯��

	//��ȡ����������
	void mytrain();
	//ʶ��
	int gettype(Mat faceimg);

		//���ݿ����
	char m_szHost[20];//���ݿ��ַ
	char m_szUser[20];//�û���
	char m_szPwd[20];//����
	char m_szDef[20];//�������ݿ�

	_ConnectionPtr m_Connection ;//���ݿ����Ӿ��

	BOOL InitDataBase();//��ʼ�������ݿ�

	void ReadIni();

	void GetAllPerson();

	vector<PERSONINFO> m_personlist;//��Ա�б�
// �Ի�������
	enum { IDD = IDD_TARGETTRACK_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
