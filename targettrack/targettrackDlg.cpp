// targettrackDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "targettrack.h"
#include "targettrackDlg.h"
#include "MyBackground.h"
#include "StateCount.h"
#include <io.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//�쳣��������
const char* strtypenames[] = {"����","����","��Ծ","ͣ��"};
const char* strtypenamesEng[] = {"Squat","Lie","Jump","Stop"};
const int typenum = 4; /* */

const int weights[]={30,100,50,10};//�⼸���Ȩ��ֵ


int bgframenum = 30;//������㱳����֡���������һֱ���㣬���Խ�������ΪС�ڵ���0����
int bwthresholdvalue = 22;//��ֵ����ֵ
double areathresholdvalue = 0.04;//Ŀ�������ֵ���ڻ�������ռ����

int statecalcnum = 30;//����ͳ�Ƶ������֡��
StateCount state(statecalcnum);//���ø�״̬ͳ�ƶ�����Щ��̬��Ҫ����N֡����ȷ����

vector<Point2f> tracklist;//�켣 �����ж��Ƿ�ͣ�£�����

int stopT = 45;//ͣ����ֵ������֡��
double stopdistT = 12;//פ��������ֵ������˵�λ�Ʋ��� 

//�����ж��Ƿ���Ծ
int jumpT = 100;//��Ծ��ֵ��Ŀ�����֡���ﵽ����ż�����Ծ
double bottomT = 0;//ƽ���ײ�
double meanyT = 0;//ƽ�����ĸ߶�

int disappearnum = 0;//������ʧ֡��
int totalweight = 0;//�ۺ�Ȩ��
bool bhaveobject = false;//�Ƿ���Ŀ�����

int oldtype = -1;//��һ֡��״̬
int weightT = 150;//Ȩ����ֵ

int maxnum = 1000;//������һ�ּ������֡��

//��������������
double compute_dist(Point2f pt1,Point2f pt2)
{
	double dist = 0;
	//dist = (pt1.x - pt2.x)*(pt1.x - pt2.x) + (pt1.y - pt2.y)*(pt1.y - pt2.y);
	dist = (pt1.x - pt2.x)*(pt1.x - pt2.x);
	dist = sqrt(dist);
	return dist;
}

//��������ж��Ƿ�פ��
bool isstop(vector<Point2f> &points,int T = stopT)
{
	int ptnum = points.size();//��ĸ���
	if(ptnum < T)
	{
		return false;//��������������פ��
	}

	int startindex = ptnum-T;//ǰN֡�����

	double dist = compute_dist(points[startindex],points[ptnum-1]);//�������

	if(dist <= stopdistT)
	{
		return true;//פ��
	}
	else
	{
		return false;
	}
}

//��������ж��Ƿ���Ծ
bool isjump(Rect r)
{

	if(tracklist.size() < jumpT)
	{
		return false;
	}
	//���ĸ߶�
	double centery = double(r.y) + double(r.height)*0.5;

	double buttom = double(r.y) + double(r.height);

	//��Ծ�� ��������  ��������ж�
	if( (centery < meanyT-15) && (buttom < bottomT-15) )
	{
		return true;
	}
	else
	{
		return false;
	}

}

//ѵ����������������
Mat trainlbpfeatures;
Mat trainLabel; //ѵ�������ͱ�ǩ

//58�ֶ�����ͻ�����С�ڵ���2����
const int LBP_NUM[]={0,1,2,3,4,6,7,8,12,14,15,16,24,28,30,31,32,48,56,60,62,63,64,96,112,120,124,126,127,128,129,131,135,
143,159,191,192,193,195,199,207,223,224,225,227,231,239,240,241,243,247,248,249,251,252,253,254,255};
//���ⲻ������58������һ�࣬�ܹ�59�࣬Ҳ����˵lbpֱ��ͼ������59ά��
uchar maptable[256] = {0};

//����ӳ���256ά�ȵ�ӳ�䵽59ά��
void CreateMap(uchar maptable[])
{
	int i;
	for (i=0;i<256;i++)
	{
		maptable[i] = 58;//�ȳ�ʼ������58
	}

	for (i=0;i<58;i++)
	{
		int index = LBP_NUM[i];//�õ���Ӧ��λ��
		maptable[index] = i;//��Ӧλ�ø�ֵ
	}
}


//�����������lbp��ͼ������LBP����
Mat COMPUTE_LBP(Mat image,float result[],uchar maptable[])
{
	
	Mat lbpimg = image.clone();
	uchar center=0;//ĳ���ԭʼ����ֵ
	uchar center_lbp=0;//Ҫת����lbpֵ
	int row,col;
	//�������ص㣬���ص���ܱߵ�8�����űȽϣ����������
	for (int row=1; row<image.rows-1; row++)
	{
		for (int col=1; col<image.cols-1; col++)
		{
			center = image.at<uchar>(row,col);//����ֵ
			center_lbp = 0;

			uchar tmpvalue =  image.at<uchar>(row-1,col-1);//����
			if(center <= tmpvalue)
			{
				center_lbp += 128;
			}

			tmpvalue =  image.at<uchar>(row-1,col);//����
			if(center <= tmpvalue)
			{
				center_lbp += 64;
			}


			tmpvalue =  image.at<uchar>(row-1,col+1);//����
			if(center <= tmpvalue)
			{
				center_lbp += 32;
			}

			tmpvalue =  image.at<uchar>(row,col+1);//����
			if(center <= tmpvalue)
			{
				center_lbp += 16;
			}

			tmpvalue =  image.at<uchar>(row+1,col+1);//����
			if(center <= tmpvalue)
			{
				center_lbp += 8;
			}

			tmpvalue =  image.at<uchar>(row+1,col);//����
			if(center <= tmpvalue)
			{
				center_lbp += 4;
			}

			tmpvalue =  image.at<uchar>(row+1,col-1);//����
			if(center <= tmpvalue)
			{
				center_lbp += 2;
			}

			tmpvalue =  image.at<uchar>(row,col-1);//����
			if(center <= tmpvalue)
			{
				center_lbp += 1;
			}

			lbpimg.at<uchar>(row,col) = center_lbp;//lbpֵ
			uchar dimvalue = maptable[center_lbp];//�ӱ���תΪ0-58ֵ
			result[dimvalue] = result[dimvalue]+1;//��1ͳ��
		}
	}
	//�������ϣ��õ���lbpimg����lbp��ͼ
	//Ȼ��ֱ��ͼ������һ��
	int i;
	for (i=0;i<59;i++)
	{
		result[i] = result[i]/((image.rows-2)*(image.cols-2));
	}


	return lbpimg;//ͬʱ����lbp��ͼ ���ڷ�����
}

//�����ཻϵ������
float mysimilar(Mat f1,Mat f2)
{
	int dim1 = f1.cols;
	int dim2 = f2.cols;

	float similar = 0;
	for(int i=0;i<dim1;i++)
	{
		float h1 = f1.at<float>(i);
		float h2 = f2.at<float>(i);
		similar = similar+min(h1,h2);
	}

	return similar;
}


//IplImage ת BMP
LPBITMAPINFO  CreateMapInfo(IplImage* workImg)    //  ����λͼ��Ϣ
{                                           
	BITMAPINFOHEADER BIH={40,1,1,1,8,0,0,0,0,0,0};
	LPBITMAPINFO lpBmi;
	int          wid, hei, bits, colors,i;
	RGBQUAD  ColorTab[256];
	wid =workImg->width;     hei =workImg->height;
	bits=workImg->depth*workImg->nChannels;
	if (bits>8) colors=0;
	else colors=1<<bits;
	lpBmi=(LPBITMAPINFO) malloc(40+4*colors);
	BIH.biWidth   =wid;     
	BIH.biHeight  =hei;
	BIH.biBitCount=(BYTE) bits;
	memcpy(lpBmi,&BIH,40);                   //  ����λͼ��Ϣͷ
	if (bits==8) {                           //  256 ɫλͼ
		for (i=0;i<256;i++)  {                //  ���ûҽ׵�ɫ��
			ColorTab[i].rgbRed=ColorTab[i].rgbGreen=ColorTab[i].rgbBlue=(BYTE) i;
		}
		memcpy(lpBmi->bmiColors, ColorTab, 1024);
	}
	return(lpBmi);
}

//��ȡ�������·��·��
CString GetFilePath()
{
	char exepath[MAX_PATH];
	CString strdir,tmpdir;
	memset(exepath,0,MAX_PATH);
	GetModuleFileName(NULL,exepath,MAX_PATH);
	tmpdir=exepath;
	strdir=tmpdir.Left(tmpdir.ReverseFind('\\'));
	return strdir;
} 

//���������������ļ����ڵ�����ͼƬ�ļ� bmp jpg png
void getPicFiles(string path, vector<string>& files)
{
	//�ļ����
	long  long hFile   =   0;
	//�ļ���Ϣ
	struct _finddata_t fileinfo;
	string p;
	if((hFile = _findfirst(p.assign(path).append("\\*").c_str(),&fileinfo)) !=  -1)
	{
		do
		{
			//�����Ŀ¼,����֮
			//�������,�����б�
			if((fileinfo.attrib &  _A_SUBDIR))
			{
				if(strcmp(fileinfo.name,".") != 0  &&  strcmp(fileinfo.name,"..") != 0)
					getPicFiles(path+"\\"+fileinfo.name, files);
			}
			else
			{

				char *pp;
				pp = strrchr(fileinfo.name,'.');//���������ֵ�λ��
				if (_stricmp(pp,".bmp")==0 || _stricmp(pp,".jpg")==0 || _stricmp(pp,".png")==0 )//����ҵ�����ͼƬ���д���
				{
					files.push_back(p.assign(path).append("\\").append(fileinfo.name));//����洢��·�����ļ�ȫ��
				}
			}
		}while(_findnext(hFile, &fileinfo)  == 0);
		_findclose(hFile);
	}
}



//����IplImage
int imageClone(IplImage* pi,IplImage** ppo)  
{
	if (*ppo) {
		cvReleaseImage(ppo);                //  �ͷ�ԭ��λͼ
	}
	(*ppo) = cvCloneImage(pi);              //  ������λͼ
	return(1);
}


//frame ��ǰ֡
//bgimg ����ͼ��
//fgimg �ڰ�ǰ��ͼ  �������֡�� �õ�ǰ��
void GetForeground(Mat &frame,Mat &bgimg,Mat &fgimg)
{
	Mat framegray;//ԭͼ�ĻҶ�ͼ
	Mat bgimggray;//����ͼ�ĻҶ�ͼ

	cvtColor(frame, framegray, CV_BGR2GRAY);  //תΪ�Ҷ�ͼ��
	cvtColor(bgimg, bgimggray, CV_BGR2GRAY);  //תΪ�Ҷ�ͼ��

	absdiff(bgimggray,framegray,fgimg);//�ͱ��������
	imshow("�������ͼ",fgimg);

	threshold(fgimg,fgimg,bwthresholdvalue,255,THRESH_BINARY);  //��ֵ��

}

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CtargettrackDlg �Ի���




CtargettrackDlg::CtargettrackDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CtargettrackDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CtargettrackDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CtargettrackDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_PLAY, &CtargettrackDlg::OnBnClickedButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CtargettrackDlg::OnBnClickedButtonPause)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CtargettrackDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDOK, &CtargettrackDlg::OnBnClickedOk)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CtargettrackDlg ��Ϣ�������

BOOL CtargettrackDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	//�߳���Ϣ��ʼ��
	m_bKillThread = FALSE;
	m_bPause = FALSE;
	m_pThread = NULL;
	m_FrameCount = 0;
	m_FrameAll = 0;

	CString strfilename = GetFilePath()+ "\\haarcascade_frontalface_alt2.xml";//xml
	 // װ�ض�ȡ���������
	if( !cascade.load( strfilename.GetBuffer(0) ) )
	{
        cerr << "ERROR: Could not load classifier cascade" << endl;
        exit(0) ;
    }

		ReadIni();//��ȡ�����ļ�
	BOOL bret = InitDataBase();//�������ݿ�
	if (!bret)
	{
		MessageBox( "���ݿ����ʧ��,�����쳣�ر�!", "������", MB_OK) ;
		exit(1) ;
	}

	CreateMap(maptable);//����lbpӳ���

	GetAllPerson();//��ȡ������Ա��Ϣ
	mytrain();//��ȡ����������


	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CtargettrackDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CtargettrackDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ��������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
//
HCURSOR CtargettrackDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//�̺߳���
unsigned int ThreadFunction( void* param )
{
	IplImage *image;

	//	TagParam *m_tagparam = (TagParam*)param;
	CtargettrackDlg *pdlg = (CtargettrackDlg *)param;//ȡ��������ָ��
	CvCapture *capture = pdlg->m_capture;//�õ���Ƶָ��
	if (!capture)
	{
		return 0;
	}
	int framenum = 0;//֡����
	BackgroundSubtractorMOG2 bgSubtractor(20,60,true); //��˹����������
	MyBackground mybgSubtractor;//�Լ�д�ı���������

	Mat frame,bgimg,fgimg;//�ֱ��ǵ�ǰ֡ ���� ǰ��

	state.clear();//״̬���
	disappearnum = 0;//������ʧ֡��
	tracklist.clear();//���
	bottomT = 0;//��0
	meanyT = 0;
	totalweight = 0;
	bhaveobject = false;
	oldtype = -1;

	while( (pdlg->m_bKillThread) == false )
	{
		image = cvQueryFrame(pdlg->m_capture);//��ȡһ֡
		if(image == NULL)//ȡ���������ˣ�˵��������ϣ��˳�ѭ��
		{
			break;
		}
		//cvResize(image,framesize);//���ڴ�С

		frame = cvarrToMat(image);//תΪmat
		//���ͼ���������Ҫ�Լ���ͼ����С��
		int maxHW = MAX(frame.rows,frame.cols);//���ͼ�����߳�
		int maxlength = 800;//Ҫת�������߳�
		Mat imgsize;
		if(maxHW > maxlength)
		{
			double ratio = double(maxlength)/double(maxHW);//Ҫ��С�ı���
			resize(frame, imgsize, cv::Size(0, 0), ratio,ratio);//��С��ͼ��
		}
		else
		{
			imgsize = frame;
		}
		frame = imgsize;//��ǰ֡

		//�Լ�д�ľ�ֵ������ʽ
		if(bgframenum <= 0 )
		{
			mybgSubtractor.updateBackground(frame);//ˢ�±���
			
		}
		else if(framenum <= bgframenum)
		{
			mybgSubtractor.updateBackground(frame);//ˢ�±���
		}
		mybgSubtractor.getBackgroundImage(bgimg);//ȡ��ģ���еı���

	
		//�õ������󣬼���õ�ǰ����
		GetForeground(frame,bgimg,fgimg);//����ڰ�ǰ��ͼ
		//
		
		pdlg->ProcessForeground(frame,fgimg);//����ǰ��,�õ���һ���Ƿ�ˤ��
		

		IplImage imgshow = fgimg;
		IplImage imgshow2 = frame;
		pdlg->show_pic(&imgshow2,IDC_STATIC_PIC1);
		pdlg->show_pic(&imgshow,IDC_STATIC_PIC2);
	
		framenum++;//֡������1
		pdlg->m_FrameCount = framenum;
		CString str;
		str.Format("���Ž���:%d/%d֡",pdlg->m_FrameCount,pdlg->m_FrameAll);
		pdlg->SetDlgItemText(IDC_STATIC_STATE,str);
		
		if(pdlg->m_bPause)
		{
			str.Format("���Ž���:��ͣ%d/%d֡",pdlg->m_FrameCount,pdlg->m_FrameAll);
			pdlg->SetDlgItemText(IDC_STATIC_STATE,str);
		}
		while(pdlg->m_bPause)
		{
			
			Sleep(30);//�����ͣ�ˣ������⿨ס�ȴ�
		}
		
		imshow("����",bgimg);
		cvWaitKey(25);
	}

	cvReleaseCapture(&capture);
	pdlg->m_capture = NULL;
	pdlg->m_bKillThread = TRUE;
	pdlg->m_pThread = NULL;
	pdlg->m_bPause = FALSE;
	CString str;
	str.Format("���Ž��ȣ�δ����");
	pdlg->SetDlgItemText(IDC_STATIC_STATE,str);
	return( 0 );
}

void CtargettrackDlg::OnBnClickedButtonPlay()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	if(m_pThread)
	{
		return;//����Ѿ��򿪲����ˣ�ֱ�ӷ���
	}
	
	//����һ���Ի���ѡ���ļ�
	char szFilter[] = "video Files (*.avi;*.mp4)|*.avi;*.mp4|All Files (*.*)|*.*||";
	CFileDialog dlg(true, "avi", 
		NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
	dlg.m_ofn.lpstrTitle = "Please select a video file";

	if(dlg.DoModal() == IDOK)
	{
		CString filename = dlg.GetPathName();	
		m_capture = cvCaptureFromFile(filename);//������Ƶ
		if(!m_capture)
		{
			MessageBox("����Ƶʧ��");
			return;
		}
		m_FrameAll = cvGetCaptureProperty(m_capture,CV_CAP_PROP_FRAME_COUNT);//��ȡ��֡��

		m_height = cvGetCaptureProperty(m_capture,CV_CAP_PROP_FRAME_HEIGHT);//��ȡ��Ƶ�߶�
		m_width = cvGetCaptureProperty(m_capture,CV_CAP_PROP_FRAME_WIDTH);//��ȡ��Ƶ���


		cvNamedWindow("����",0);
		cvNamedWindow("�������ͼ",0);
		m_bKillThread = false;
		m_pThread = AfxBeginThread(ThreadFunction, (void*)this);//��������ָ�봫���߳�


	}	
}

void CtargettrackDlg::ProcessForeground(Mat &frame,Mat &fgimg)
{
	Mat frame2 = frame.clone();
	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(7, 7), Point(3, 3));
	//ȥ���ӵ�
	erode(fgimg, fgimg, element);//��ʴ
	dilate(fgimg, fgimg, element);//����
	
	//���������ֹ����
	element = getStructuringElement(MORPH_RECT, Size(25, 31), Point(12, 15));
	dilate(fgimg, fgimg, element);//����
	erode(fgimg, fgimg, element);//��ʴ

	//�����ǰ��ͼ������һϵ�д���ɸѡ�ҳ����ʵ��������
	Mat bwimg2 = fgimg.clone();//Ѱ����ͨ���򣬻��ƻ�ԭͼ�����Ա��ݸ�����
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(bwimg2, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));//Ѱ����ͨ����

	/*cvFindContours*/
	//�����������
	double area = 0;
	int i, j;
	double imagearea = bwimg2.rows * bwimg2.cols;//����ͼ����� ��*��
	int objectnum =  0;//Ŀ�����
	vector<CvRect> objectRects;//��ż����ľ�������

	//ɸѡ�����£�ȥ��һЩС���ӵ�
	for (i = 0; i < contours.size(); i++)
	{
		Rect r = boundingRect(contours[i]);//��С��Ӿ���
		//area = fabs(contourArea(Mat(contours[i])));//�����������
		area = r.width * r.height;
		double ratio = area/imagearea;//������ı���
		if (ratio > areathresholdvalue)//���������ֵ�Ĳ���Ϊ������Ŀ�꣬��Ҫ����
		{
			objectRects.push_back(r);
			//rectangle(frame,r,Scalar(0,255, 0));//������ɫ����
		}
	}

	objectnum = objectRects.size();
	if(objectnum <= 0)
	{
		state.updateState(0);
		if(bhaveobject)
		{
			disappearnum++;//������ʧ֡��
		}

		//������ʧ��
		if(bhaveobject && disappearnum > 20)
		{
			state.clear();//״̬���
			disappearnum = 0;//������ʧ֡��
			tracklist.clear();//���
			bottomT = 0;//��0
			meanyT = 0;
			totalweight = 0;
			bhaveobject = false;
			oldtype = -1;
			
			GetDlgItem(IDC_EDIT_W)->SetWindowText("0");
		}

		return;
	}
	else
	{
		disappearnum = 0;
		
		bhaveobject = true;
	}
	//�ҳ�һ����� ȥ���Ա�
	double maxarea = 0;
	int maxindex = -1;
	for (i = 0; i < objectRects.size(); i++)
	{
		Rect r = objectRects[i];//ȡ��һ������
		//area = fabs(contourArea(Mat(contours[i])));//�����������
		area = r.width * r.height;
		if (area > maxarea)
		{
			maxindex = i;
		}
	}

	Rect maxr = objectRects[maxindex];

	//�������ĵ�
	float x = maxr.x + float(maxr.width)*0.5;
	float y = maxr.y + float(maxr.height)*0.5;
	tracklist.push_back(Point2f(x,y));//�켣�洢

	//����Ծ��ֵ
	if(tracklist.size() <= jumpT)
	{
		//���ĸ߶�
		double centery = double(maxr.y) + double(maxr.height)*0.5;
		//�ײ�
		double bottom = double(maxr.y) + double(maxr.height);

		bottomT  = bottomT+bottom;
		meanyT = meanyT + centery;
		//����ƽ��ֵ,������������� ����Ϊ��Ծ�Ĳο�ֵ
		if(tracklist.size() == jumpT)
		{
			bottomT = bottomT/jumpT;
			meanyT = meanyT/jumpT;
		}
	}


	double rate = double(maxr.height)/double(maxr.width);//���ж���̬�Ƿ��쳣���߿�� �ж��Ƿ���� ſ��

	//double ratio = state.getRate();//��ȡˤ����֡��ռ����
	bool bunusual = false;//�Ƿ��쳣
	int unusualtype = -1;//�쳣����
	if(rate <= 1.3 )//��������ﵽ��ֵ����Ϊ�쳣
	{
		
		//rectangle(frame,maxr,Scalar(0,0,255),2);//���ƺ����

		Point pt;
		pt.x = maxr.x;
		pt.y = maxr.y-5;
		if(rate > 0.9 )
		{
			unusualtype = 0;//����
			/*putText(frame,"Squat",pt,1,1.3,CV_RGB(255,0,0),2);
			GetDlgItem(IDC_EDIT_STATE)->SetWindowText("����");*/
		}
		else
		{
			unusualtype = 1;//����
			/*putText(frame,"Lie",pt,1,1.3,CV_RGB(255,0,0),2);
			GetDlgItem(IDC_EDIT_STATE)->SetWindowText("����");*/
		}
		bunusual = true;
		
	}
	else //��̬�������ٶ�̬���ж��Ƿ���Ծ ͣ��
	{
		//��������ж��Ƿ���Ծ
		bool bjump = isjump(maxr);
		if(bjump)//�����Ծ
		{
			//rectangle(frame,maxr,Scalar(0,0,255),2);//���ƺ����
			//putText(frame,"Jump",pt,1,1.3,CV_RGB(255,0,0),2);
			//GetDlgItem(IDC_EDIT_STATE)->SetWindowText("��Ծ");
			unusualtype = 2;//��Ծ
			bunusual = true;
		}
		else
		{
			bool bstop = isstop(tracklist);//�ж��Ƿ�ͣ��
			if(bstop)
			{
				//rectangle(frame,maxr,Scalar(0,0,255),2);//���ƺ����
				//putText(frame,"Stop",pt,1,1.3,CV_RGB(255,0,0),2);
				//GetDlgItem(IDC_EDIT_STATE)->SetWindowText("ͣ��");
				unusualtype = 3;//ͣ��
				bunusual = true;
			}
			else
			{
				//rectangle(frame,maxr,Scalar(0,255,0),2);//�����̾���
				//putText(frame,"Normal",pt,1,1.3,CV_RGB(0,255,0),2);
				//GetDlgItem(IDC_EDIT_STATE)->SetWindowText("����");
				unusualtype = -1;
			}
		}
		
		
	}

	if(bunusual)
	{
		//ˢ��״̬
		state.updateState(1);
	}
	else
	{
		//ˢ��״̬
		state.updateState(0);
	}

	//��Ծ�Ƚ϶��� �Ͳ����������ж���
	int type = -1;//����
	if(unusualtype == 2)
	{
		type = unusualtype;
	}
	else
	{
		//�쳣�����ﵽһ������
		if(state.getRate() > 0.4)
		{
			if(bunusual)
			{
				type = unusualtype;
			}
			else
			{
				type = oldtype;//��״̬
			}
		}
		else
		{
			type = -1;//����
		}
	}

	/*if (type >= 0 && type!=oldtype)
	{
		totalweight = totalweight+weights[type];//Ȩ������
		CString str;
		str.Format("%d",totalweight);
		GetDlgItem(IDC_EDIT_W)->SetWindowText(str);
	}*/
	if(m_FrameCount % maxnum == 0)
	{
		totalweight = 0;
	}
	/*else
	{
		if (type >= 0 && type!=oldtype)
	    {
		    totalweight = totalweight+weights[type];//Ȩ������
		    CString str;
		    str.Format("%d",totalweight);
		    GetDlgItem(IDC_EDIT_W)->SetWindowText(str);
	    }
	}*/
	if (type >= 0 && type!=oldtype)
	{
		totalweight = totalweight+weights[type];//Ȩ������
		CString str;
		str.Format("%d",totalweight);
		GetDlgItem(IDC_EDIT_W)->SetWindowText(str);
	}

	Point pt;
	pt.x = maxr.x;
	pt.y = maxr.y-5;

	if (pt.y < 10)
	{
		pt.y = 10;
	}
	if(type == -1)
	{
		rectangle(frame,maxr,Scalar(0,255,0),2);//�����̾���
		putText(frame,"Normal",pt,1,1.3,CV_RGB(0,255,0),2);
		GetDlgItem(IDC_EDIT_STATE)->SetWindowText("����");
	}
	else
	{
		rectangle(frame,maxr,Scalar(0,0,255),2);//���ƺ����
		putText(frame,strtypenamesEng[type],pt,1,1.3,CV_RGB(255,0,0),2);
		GetDlgItem(IDC_EDIT_STATE)->SetWindowText(strtypenames[type]);
	}

	oldtype = type;//����oldtype
	if(type >= 0)
	{
		
		//�����������
		Rect facer;
		bool bd = detectOneFace(frame2,facer);
		if(bd)
		{
			m_faceimg = frame2(facer);

			IplImage imgshow2 = m_faceimg;
			show_pic(&imgshow2,IDC_STATIC_PIC3);

			////�쳣ͼƬ�洢
			//CString strname;
			//strname.Format("pics\\%d.jpg",m_FrameCount);
			//imwrite(strname.GetBuffer(0),frame2);
			if(totalweight >= weightT) //Ȩ�ش� ��ʶ����Ϣ
			{
				int type = gettype(m_faceimg);
				CString strresult;
				strresult.Format("���֤��:\r\n%s\r\n����:\r\n%s\r\n",m_personlist[type].idno,m_personlist[type].name);//��֯Ҫ��ʾ�Ľ������
			
				GetDlgItem(IDC_STATIC_FACE)->SetWindowText(strresult);
			}
		}
	}
}

//�ڿؼ�����ʾͼƬ
void CtargettrackDlg::show_pic(IplImage *t,int nID) 
{
	LPBITMAPINFO lpBmi=CreateMapInfo(t);//����λͼ��Ϣ��������ʾ 

	//�趨��ͼ�������CDC
	CDC *pDC = GetDlgItem(nID)->GetDC();
	//	pDC = GetDC();
	CRect rect;
	GetDlgItem(nID)->GetWindowRect(&rect);
	ScreenToClient(rect);

	IplImage* imgcopy = cvCreateImage( cvSize(t->width,t->height), 8, t->nChannels );//�Ҷ�ͼ��
	cvCopy(t,imgcopy);
	cvFlip(imgcopy);


	if( imgcopy->width > rect.Width() )
	{
		SetStretchBltMode(
			pDC->m_hDC,           // handle to device context
			HALFTONE );
	}
	else
	{
		SetStretchBltMode(
			pDC->m_hDC,           // handle to device context
			COLORONCOLOR );
	}
	StretchDIBits(pDC->m_hDC,
		0,0,rect.Width(),rect.Height(),
		0,0,t->width,t->height,
		imgcopy->imageDataOrigin,lpBmi,DIB_RGB_COLORS,SRCCOPY);

	//����ڴ�
	cvReleaseImage(&imgcopy);
	free(lpBmi);
	ReleaseDC(pDC);
}

void CtargettrackDlg::OnBnClickedButtonPause()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(m_pThread)//���ڲ��ŵ�ʱ��Ŵ�������� ����
	{
		m_bPause = !m_bPause;//��ͣ״̬ȡ��
		if (m_bPause)//�����������ͣ
		{
			SetDlgItemTextA(IDC_BUTTON_PAUSE, "����") ;
		}
		else
		{
			SetDlgItemTextA(IDC_BUTTON_PAUSE, "��ͣ") ;
		}

	}
}

void CtargettrackDlg::OnBnClickedButtonStop()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(m_pThread)//���ڲ��ŵ�ʱ��Ŵ�������� ����
	{
		m_bPause = false;//ʹ��ͣΪfalse���Է�ֹ�俨ס
		m_bKillThread = true;//֪ͨ�߳̽�����Ҳ���Ǵ��������whileѭ��
		WaitForSingleObject(m_pThread->m_hThread,2000);//�ȴ��߳̽���
		m_pThread = NULL;

		SetDlgItemTextA(IDC_BUTTON_PAUSE, "��ͣ") ;
	}
}

void CtargettrackDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OnBnClickedButtonStop();//��ֹͣ����
	OnOK();
}

void CtargettrackDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	OnBnClickedButtonStop();//��ֹͣ����
	CDialog::OnClose();
}


bool CtargettrackDlg::detectOneFace(Mat& img, Rect &facer)
{
	std::vector<Rect> faceList;

	Mat sizeimg;
	resize(img,sizeimg,Size(0,0),0.5,0.5);//��Сͼ��
	//ִ�м��
	cascade.detectMultiScale(sizeimg, faceList,
		1.1, 2, 0
		//|CV_HAAR_FIND_BIGGEST_OBJECT
		//|CV_HAAR_DO_ROUGH_SEARCH
		| CV_HAAR_SCALE_IMAGE
		,
		Size(20, 20));

	if (faceList.size() == 0)
	{
		return false;
	}

	int maxarea = 0;
	for (int i = 0; i < faceList.size(); i++)
	{
		int area = faceList[i].area();
		//������ѯ��ѡ����������
		if (area > maxarea)
		{
			maxarea = area;
			facer = faceList[i];
		}
	}

	facer.x = facer.x*2;
	facer.y = facer.y*2;
	facer.width = facer.width*2;
	facer.height = facer.height*2;
	return true;
}


void CtargettrackDlg::mytrain()
{
	trainlbpfeatures = Mat();
	trainLabel = Mat(); //ѵ�������ͱ�ǩ
	int i = 0;
	
	for (i=0;i<m_personlist.size();i++)//������Щ���
	{	
		char strpicpath[260] = {0};
		sprintf(strpicpath,"%s\\������\\%d",GetFilePath().GetBuffer(0),i+1);//��֯���ļ�������

		vector<string> files;
		getPicFiles(strpicpath, files);//��ȡ����ļ����µ�����ͼ
		int filenum = files.size();//�ļ�����

		for (int j = 0; j < filenum; j++)//������Щѵ��ͼ
		{
			Mat image = imread(files[j].c_str());//��ȡ���ͼƬ
			if(image.data == NULL)
			{
				continue;//�����ȡʧ��
			}
			Rect facer;
			bool bd = detectOneFace(image,facer);
			if(!bd)
			{
				continue;
			}
			Mat faceimg = image(facer);
			Mat procimg;
			cvtColor(faceimg,procimg,CV_BGR2GRAY);//תΪ�Ҷ�ͼ��
			resize(procimg,procimg,Size(100,100));//תΪ�̶��ߴ�

			float lbpfeature[59] = {0};
			COMPUTE_LBP(procimg,lbpfeature,maptable);//����lbp����
			Mat lfeature(1,59,CV_32FC1);
			memcpy(lfeature.data,lbpfeature,sizeof(float)*59);
			
			
			trainlbpfeatures.push_back(lfeature);//��һ�д洢��traindata��
	
			trainLabel.push_back(i);//�����

		}

	}
}

int CtargettrackDlg::gettype(Mat faceimg)
{
	Mat procimg;
	cvtColor(faceimg,procimg,CV_BGR2GRAY);//תΪ�Ҷ�ͼ��
	resize(procimg,procimg,Size(100,100));//תΪ�̶��ߴ�

	float lbpfeature[59] = {0};
	COMPUTE_LBP(procimg,lbpfeature,maptable);//����lbp����
	Mat lfeature(1,59,CV_32FC1);
	memcpy(lfeature.data,lbpfeature,sizeof(float)*59);
			

	int trainnum  = trainLabel.rows;//�ܹ���ѵ��������
	float maxss = 0;//������ƶ�
	int testlabel = 0;
	for(int i=0;i<trainnum;i++)
	{
		Mat tmp = trainlbpfeatures.row(i);//ȡ��һ�����ݣ�Ҳ����һ��ͼ����������
		float ss = mysimilar(lfeature,tmp);//�������ƶ�

		
		if(ss > maxss)//�ҳ�������ƶ�
		{
			maxss = ss;
			testlabel = trainLabel.at<int>(i,0);//�������б���ȡ����Ӧ������
		}
	}

	return testlabel;
}

//��ʼ�������ݿ�
BOOL CtargettrackDlg::InitDataBase()
{
	
	char m_szConnect[512];
	char m_szTmp[1024]="" ;
	
		HRESULT hr;
	try
	{
		//����XdData
		hr = m_Connection.CreateInstance(__uuidof(Connection));
		//�û����뷽ʽ
		//sprintf(m_szConnect,"provider = sqloledb;server=%s;database=%s;", m_szHost, m_szDef);//��֯�����ַ���
		//hr=m_Connection->Open(_bstr_t(m_szConnect),_bstr_t(m_szUser),_bstr_t(m_szPwd),-1);

		//win��ʽ
		sprintf(m_szConnect,"provider = sqloledb;server=%s;database=%s;Integrated Security=SSPI;", m_szHost, m_szDef);//��֯�����ַ���
		hr=m_Connection->Open(_bstr_t(m_szConnect),"", "", -1);
	
		sprintf(m_szTmp, "���ݿ����ӳɹ�!");
		//����XdData
	}
	catch(_com_error & e) 
	{
		sprintf(m_szTmp, "���ݿ��ʧ��,����ԭ��%s\n",LPCTSTR(e.Description()));
		return FALSE ;
	}
	return TRUE;
}

//��ȡ������Ϣ�����ݿ���Ϣ
void CtargettrackDlg::ReadIni()
{
	CString IniFile = GetFilePath() + "\\����.ini";//���������ļ�·��

	memset(m_szHost,0,sizeof(m_szHost));
	memset(m_szUser,0,sizeof(m_szUser));
	memset(m_szPwd,0,sizeof(m_szPwd));
	memset(m_szDef,0,sizeof(m_szDef));
	GetPrivateProfileString("���ݿ�", "������", NULL, m_szHost, sizeof(m_szHost), IniFile) ;
	GetPrivateProfileString("���ݿ�", "�û���", NULL, m_szUser, sizeof(m_szUser), IniFile) ;
	GetPrivateProfileString("���ݿ�", "����", NULL, m_szPwd, sizeof(m_szPwd), IniFile) ;
	GetPrivateProfileString("���ݿ�", "���ݿ���", NULL, m_szDef, sizeof(m_szDef), IniFile) ;

}

//���������ȡ���ݿ���������Ա��Ϣ
void CtargettrackDlg::GetAllPerson()
{
	_variant_t v ;
	_RecordsetPtr m_Rsp ;
	char m_szSql[512] = {0};
	CString str;
	sprintf(m_szSql, "select * from pinfo") ;//��֯sql���
	m_personlist.clear();
	try
	{
		m_Rsp = m_Connection->Execute(_bstr_t(m_szSql), &v, adCmdText) ;
		while(!m_Rsp->GetadoEOF())
		{
			PERSONINFO info;

			v = m_Rsp->GetCollect("IDNO") ;
			info.idno = (LPCTSTR)_bstr_t(v);

			v = m_Rsp->GetCollect("name") ;
			info.name = (LPCTSTR)_bstr_t(v);

			v = m_Rsp->GetCollect("address") ;
			info.address = (LPCTSTR)_bstr_t(v);

			v =  m_Rsp->GetCollect("phone") ;
			info.phone = (LPCTSTR)_bstr_t(v);

			m_Rsp->MoveNext() ;
			m_personlist.push_back(info);//������Ϣ �����б���
		}
	}
	catch (_com_error & e)
	{
		char m_szTmp[1024] ;
		sprintf(m_szTmp, "ִ��==>%s<==, ���ݿ����ʧ��,����ԭ��%s\n",m_szSql, LPCTSTR(e.Description()));
	}
	return;
}