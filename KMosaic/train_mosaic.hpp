#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include<fstream>
using namespace cv;
using namespace std;

//////////////////////////////////////////////////////// Train_Mosaic class declaration   /////////////////////////////////////////////////////////
class Train_Mosaic
{

private:
	char *m_video_path; // video path
	VideoCapture capture;
	
	//video information---------------------------------------------------
	int frame_w;
	int frame_h;
	
	double rate;
	long totalFrameNumber;
	
	//template�������,ע��template��x����ʼ��λ������λ��-----------------------------------------------------------
	float temp_aspect_ratio; //template��߱�        
	int temp_h,temp_w; //template�Ŀ�͸�     
	int temp_x0,temp_y0;
	
	//------------------------------------------------------
	Mat frame;            //��ǰ֡ͼ��
	Mat frame_c1;	      //��ɫ֡ͼ��
	Mat temp;          //ģ��
	Mat temp_c1;       //��ɫģ��
	Mat result;           //ƥ����map
	
	//-----------------��׼���---------------------------
	int *match_result;


	
public:
	Train_Mosaic(char* video_path,int h_min,int h_max)  // construction
		{
			//��ʼ����Ƶ�ļ�·��
			m_video_path=video_path;
			
			//��ʼ��ģ�����
			temp_aspect_ratio=1.0;
			temp_h  = h_max-h_min;
			temp_w  = temp_h*temp_aspect_ratio; 		
			
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
			
			totalFrameNumber = capture.get(CV_CAP_PROP_FRAME_COUNT);
			match_result=(int*)malloc(sizeof(int)*totalFrameNumber);//���Դ洢��׼�������������
			
			capture.read(frame);
			frame_w = frame.cols;
			frame_h = frame.rows;
			
			temp_x0=(frame_w-temp_w)/2;
			temp_y0=h_min;
						
			capture.release();//�ͷ���Ƶ
		}
		
	void rough_mosaic();
	void finetuned_mosaic();
	void print_match_result(char*file_path)
		{
			ofstream ofile;
			ofile.open(file_path);
			for(int i=0;i<totalFrameNumber;i++)
				ofile<<match_result[i]<<endl;
			ofile.close();
		}

};

///////////////////////////////////////////////////////////  rough mosaic  //////////////////////////////////////////////////////////

void Train_Mosaic::rough_mosaic()
{

	
	
	capture=VideoCapture(m_video_path);
	int currentFrame=0;
		
	//����whileѭ����ȡ֡������ƴ��-----------------------------------------
	Rect rect(temp_x0,temp_y0, temp_w, temp_h); 
		                       //class Rect(x,y,w,h),(x,y)is the upperleft point and w,h are width & height
	frame(rect).copyTo(temp);
	
	while(true)
	{
		if(!capture.read(frame))
		{
			cout<<"Fail to read frame��"<<endl;
		}
		
		
		
		//ͼ��֡�ҶȻ�
		frame_c1.create(frame.rows,frame.cols,CV_8UC1 );
		cvtColor(frame,frame_c1,CV_BGR2GRAY);
			
		temp_c1.create(temp.rows,temp.cols,CV_8UC1 );
		cvtColor(temp,temp_c1,CV_BGR2GRAY);
	
		//ģ��ƥ��
		result.create(frame.cols - temp.cols + 1, frame.rows - temp.rows + 1, CV_32FC1 );
		matchTemplate(temp_c1, frame_c1, result, CV_TM_SQDIFF_NORMED);

			//Ѱ�Ҽ�ֵ
		double minVal,maxVal;
		Point minLoc,maxLoc;			
		minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );

	    match_result[currentFrame]=minLoc.x;
	    
	    //��ȡģ��
		Rect rect(temp_x0,temp_y0, temp_w, temp_h); 
		                       //class Rect(x,y,w,h),(x,y)is the upperleft point and w,h are width & height
		frame(rect).copyTo(temp);
		
		currentFrame++;
		if(currentFrame >= totalFrameNumber)
			break;
	
	}

	//����׼�����ֵ�˲�
	mid_filter(match_result,totalFrameNumber,11);
	
	//�ر���Ƶ�ļ�
	capture.release();
	
	cout<<"Rough mosaic done!"<<endl;
}

