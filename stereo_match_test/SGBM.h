#pragma once
#include"stdafx.h"
#include"CommonAlgorithms.h"

class stereo_SGBM
{
private:
	Ptr<StereoSGBM> sgbm;
	int SADWindowSize, numberOfDisparities;
	//
	Mat M1, D1, M2, D2;
	Mat R, T, R1, P1, R2, P2;
	Rect roi1, roi2;
	Size img_size;
	
public:
	stereo_SGBM();
	Mat M,Q;
	std::string intrinsic_filename = "";
	std::string extrinsic_filename = "";
	float scale;
	Mat img_L, img_R;//输入
	Mat disp, disp8;//输出
	int cn;//图像频道数目
	int alg;
	//
	int init(int blocksize,int maxdis);
	void calibration();
	void setSGBM();
	void compute();
};