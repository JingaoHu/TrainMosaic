#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>

using namespace cv;
using namespace std;

//////////////////////////////////////////////////////// Train_Mosaic class declaration   /////////////////////////////////////////////////////////
class Train_Mosaic
{

private:
	char *m_video_path; // video path
	int m_nStart;     //��ʼ֡Ƶ��
	int m_nEnd;       //��ֹ֡Ƶ��
	int currentFrame;
	int m_nPosBasic;  //ƥ��ƫ�ƻ���
	int m_nPosdifW;   //��׼ƫ�Ʒ���,�����޳������
	int m_nPosdifH;
	float m_fLpp;     //ÿ����������ߴ�
	float m_fV;       //�ٶ�
	vector <int> m_vnPos; //ƫ������
	VideoCapture capture;
	
	//video information---------------------------------------------------
	int w;
	int h;
	int w0;
	int h0;
	
	double rate;
	int delay;
	long totalFrameNumber;
	
	//ROI�������-----------------------------------------------------------
    float m_fw;     //ROI�ĺ������          
	float m_fh;     //ROI���������
	int m_h_pos;	   //��������ƫ150
	int m_w_pos;			//����ƫ
	
	//------------------------------------------------------
	Mat frame;            //��ǰ֡ͼ��
	Mat frame_c1;	      //��ɫģ��
	Mat frame_t;          //ƴ��ģ��ͼ��
	Mat frame_t_c1;       //��ɫģ��
	Mat result;

	vector<int> vnPos;    //ƴ��ƫ��
	vector<bool> vbPos;   //ƴ��ƫ�Ʊ�ʾ
	vector <Mat> vFrameROI;  //roi����
	
	int nPos_min;         //ƫ�Ƶ����ޣ��ݴ˿��Լ����ʻ���ʻ����λ��
	bool stop;             //������Ƶ��֡�Ķ���
	
public:
	Train_Mosaic(char* video_path)  // construction
		{
			m_video_path=video_path;
			
			m_fw = 1.0f/3.0f;          
			m_fh = 1.8f/3.0f;    
			m_h_pos = -37;		
			m_w_pos = 0;
			
			//load video----------------------------------------------------
			capture=VideoCapture(m_video_path);
			if(!capture.isOpened())
				{
				cout<<"Fail to load video!"<<endl;
				return ;
				}
			else cout<<"Success to load video!"<<endl;	
			
			rate = capture.get(CV_CAP_PROP_FPS);
			rate=30.0;
					
			delay = 1000/rate;//��֡��ļ��ʱ��:
			
			totalFrameNumber = capture.get(CV_CAP_PROP_FRAME_COUNT);
			
			nPos_min=10;
			
			capture.read(frame);
			w = frame.cols;
			h = frame.rows;
			w0 = int(w*(1.0f-m_fw)/2.0f)+m_w_pos;
			h0 = int(h*(1.0f-m_fh)/2.0f)+m_h_pos;	
			
			capture.release();
		}
		
	void rough_mosaic();
	void finetuned_mosaic();
	int FindPos(vector <int>  * pvnPos,int nPos_min,float fPosdifLimit,vector <bool>  * pvbPos = NULL);


};



///////////////////////////////////////////////////////////////////////  ��ƥ��   //////////////////////////////////////////////////////////////////////////

void Train_Mosaic::rough_mosaic()
{

	//ƴ��ƫ�Ʋ���-----------------------------------------------------------

	float fPosdifLimit = 5.0f;   //ƫ����Χ
	m_nPosdifW = 20;
	m_nPosdifH = 20;

	stop = false;    
	m_nStart = 0;//��ʼ֡Ƶλ��
	
	
	capture=VideoCapture(m_video_path);
	capture.set( CV_CAP_PROP_POS_FRAMES,m_nStart);

	
		
	//����whileѭ����ȡ֡������ƴ��-----------------------------------------
	bool bActive = false;

	currentFrame = m_nStart;
	
	while(!stop)
	{
		if(!capture.read(frame))
		{
			cout<<"Fail to read frame��"<<endl;
		}


		
		Point Loc;

		//step1.ģ��ƥ��----------------------------------------------------------------
		if(bActive)
		{
			//�ҶȻ�
			frame_t_c1.create(frame_t.rows,frame_t.cols,CV_8UC1 );
			cvtColor(frame_t,frame_t_c1,CV_BGR2GRAY);
	
			frame_c1.create(frame.rows,frame.cols,CV_8UC1 );
			cvtColor(frame,frame_c1,CV_BGR2GRAY);

			//ģ��ƥ��
			result.create(frame.cols - frame_t.cols + 1, frame.rows - frame_t.rows + 1, CV_32FC1 );
			matchTemplate(frame_t_c1, frame_c1, result, CV_TM_SQDIFF_NORMED);

			//Ѱ�Ҽ�ֵ
			double minVal,maxVal;
			Point minLoc,maxLoc;			
			minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
			Loc = minLoc;
			
			int npos =  Loc.x - w0;
			vnPos.push_back(npos);
			vbPos.push_back(true);

			//��ʾλ��
			Point p;
			p.x = Loc.x + frame_t.cols;
			p.y = Loc.y + frame_t.rows;
			Mat show;
			frame.copyTo(show);
			
			rectangle(show,Loc,p,Scalar(0,0,255),2,8,0); //�ڼ��λ�û���ɫ��
			//cout<<"current frame:"<<currentFrame<<endl;
		}		

	    

		//step2.��ȡģ��----------------------------------------------------------------		
		Rect rect(w0, h0, int(w*m_fw), int(h*m_fh));    
		frame(rect).copyTo(frame_t);
		bActive = true;		
		
		currentFrame++;
		if(currentFrame >= totalFrameNumber)
			stop=true;
	
	}
	

	
	
	int nAve = FindPos(&vnPos,nPos_min,fPosdifLimit,&vbPos);

	//�����޳������
	int n = vnPos.size();
	float fAveEx = 0.0f;
	int num = 0;
	for(int i=0;i<n;i++)

	{
		if(vbPos[i])
		{
			float fTmp = abs(vnPos[i]-nAve);
			if(fTmp>m_nPosdifW)
			{
				vbPos[i] = false;
			}

			else
			{
				fAveEx += vnPos[i];
				num++;

			}
		}
	}
	fAveEx = fAveEx/float(num);
	m_nPosBasic = int(fAveEx);


	//compute m_nstart (train head) and m_nEnd (train tail)-------------------------------------

	bool ishead = true;

	for(int i=0;i<n;i++)
	{
		if(ishead && abs(vnPos[i])>nPos_min)
		{
			m_nStart = i;
			ishead = false;	
		}
		if(!ishead && abs(vnPos[i])<nPos_min)
		{
			m_nEnd=i;
			break;
		}
	}
	//
	for(int i=0;i<n;i++)
		m_vnPos.push_back(vnPos[i]);

	
	//show train head and tail-------------------------------------------------------------------

	Point p;
	p.x = w0 + frame_t.cols;
	p.y = h0 + frame_t.rows;
	
	Point p0;
	p0.x = w0;
	p0.y = h0;


	capture.set( CV_CAP_PROP_POS_FRAMES,m_nStart);
	capture.read(frame);
	rectangle(frame,p0,p,Scalar(0,0,255),2,8,0); 
	imshow("head",frame);

	capture.set( CV_CAP_PROP_POS_FRAMES,m_nEnd);
	capture.read(frame);
	rectangle(frame,p0,p,Scalar(0,0,255),2,8,0); 
	imshow("tail",frame);



	waitKey(0);

	//�ر���Ƶ�ļ�
	capture.release();
	
	cout<<"Rough mosaic done!"<<endl;
}

//////////////////////////////////////////////////////////////////  finetuned mosaic   /////////////////////////////////////////////////////////////////
void Train_Mosaic::finetuned_mosaic()
{
	//����-----------------------------------------------------------------
	int rols_gray_th = 20;      //����ģ�����1 �����÷��������ñ������仯�����ɿ���
	float rols_num_th = 0.3f;   //����ģ�����2
	m_nPosdifW = 3;
	m_nPosdifH = 10;
	float fPosdifLimit = 2.0f;   //ƫ����Χ
	int nFrameNum = 50;          //�ȶ�֡Ƶ�İ���֡��


	//���ÿ�ʼ֡,��ȡ֡����֡Ƶ--------------------------------------------
	stop = false;    //��Ƶ�˳�����	
	capture.set( CV_CAP_PROP_POS_FRAMES,0); //��ʼ֡Ƶλ��
	
	
		
	//����whileѭ����ȡ֡������ƴ��-----------------------------------------
	bool bActive = false;
	bool bCutT = true;
	long currentFrame = 0;


	//��ȡ����ͼ��	
	capture=VideoCapture(m_video_path);
	
	Mat frame_bg_c1;      //��ͨ�� ģ�� ����ͼ��
	capture.read(frame);	
	int	rows = int(h*m_fh);
	Mat tmp;
	Rect rect_bg(w0, h0, int(w*m_fw), int(h*m_fh));
	frame(rect_bg).copyTo(tmp);
	cvtColor(tmp,frame_bg_c1,CV_BGR2GRAY);

	capture.set(CV_CAP_PROP_POS_FRAMES,0); //��λ

	//��ʾ
//	CvFont font;    
    double hScale=1;   
    double vScale=0.5; 
       
//    int lineWidth=2;    
//    cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);//��ʼ�����壬׼��д��ͼƬ�ϵ� 
	cout<<"before while..."<<endl;    



//step1 ģ��ƥ�� and step2 ��ȡģ��---------------------------------------------------------------------
	while(!stop)
	{
		//��ȡ��һ֡
		if(!capture.read(frame))
		{
			cout<<"Fail to read frame!"<<endl;
			return ;
		}
		
		//��ʾͼ��
		
			
		Point Loc;

		//step1.ģ��ƥ��----------------------------------------------------------------����ͳ�����һ��
		if(bActive && currentFrame>=m_nStart+3 &&  currentFrame<=m_nEnd)
		{
			//�ҶȻ�
			frame_t_c1.create(frame_t.rows,frame_t.cols,CV_8UC1 );
			cvtColor(frame_t,frame_t_c1,CV_BGR2GRAY);
			Mat frame_c1;
			frame_c1.create(frame.rows,frame.cols,CV_8UC1 );
			cvtColor(frame,frame_c1,CV_BGR2GRAY);
			


			//Ŀ��������
			Mat frame_t_cut_c1;
			int w_LR = 0;
			int j_th_L = 0;
			int j_th_R = 0;
			
			
				//����ģ��	��ģ���еı�����һ��ȥ����			
			for(int j=0;j<frame_t.cols;j++)			
			{
				int nCount = 0;
				for(int i=0;i<frame_t.rows;i++)
				{
					float diff = abs(frame_t_c1.ptr<uchar>(i)[j] - frame_bg_c1.ptr<uchar>(i)[j]);
					if(diff>rols_gray_th)
						nCount++;
				}
				if(nCount>int(rols_num_th*frame_t.rows))
				{
					j_th_L = j;
					break;
				}
			}
			for(int j=frame_t.cols-1;j>=0;j--)			
			{
				int nCount = 0;
				for(int i=0;i<frame_t.rows;i++)
				{
					float diff = abs(frame_t_c1.ptr<uchar>(i)[j] - frame_bg_c1.ptr<uchar>(i)[j]);
					if(diff>rols_gray_th)
						nCount++;
				}
				if(nCount>int(rols_num_th*frame_t.rows))
				{
					j_th_R = j;
					break;
				}
			}
			w_LR = j_th_R-j_th_L+1;				
			
			
			frame_t_cut_c1.create(frame_t.rows,w_LR,CV_8UC1 );
			Rect rect_cut(j_th_L, 0, w_LR, frame_t.rows);
			frame_t_c1(rect_cut).copyTo(frame_t_cut_c1);
		//	imshow("frame_t_cut_c1",frame_t_cut_c1);
		//	waitKey(0);

			//���泵ͷ
			if(currentFrame==m_nStart+1)
			{
				Mat head;
				frame_t(rect_cut).copyTo(head);
				vFrameROI.push_back(head);
			}
	

			
			
			int n1 = currentFrame-nFrameNum/2;
			if(n1<m_nStart)
				n1 = m_nStart+1;
			int n2 = n1 + nFrameNum -1;
			if(n2>m_nEnd)
				{
				n2 = m_nEnd-1;
				n1 = n2 - nFrameNum +1;
				}
			for(int i=n1;i<=n2;i++)
			{
				vnPos.push_back(m_vnPos[i]);
				vbPos.push_back(true);
			}
		
			int nPosBasic = FindPos(&vnPos,nPos_min,fPosdifLimit,&vbPos);		
			int nw_pos = w0+nPosBasic-m_nPosdifW;
			//int nw_pos = w0+m_nPosBasic-m_nPosdifW;
			int nh_pos = h0-m_nPosdifH;
			Mat frame_limit_c1;
			Rect rect_limit(nw_pos, nh_pos, int(w*m_fw)+2*m_nPosdifW, int(h*m_fh)+2*m_nPosdifH);    
			frame_c1(rect_limit).copyTo(frame_limit_c1);

			
			
			//ģ��ƥ��
			//matchTemplate(frame_t_g, frame_g, result, CV_TM_SQDIFF_NORMED);
			//matchTemplate(frame_t_cut_c1, frame_limit_c1, result, CV_TM_CCOEFF_NORMED);
			matchTemplate(frame_t_cut_c1, frame_limit_c1, result, CV_TM_SQDIFF_NORMED);
			

			//Ѱ�Ҽ�ֵ
			double minVal,maxVal;
			Point minLoc,maxLoc;			
			minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
			//Loc = maxLoc;
			Loc = minLoc;
		

			//����֡Ƶ
			int npos =  nw_pos + Loc.x - (w0+j_th_L);
			vnPos.push_back(npos);

			Mat ROI;
			Rect rectROI;
			rectROI.y = nh_pos + Loc.y -1; //Ϊ�����������ƶ�1�����أ�
			rectROI.height = int(h*m_fh);	
			if(m_nPosBasic>0)   //����
			{
				rectROI.x = w0;
				rectROI.width = npos;
			}
			else                //�ҵ���
			{
				rectROI.x = w0+frame_t.cols-npos-1;
				rectROI.width = npos;
			}
			frame(rectROI).copyTo(ROI);

			//��ʾ
			IplImage * t_rs = cvCreateImage(ROI.size(),8,3);
			int hh = ROI.size().height;
			int ww = ROI.size().width;
			for(int i=0;i<hh;i++)
			{
				for(int j=0;j<ww;j++)
				{
					((uchar*)( t_rs->imageData + i * t_rs->widthStep))[j] = ROI.ptr<uchar>(i)[j];
				}
			}

/*
			CString strw;
			strw.Format(_T("%d"),currentFrame);
			char * chw1 = T2A(strw);
			cvPutText(t_rs,chw1,cvPoint(0,50),&font,CV_RGB(255,0,0));//��ͼƬ������ַ�			
			strw.Format(_T("%d"),npos);
			char * chw2 = T2A(strw);
			cvPutText(t_rs,chw2,cvPoint(0,150),&font,CV_RGB(255,0,0));//��ͼƬ������ַ�
*/

			
			for(int i=0;i<hh;i++)
			{
				for(int j=0;j<ww;j++)
				{
					ROI.ptr<uchar>(i)[j] = ((uchar*)( t_rs->imageData + i * t_rs->widthStep))[j];
				}
			}
			cvReleaseImage(&t_rs);

			//����
			vFrameROI.push_back(ROI);	

			//��ʾλ��
			Point p1,p2;
			p1.x = nw_pos + Loc.x;
			p1.y = nh_pos + Loc.y;
			p2.x = nw_pos + Loc.x + frame_t.cols;
			p2.y = nh_pos + Loc.y + frame_t.rows;
			Mat show;
			frame.copyTo(show);
			rectangle(show,p1,p2,Scalar(0,0,255),2,8,0); //�ڼ��λ�û���ɫ��
			
			imshow("match",show);
			waitKey(delay);
			//waitKey(0);
			//cout<<"step1 success"<<endl;
		}		


		//step2.��ȡģ��----------------------------------------------------------------		
		Rect rect(w0, h0, int(w*m_fw), int(h*m_fh));    
		frame(rect).copyTo(frame_t);
		
		bActive = true;	
		currentFrame++;
		if(currentFrame>=totalFrameNumber) stop=true;
		
		//cout<<"step2 success!"<<endl;
	
	}
	

	//step3.ƴ�ϳ���------------------------------------------------------------------------------------------------
	Mat dst;                 //ƴ�ӵ�ͼ��
	int ns = vFrameROI.size();	
	int clos = 0;
	for(int k=0;k<ns;k++)
	{
		clos += vFrameROI[k].cols;		
	}
	dst.create(rows,clos,CV_8UC3);

	if(m_nPosBasic>0)   //����
	{
		int nTmp = 0;
		for(int k=0;k<ns;k++)
		{
			int j_pos = 0;
			nTmp += vFrameROI[k].cols;
			j_pos = clos - nTmp;
			for(int i=0;i<vFrameROI[k].rows;i++)
			{
				for(int j=0;j<vFrameROI[k].cols;j++)
				{
					dst.ptr<uchar>(i)[(j+j_pos)*3] = vFrameROI[k].ptr<uchar>(i)[j*3];
					dst.ptr<uchar>(i)[(j+j_pos)*3+1] = vFrameROI[k].ptr<uchar>(i)[j*3+1];
					dst.ptr<uchar>(i)[(j+j_pos)*3+2] = vFrameROI[k].ptr<uchar>(i)[j*3+2];
				}
			}		
		}
	}
	else               //�ҵ���
	{
		int j_pos = 0;
		for(int k=0;k<ns;k++)
		{
			for(int i=0;i<vFrameROI[k].rows;i++)
			{
				for(int j=0;j<vFrameROI[k].cols;j++)
				{
					dst.ptr<uchar>(i)[(j+j_pos)*3] = vFrameROI[k].ptr<uchar>(i)[j*3];
					dst.ptr<uchar>(i)[(j+j_pos)*3+1] = vFrameROI[k].ptr<uchar>(i)[j*3+1];
					dst.ptr<uchar>(i)[(j+j_pos)*3+2] = vFrameROI[k].ptr<uchar>(i)[j*3+2];
				}
			}
			j_pos += vFrameROI[k].cols;
		}
	}	
	imwrite("/home/jingao/train_mosaic/train.bmp",dst);	

	//�ر���Ƶ�ļ�
	capture.release();
	cout<<"Finetuned mosaic done!"<<endl;
}

///////////////////////////////////////////////////////////////////////////		����			/////////////////////////////////////////////////////////////////////


int Train_Mosaic::FindPos(vector <int>  * pvnPos,int nPos_min,float fPosdifLimit,vector <bool>  * pvbPos)

{
	int n = pvnPos->size();
	//�޳���ֹλ��
	for(int i=0;i<n;i++)
	{
		if(abs((*pvnPos)[i])<=nPos_min)
		{
			if(pvbPos!=NULL)
				(*pvbPos)[i] = false;
		}
	}
	//�����޳������
	float fAve = 0.0f;
	float fDif = 0.0f;
	int num = 0;
	while(1)
	{
		//�����ֵ
		fAve = 0.0f;
		num = 0;
		for(int i=0;i<n;i++)
		{
			if((*pvbPos)[i])
			{
				fAve += (*pvnPos)[i];
				num++;
			}
		}
		fAve = fAve/float(num);

		//�޳���������
		float fMaxDif = 0.0f;
		int nMaxDifIndex = 0;
		for(int i=0;i<n;i++)
		{
			if((*pvbPos)[i])
			{
				float fTmp = abs((*pvnPos)[i]-fAve);
				if(fTmp>fMaxDif)
				{
					fMaxDif = fTmp;
					nMaxDifIndex = i;
				}
			}
		}
		(*pvbPos)[nMaxDifIndex] = false;

		//����ƫ�����
		fDif = 0.0f;
		num = 0;
		for(int i=0;i<n;i++)
		{
			if((*pvbPos)[i])
			{
				fDif += abs((*pvnPos)[i]-fAve);
				num++;
			}
		}
		fDif = fDif/float(num);

		//��������
		if(fDif<fPosdifLimit) break;
	
	}
	
	return int(fAve);
}

