// targettrackDlg.cpp : 实现文件
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

//异常类型名称
const char* strtypenames[] = {"蹲下","躺下","跳跃","停留"};
const char* strtypenamesEng[] = {"Squat","Lie","Jump","Stop"};
const int typenum = 4; /* */

const int weights[]={30,100,50,10};//这几类的权重值


int bgframenum = 30;//参与计算背景的帧数，如果想一直计算，可以将此设置为小于等于0即可
int bwthresholdvalue = 22;//二值化阈值
double areathresholdvalue = 0.04;//目标面积阈值，在画面中所占比例

int statecalcnum = 30;//参与统计的最近的帧数
StateCount state(statecalcnum);//设置个状态统计对象，有些形态需要持续N帧才能确定，

vector<Point2f> tracklist;//轨迹 用来判断是否停下，行走

int stopT = 45;//停留阈值，持续帧数
double stopdistT = 12;//驻留距离阈值，这个人的位移差异 

//用于判断是否跳跃
int jumpT = 100;//跳跃阈值，目标持续帧数达到这个才计算跳跃
double bottomT = 0;//平均底部
double meanyT = 0;//平均中心高度

int disappearnum = 0;//连续消失帧数
int totalweight = 0;//综合权重
bool bhaveobject = false;//是否有目标出现

int oldtype = -1;//上一帧的状态
int weightT = 150;//权重阈值

int maxnum = 1000;//开启下一轮检测的最大帧数

//这个函数计算距离
double compute_dist(Point2f pt1,Point2f pt2)
{
	double dist = 0;
	//dist = (pt1.x - pt2.x)*(pt1.x - pt2.x) + (pt1.y - pt2.y)*(pt1.y - pt2.y);
	dist = (pt1.x - pt2.x)*(pt1.x - pt2.x);
	dist = sqrt(dist);
	return dist;
}

//这个函数判断是否驻留
bool isstop(vector<Point2f> &points,int T = stopT)
{
	int ptnum = points.size();//点的个数
	if(ptnum < T)
	{
		return false;//点数不够，不算驻留
	}

	int startindex = ptnum-T;//前N帧的序号

	double dist = compute_dist(points[startindex],points[ptnum-1]);//计算距离

	if(dist <= stopdistT)
	{
		return true;//驻留
	}
	else
	{
		return false;
	}
}

//这个函数判断是否跳跃
bool isjump(Rect r)
{

	if(tracklist.size() < jumpT)
	{
		return false;
	}
	//中心高度
	double centery = double(r.y) + double(r.height)*0.5;

	double buttom = double(r.y) + double(r.height);

	//跳跃后 质心上移  依靠这个判断
	if( (centery < meanyT-15) && (buttom < bottomT-15) )
	{
		return true;
	}
	else
	{
		return false;
	}

}

//训练的人脸特征集合
Mat trainlbpfeatures;
Mat trainLabel; //训练的类型标签

//58种二进制突变次数小于等于2的数
const int LBP_NUM[]={0,1,2,3,4,6,7,8,12,14,15,16,24,28,30,31,32,48,56,60,62,63,64,96,112,120,124,126,127,128,129,131,135,
143,159,191,192,193,195,199,207,223,224,225,227,231,239,240,241,243,247,248,249,251,252,253,254,255};
//另外不属于这58个算作一类，总共59类，也就是说lbp直方图特征是59维度
uchar maptable[256] = {0};

//创建映射表，256维度的映射到59维度
void CreateMap(uchar maptable[])
{
	int i;
	for (i=0;i<256;i++)
	{
		maptable[i] = 58;//先初始都等于58
	}

	for (i=0;i<58;i++)
	{
		int index = LBP_NUM[i];//得到对应的位置
		maptable[index] = i;//对应位置赋值
	}
}


//这个函数生成lbp码图，计算LBP特征
Mat COMPUTE_LBP(Mat image,float result[],uchar maptable[])
{
	
	Mat lbpimg = image.clone();
	uchar center=0;//某点的原始像素值
	uchar center_lbp=0;//要转换的lbp值
	int row,col;
	//遍历像素点，像素点和周边的8个挨着比较，二进制相加
	for (int row=1; row<image.rows-1; row++)
	{
		for (int col=1; col<image.cols-1; col++)
		{
			center = image.at<uchar>(row,col);//像素值
			center_lbp = 0;

			uchar tmpvalue =  image.at<uchar>(row-1,col-1);//左上
			if(center <= tmpvalue)
			{
				center_lbp += 128;
			}

			tmpvalue =  image.at<uchar>(row-1,col);//正上
			if(center <= tmpvalue)
			{
				center_lbp += 64;
			}


			tmpvalue =  image.at<uchar>(row-1,col+1);//右上
			if(center <= tmpvalue)
			{
				center_lbp += 32;
			}

			tmpvalue =  image.at<uchar>(row,col+1);//正右
			if(center <= tmpvalue)
			{
				center_lbp += 16;
			}

			tmpvalue =  image.at<uchar>(row+1,col+1);//右下
			if(center <= tmpvalue)
			{
				center_lbp += 8;
			}

			tmpvalue =  image.at<uchar>(row+1,col);//正下
			if(center <= tmpvalue)
			{
				center_lbp += 4;
			}

			tmpvalue =  image.at<uchar>(row+1,col-1);//左下
			if(center <= tmpvalue)
			{
				center_lbp += 2;
			}

			tmpvalue =  image.at<uchar>(row,col-1);//正左
			if(center <= tmpvalue)
			{
				center_lbp += 1;
			}

			lbpimg.at<uchar>(row,col) = center_lbp;//lbp值
			uchar dimvalue = maptable[center_lbp];//从表里转为0-58值
			result[dimvalue] = result[dimvalue]+1;//加1统计
		}
	}
	//经过以上，得到的lbpimg就是lbp谱图
	//然后直方图特征归一化
	int i;
	for (i=0;i<59;i++)
	{
		result[i] = result[i]/((image.rows-2)*(image.cols-2));
	}


	return lbpimg;//同时返回lbp码图 便于分析用
}

//采用相交系数来求
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


//IplImage 转 BMP
LPBITMAPINFO  CreateMapInfo(IplImage* workImg)    //  建立位图信息
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
	memcpy(lpBmi,&BIH,40);                   //  复制位图信息头
	if (bits==8) {                           //  256 色位图
		for (i=0;i<256;i++)  {                //  设置灰阶调色板
			ColorTab[i].rgbRed=ColorTab[i].rgbGreen=ColorTab[i].rgbBlue=(BYTE) i;
		}
		memcpy(lpBmi->bmiColors, ColorTab, 1024);
	}
	return(lpBmi);
}

//获取程序绝对路径路径
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

//这个函数遍历获得文件夹内的所有图片文件 bmp jpg png
void getPicFiles(string path, vector<string>& files)
{
	//文件句柄
	long  long hFile   =   0;
	//文件信息
	struct _finddata_t fileinfo;
	string p;
	if((hFile = _findfirst(p.assign(path).append("\\*").c_str(),&fileinfo)) !=  -1)
	{
		do
		{
			//如果是目录,迭代之
			//如果不是,加入列表
			if((fileinfo.attrib &  _A_SUBDIR))
			{
				if(strcmp(fileinfo.name,".") != 0  &&  strcmp(fileinfo.name,"..") != 0)
					getPicFiles(path+"\\"+fileinfo.name, files);
			}
			else
			{

				char *pp;
				pp = strrchr(fileinfo.name,'.');//查找最后出现的位置
				if (_stricmp(pp,".bmp")==0 || _stricmp(pp,".jpg")==0 || _stricmp(pp,".png")==0 )//如果找到的是图片就行处理
				{
					files.push_back(p.assign(path).append("\\").append(fileinfo.name));//这个存储带路径的文件全名
				}
			}
		}while(_findnext(hFile, &fileinfo)  == 0);
		_findclose(hFile);
	}
}



//复制IplImage
int imageClone(IplImage* pi,IplImage** ppo)  
{
	if (*ppo) {
		cvReleaseImage(ppo);                //  释放原来位图
	}
	(*ppo) = cvCloneImage(pi);              //  复制新位图
	return(1);
}


//frame 当前帧
//bgimg 背景图像
//fgimg 黑白前景图  这个函数帧差 得到前景
void GetForeground(Mat &frame,Mat &bgimg,Mat &fgimg)
{
	Mat framegray;//原图的灰度图
	Mat bgimggray;//背景图的灰度图

	cvtColor(frame, framegray, CV_BGR2GRAY);  //转为灰度图像
	cvtColor(bgimg, bgimggray, CV_BGR2GRAY);  //转为灰度图像

	absdiff(bgimggray,framegray,fgimg);//和背景做差分
	imshow("背景差分图",fgimg);

	threshold(fgimg,fgimg,bwthresholdvalue,255,THRESH_BINARY);  //二值化

}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CtargettrackDlg 对话框




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


// CtargettrackDlg 消息处理程序

BOOL CtargettrackDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//线程信息初始化
	m_bKillThread = FALSE;
	m_bPause = FALSE;
	m_pThread = NULL;
	m_FrameCount = 0;
	m_FrameAll = 0;

	CString strfilename = GetFilePath()+ "\\haarcascade_frontalface_alt2.xml";//xml
	 // 装载读取人脸检测器
	if( !cascade.load( strfilename.GetBuffer(0) ) )
	{
        cerr << "ERROR: Could not load classifier cascade" << endl;
        exit(0) ;
    }

		ReadIni();//读取配置文件
	BOOL bret = InitDataBase();//连接数据库
	if (!bret)
	{
		MessageBox( "数据库访问失败,程序异常关闭!", "出错啦", MB_OK) ;
		exit(1) ;
	}

	CreateMap(maptable);//创建lbp映射表

	GetAllPerson();//获取所有人员信息
	mytrain();//提取人脸库特征


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CtargettrackDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CtargettrackDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//线程函数
unsigned int ThreadFunction( void* param )
{
	IplImage *image;

	//	TagParam *m_tagparam = (TagParam*)param;
	CtargettrackDlg *pdlg = (CtargettrackDlg *)param;//取出窗口类指针
	CvCapture *capture = pdlg->m_capture;//得到视频指针
	if (!capture)
	{
		return 0;
	}
	int framenum = 0;//帧计数
	BackgroundSubtractorMOG2 bgSubtractor(20,60,true); //高斯背景操作类
	MyBackground mybgSubtractor;//自己写的背景操作类

	Mat frame,bgimg,fgimg;//分别是当前帧 背景 前景

	state.clear();//状态清空
	disappearnum = 0;//持续消失帧数
	tracklist.clear();//清空
	bottomT = 0;//归0
	meanyT = 0;
	totalweight = 0;
	bhaveobject = false;
	oldtype = -1;

	while( (pdlg->m_bKillThread) == false )
	{
		image = cvQueryFrame(pdlg->m_capture);//获取一帧
		if(image == NULL)//取不出数据了，说明播放完毕，退出循环
		{
			break;
		}
		//cvResize(image,framesize);//调节大小

		frame = cvarrToMat(image);//转为mat
		//如果图像本身过大，需要对检测的图像缩小下
		int maxHW = MAX(frame.rows,frame.cols);//这个图的最大边长
		int maxlength = 800;//要转换的最大边长
		Mat imgsize;
		if(maxHW > maxlength)
		{
			double ratio = double(maxlength)/double(maxHW);//要缩小的比例
			resize(frame, imgsize, cv::Size(0, 0), ratio,ratio);//缩小下图像
		}
		else
		{
			imgsize = frame;
		}
		frame = imgsize;//当前帧

		//自己写的均值背景方式
		if(bgframenum <= 0 )
		{
			mybgSubtractor.updateBackground(frame);//刷新背景
			
		}
		else if(framenum <= bgframenum)
		{
			mybgSubtractor.updateBackground(frame);//刷新背景
		}
		mybgSubtractor.getBackgroundImage(bgimg);//取出模型中的背景

	
		//得到背景后，计算得到前景，
		GetForeground(frame,bgimg,fgimg);//计算黑白前景图
		//
		
		pdlg->ProcessForeground(frame,fgimg);//处理前景,得到这一贞是否摔倒
		

		IplImage imgshow = fgimg;
		IplImage imgshow2 = frame;
		pdlg->show_pic(&imgshow2,IDC_STATIC_PIC1);
		pdlg->show_pic(&imgshow,IDC_STATIC_PIC2);
	
		framenum++;//帧计数加1
		pdlg->m_FrameCount = framenum;
		CString str;
		str.Format("播放进度:%d/%d帧",pdlg->m_FrameCount,pdlg->m_FrameAll);
		pdlg->SetDlgItemText(IDC_STATIC_STATE,str);
		
		if(pdlg->m_bPause)
		{
			str.Format("播放进度:暂停%d/%d帧",pdlg->m_FrameCount,pdlg->m_FrameAll);
			pdlg->SetDlgItemText(IDC_STATIC_STATE,str);
		}
		while(pdlg->m_bPause)
		{
			
			Sleep(30);//如果暂停了，就在这卡住等待
		}
		
		imshow("背景",bgimg);
		cvWaitKey(25);
	}

	cvReleaseCapture(&capture);
	pdlg->m_capture = NULL;
	pdlg->m_bKillThread = TRUE;
	pdlg->m_pThread = NULL;
	pdlg->m_bPause = FALSE;
	CString str;
	str.Format("播放进度：未播放");
	pdlg->SetDlgItemText(IDC_STATIC_STATE,str);
	return( 0 );
}

void CtargettrackDlg::OnBnClickedButtonPlay()
{
	// TODO: 在此添加控件通知处理程序代码

	if(m_pThread)
	{
		return;//如果已经打开播放了，直接返回
	}
	
	//弹出一个对话框选择文件
	char szFilter[] = "video Files (*.avi;*.mp4)|*.avi;*.mp4|All Files (*.*)|*.*||";
	CFileDialog dlg(true, "avi", 
		NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
	dlg.m_ofn.lpstrTitle = "Please select a video file";

	if(dlg.DoModal() == IDOK)
	{
		CString filename = dlg.GetPathName();	
		m_capture = cvCaptureFromFile(filename);//载入视频
		if(!m_capture)
		{
			MessageBox("打开视频失败");
			return;
		}
		m_FrameAll = cvGetCaptureProperty(m_capture,CV_CAP_PROP_FRAME_COUNT);//获取总帧数

		m_height = cvGetCaptureProperty(m_capture,CV_CAP_PROP_FRAME_HEIGHT);//获取视频高度
		m_width = cvGetCaptureProperty(m_capture,CV_CAP_PROP_FRAME_WIDTH);//获取视频宽度


		cvNamedWindow("背景",0);
		cvNamedWindow("背景差分图",0);
		m_bKillThread = false;
		m_pThread = AfxBeginThread(ThreadFunction, (void*)this);//将窗口类指针传给线程


	}	
}

void CtargettrackDlg::ProcessForeground(Mat &frame,Mat &fgimg)
{
	Mat frame2 = frame.clone();
	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(7, 7), Point(3, 3));
	//去除杂点
	erode(fgimg, fgimg, element);//腐蚀
	dilate(fgimg, fgimg, element);//膨胀
	
	//链接人体防止断裂
	element = getStructuringElement(MORPH_RECT, Size(25, 31), Point(12, 15));
	dilate(fgimg, fgimg, element);//膨胀
	erode(fgimg, fgimg, element);//腐蚀

	//下面对前景图，进行一系列处理，筛选找出合适的区域出来
	Mat bwimg2 = fgimg.clone();//寻找连通区域，会破坏原图，所以备份个出来
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(bwimg2, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));//寻找连通区域

	/*cvFindContours*/
	//区域面积过滤
	double area = 0;
	int i, j;
	double imagearea = bwimg2.rows * bwimg2.cols;//整个图的面积 宽*高
	int objectnum =  0;//目标个数
	vector<CvRect> objectRects;//存放检测出的矩形区域

	//筛选过滤下，去除一些小的杂点
	for (i = 0; i < contours.size(); i++)
	{
		Rect r = boundingRect(contours[i]);//最小外接矩形
		//area = fabs(contourArea(Mat(contours[i])));//这个区域的面积
		area = r.width * r.height;
		double ratio = area/imagearea;//看面积的比例
		if (ratio > areathresholdvalue)//面积大于阈值的才认为是人体目标，需要处理
		{
			objectRects.push_back(r);
			//rectangle(frame,r,Scalar(0,255, 0));//绘制绿色矩形
		}
	}

	objectnum = objectRects.size();
	if(objectnum <= 0)
	{
		state.updateState(0);
		if(bhaveobject)
		{
			disappearnum++;//持续消失帧数
		}

		//人物消失了
		if(bhaveobject && disappearnum > 20)
		{
			state.clear();//状态清空
			disappearnum = 0;//持续消失帧数
			tracklist.clear();//清空
			bottomT = 0;//归0
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
	//找出一个最大 去做对比
	double maxarea = 0;
	int maxindex = -1;
	for (i = 0; i < objectRects.size(); i++)
	{
		Rect r = objectRects[i];//取出一个矩形
		//area = fabs(contourArea(Mat(contours[i])));//这个区域的面积
		area = r.width * r.height;
		if (area > maxarea)
		{
			maxindex = i;
		}
	}

	Rect maxr = objectRects[maxindex];

	//计算质心点
	float x = maxr.x + float(maxr.width)*0.5;
	float y = maxr.y + float(maxr.height)*0.5;
	tracklist.push_back(Point2f(x,y));//轨迹存储

	//求跳跃阈值
	if(tracklist.size() <= jumpT)
	{
		//中心高度
		double centery = double(maxr.y) + double(maxr.height)*0.5;
		//底部
		double bottom = double(maxr.y) + double(maxr.height);

		bottomT  = bottomT+bottom;
		meanyT = meanyT + centery;
		//计算平均值,这里求出的数据 就作为跳跃的参考值
		if(tracklist.size() == jumpT)
		{
			bottomT = bottomT/jumpT;
			meanyT = meanyT/jumpT;
		}
	}


	double rate = double(maxr.height)/double(maxr.width);//先判断形态是否异常，高宽比 判断是否蹲下 趴下

	//double ratio = state.getRate();//获取摔倒的帧所占比例
	bool bunusual = false;//是否异常
	int unusualtype = -1;//异常类型
	if(rate <= 1.3 )//这个比例达到阈值，认为异常
	{
		
		//rectangle(frame,maxr,Scalar(0,0,255),2);//绘制红矩形

		Point pt;
		pt.x = maxr.x;
		pt.y = maxr.y-5;
		if(rate > 0.9 )
		{
			unusualtype = 0;//蹲下
			/*putText(frame,"Squat",pt,1,1.3,CV_RGB(255,0,0),2);
			GetDlgItem(IDC_EDIT_STATE)->SetWindowText("蹲下");*/
		}
		else
		{
			unusualtype = 1;//躺下
			/*putText(frame,"Lie",pt,1,1.3,CV_RGB(255,0,0),2);
			GetDlgItem(IDC_EDIT_STATE)->SetWindowText("躺下");*/
		}
		bunusual = true;
		
	}
	else //形态正常，再动态的判断是否跳跃 停留
	{
		//这个函数判断是否跳跃
		bool bjump = isjump(maxr);
		if(bjump)//如果跳跃
		{
			//rectangle(frame,maxr,Scalar(0,0,255),2);//绘制红矩形
			//putText(frame,"Jump",pt,1,1.3,CV_RGB(255,0,0),2);
			//GetDlgItem(IDC_EDIT_STATE)->SetWindowText("跳跃");
			unusualtype = 2;//跳跃
			bunusual = true;
		}
		else
		{
			bool bstop = isstop(tracklist);//判断是否停留
			if(bstop)
			{
				//rectangle(frame,maxr,Scalar(0,0,255),2);//绘制红矩形
				//putText(frame,"Stop",pt,1,1.3,CV_RGB(255,0,0),2);
				//GetDlgItem(IDC_EDIT_STATE)->SetWindowText("停留");
				unusualtype = 3;//停留
				bunusual = true;
			}
			else
			{
				//rectangle(frame,maxr,Scalar(0,255,0),2);//绘制绿矩形
				//putText(frame,"Normal",pt,1,1.3,CV_RGB(0,255,0),2);
				//GetDlgItem(IDC_EDIT_STATE)->SetWindowText("正常");
				unusualtype = -1;
			}
		}
		
		
	}

	if(bunusual)
	{
		//刷新状态
		state.updateState(1);
	}
	else
	{
		//刷新状态
		state.updateState(0);
	}

	//跳跃比较短暂 就不进行连续判断了
	int type = -1;//类型
	if(unusualtype == 2)
	{
		type = unusualtype;
	}
	else
	{
		//异常比例达到一定程序
		if(state.getRate() > 0.4)
		{
			if(bunusual)
			{
				type = unusualtype;
			}
			else
			{
				type = oldtype;//旧状态
			}
		}
		else
		{
			type = -1;//正常
		}
	}

	/*if (type >= 0 && type!=oldtype)
	{
		totalweight = totalweight+weights[type];//权重增加
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
		    totalweight = totalweight+weights[type];//权重增加
		    CString str;
		    str.Format("%d",totalweight);
		    GetDlgItem(IDC_EDIT_W)->SetWindowText(str);
	    }
	}*/
	if (type >= 0 && type!=oldtype)
	{
		totalweight = totalweight+weights[type];//权重增加
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
		rectangle(frame,maxr,Scalar(0,255,0),2);//绘制绿矩形
		putText(frame,"Normal",pt,1,1.3,CV_RGB(0,255,0),2);
		GetDlgItem(IDC_EDIT_STATE)->SetWindowText("正常");
	}
	else
	{
		rectangle(frame,maxr,Scalar(0,0,255),2);//绘制红矩形
		putText(frame,strtypenamesEng[type],pt,1,1.3,CV_RGB(255,0,0),2);
		GetDlgItem(IDC_EDIT_STATE)->SetWindowText(strtypenames[type]);
	}

	oldtype = type;//更新oldtype
	if(type >= 0)
	{
		
		//进行人脸检测
		Rect facer;
		bool bd = detectOneFace(frame2,facer);
		if(bd)
		{
			m_faceimg = frame2(facer);

			IplImage imgshow2 = m_faceimg;
			show_pic(&imgshow2,IDC_STATIC_PIC3);

			////异常图片存储
			//CString strname;
			//strname.Format("pics\\%d.jpg",m_FrameCount);
			//imwrite(strname.GetBuffer(0),frame2);
			if(totalweight >= weightT) //权重大 才识别信息
			{
				int type = gettype(m_faceimg);
				CString strresult;
				strresult.Format("身份证号:\r\n%s\r\n姓名:\r\n%s\r\n",m_personlist[type].idno,m_personlist[type].name);//组织要显示的结果内容
			
				GetDlgItem(IDC_STATIC_FACE)->SetWindowText(strresult);
			}
		}
	}
}

//在控件中显示图片
void CtargettrackDlg::show_pic(IplImage *t,int nID) 
{
	LPBITMAPINFO lpBmi=CreateMapInfo(t);//创建位图信息，用于显示 

	//设定绘图的区域和CDC
	CDC *pDC = GetDlgItem(nID)->GetDC();
	//	pDC = GetDC();
	CRect rect;
	GetDlgItem(nID)->GetWindowRect(&rect);
	ScreenToClient(rect);

	IplImage* imgcopy = cvCreateImage( cvSize(t->width,t->height), 8, t->nChannels );//灰度图像
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

	//清空内存
	cvReleaseImage(&imgcopy);
	free(lpBmi);
	ReleaseDC(pDC);
}

void CtargettrackDlg::OnBnClickedButtonPause()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_pThread)//正在播放的时候才触发下面的 操作
	{
		m_bPause = !m_bPause;//暂停状态取反
		if (m_bPause)//如果现在是暂停
		{
			SetDlgItemTextA(IDC_BUTTON_PAUSE, "继续") ;
		}
		else
		{
			SetDlgItemTextA(IDC_BUTTON_PAUSE, "暂停") ;
		}

	}
}

void CtargettrackDlg::OnBnClickedButtonStop()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_pThread)//正在播放的时候才触发下面的 操作
	{
		m_bPause = false;//使暂停为false，以防止其卡住
		m_bKillThread = true;//通知线程结束，也就是打破里面的while循环
		WaitForSingleObject(m_pThread->m_hThread,2000);//等待线程结束
		m_pThread = NULL;

		SetDlgItemTextA(IDC_BUTTON_PAUSE, "暂停") ;
	}
}

void CtargettrackDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedButtonStop();//先停止播放
	OnOK();
}

void CtargettrackDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	OnBnClickedButtonStop();//先停止播放
	CDialog::OnClose();
}


bool CtargettrackDlg::detectOneFace(Mat& img, Rect &facer)
{
	std::vector<Rect> faceList;

	Mat sizeimg;
	resize(img,sizeimg,Size(0,0),0.5,0.5);//缩小图像
	//执行检测
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
		//依次轮询，选出最大的人脸
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
	trainLabel = Mat(); //训练的类型标签
	int i = 0;
	
	for (i=0;i<m_personlist.size();i++)//遍历这些类别
	{	
		char strpicpath[260] = {0};
		sprintf(strpicpath,"%s\\人脸库\\%d",GetFilePath().GetBuffer(0),i+1);//组织子文件夹名称

		vector<string> files;
		getPicFiles(strpicpath, files);//获取这个文件夹下的所有图
		int filenum = files.size();//文件个数

		for (int j = 0; j < filenum; j++)//遍历这些训练图
		{
			Mat image = imread(files[j].c_str());//读取这个图片
			if(image.data == NULL)
			{
				continue;//如果读取失败
			}
			Rect facer;
			bool bd = detectOneFace(image,facer);
			if(!bd)
			{
				continue;
			}
			Mat faceimg = image(facer);
			Mat procimg;
			cvtColor(faceimg,procimg,CV_BGR2GRAY);//转为灰度图像
			resize(procimg,procimg,Size(100,100));//转为固定尺寸

			float lbpfeature[59] = {0};
			COMPUTE_LBP(procimg,lbpfeature,maptable);//计算lbp特征
			Mat lfeature(1,59,CV_32FC1);
			memcpy(lfeature.data,lbpfeature,sizeof(float)*59);
			
			
			trainlbpfeatures.push_back(lfeature);//这一行存储到traindata内
	
			trainLabel.push_back(i);//类别编号

		}

	}
}

int CtargettrackDlg::gettype(Mat faceimg)
{
	Mat procimg;
	cvtColor(faceimg,procimg,CV_BGR2GRAY);//转为灰度图像
	resize(procimg,procimg,Size(100,100));//转为固定尺寸

	float lbpfeature[59] = {0};
	COMPUTE_LBP(procimg,lbpfeature,maptable);//计算lbp特征
	Mat lfeature(1,59,CV_32FC1);
	memcpy(lfeature.data,lbpfeature,sizeof(float)*59);
			

	int trainnum  = trainLabel.rows;//总共的训练样本数
	float maxss = 0;//最大相似度
	int testlabel = 0;
	for(int i=0;i<trainnum;i++)
	{
		Mat tmp = trainlbpfeatures.row(i);//取出一行数据，也就是一个图的特征数据
		float ss = mysimilar(lfeature,tmp);//计算相似度

		
		if(ss > maxss)//找出最大相似度
		{
			maxss = ss;
			testlabel = trainLabel.at<int>(i,0);//从类型列表里取出对应的类型
		}
	}

	return testlabel;
}

//初始链接数据库
BOOL CtargettrackDlg::InitDataBase()
{
	
	char m_szConnect[512];
	char m_szTmp[1024]="" ;
	
		HRESULT hr;
	try
	{
		//连接XdData
		hr = m_Connection.CreateInstance(__uuidof(Connection));
		//用户密码方式
		//sprintf(m_szConnect,"provider = sqloledb;server=%s;database=%s;", m_szHost, m_szDef);//组织连接字符串
		//hr=m_Connection->Open(_bstr_t(m_szConnect),_bstr_t(m_szUser),_bstr_t(m_szPwd),-1);

		//win方式
		sprintf(m_szConnect,"provider = sqloledb;server=%s;database=%s;Integrated Security=SSPI;", m_szHost, m_szDef);//组织连接字符串
		hr=m_Connection->Open(_bstr_t(m_szConnect),"", "", -1);
	
		sprintf(m_szTmp, "数据库连接成功!");
		//连接XdData
	}
	catch(_com_error & e) 
	{
		sprintf(m_szTmp, "数据库打开失败,错误原因：%s\n",LPCTSTR(e.Description()));
		return FALSE ;
	}
	return TRUE;
}

//读取配置信息，数据库信息
void CtargettrackDlg::ReadIni()
{
	CString IniFile = GetFilePath() + "\\配置.ini";//设置配置文件路径

	memset(m_szHost,0,sizeof(m_szHost));
	memset(m_szUser,0,sizeof(m_szUser));
	memset(m_szPwd,0,sizeof(m_szPwd));
	memset(m_szDef,0,sizeof(m_szDef));
	GetPrivateProfileString("数据库", "主机名", NULL, m_szHost, sizeof(m_szHost), IniFile) ;
	GetPrivateProfileString("数据库", "用户名", NULL, m_szUser, sizeof(m_szUser), IniFile) ;
	GetPrivateProfileString("数据库", "密码", NULL, m_szPwd, sizeof(m_szPwd), IniFile) ;
	GetPrivateProfileString("数据库", "数据库名", NULL, m_szDef, sizeof(m_szDef), IniFile) ;

}

//这个函数获取数据库内所有人员信息
void CtargettrackDlg::GetAllPerson()
{
	_variant_t v ;
	_RecordsetPtr m_Rsp ;
	char m_szSql[512] = {0};
	CString str;
	sprintf(m_szSql, "select * from pinfo") ;//组织sql语句
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
			m_personlist.push_back(info);//这条信息 存入列表内
		}
	}
	catch (_com_error & e)
	{
		char m_szTmp[1024] ;
		sprintf(m_szTmp, "执行==>%s<==, 数据库操作失败,错误原因：%s\n",m_szSql, LPCTSTR(e.Description()));
	}
	return;
}