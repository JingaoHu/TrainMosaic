
// trainDlg.h : ͷ�ļ�
//

#pragma once
#include "cv.h"
#include "highgui.h" 
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
using namespace cv;
using namespace std;

// CtrainDlg �Ի���
class CtrainDlg : public CDialogEx
{
// ����
public:
	CtrainDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_TRAIN_DIALOG };

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
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	//int FindPos(vector <int>  * pvnPos,int nPos_min,float fPosdifLimit);
	int FindPos(vector <int>  * pvnPos,int nPos_min,float fPosdifLimit,vector <bool>  * pvbPos = NULL);

public:
	int m_nStart;     //��ʼ֡Ƶ��
	int m_nEnd;       //��ֹ֡Ƶ��
	int m_nPosBasic;  //ƥ��ƫ�ƻ���
	int m_nPosdifW;   //��׼ƫ�Ʒ���
	int m_nPosdifH;
	float m_fLpp;     //ÿ����������ߴ�
	float m_fV;       //�ٶ�
	vector <int> m_vnPos; //ƫ������
};
