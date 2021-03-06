/*
*  stereo_match.cpp
*  calibration
*
*  Created by Victor  Eruhimov on 1/18/10.
*  Copyright 2010 Argus Corp. All rights reserved.
*
*/
#include"stdafx.h"
#include"CommonAlgorithms.h"
#include"SGBM.h"
#include <opencv2/highgui/highgui_c.h>

stereo_SGBM mySGBM;

static void saveXYZ(const char* filename, const Mat& mat);

void init()
{
	mySGBM.extrinsic_filename = "extrinsics.yml";
	mySGBM.intrinsic_filename = "intrinsics.yml";
	mySGBM.init(5,160);
	mySGBM.setSGBM();
 }


int main()
{
	VideoCapture CamR(702);//702 原始1
	VideoCapture CamL(701);//701     0
	int width;
	int height;
	//
	string str_Head = "DepthMap";
	string str_End  = ".xml";

	string str_Head2 = "PointMap";
	string str_End2 = ".txt";
	int iCount = 0;
	//
	//width  = CamL.get(CV_CAP_PROP_FRAME_WIDTH);
	//height = CamL.get(CV_CAP_PROP_FRAME_HEIGHT);
	width = CamL.get(CAP_PROP_FRAME_WIDTH);
	height = CamL.get(CAP_PROP_FRAME_HEIGHT);

	bool bRecording = false;

	//VideoWriter wri_Disp("LeftCam.avi", CV_FOURCC('M', 'J', 'P', 'G'), 25.0, Size(width, height));
	VideoWriter wri_Disp("LeftCam.avi", CAP_OPENCV_MJPEG, 25.0, Size(width, height));

	vector<int> compression_params;
	//compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(9);
	Mat depth8;
	init();
	//main Loop
	while (CamL.isOpened() && CamR.isOpened())
	{
		CamR >> mySGBM.img_R;
		CamL >> mySGBM.img_L;
		//图像矫正
		mySGBM.calibration();
		//显示输入图像
		imshow("左边L", mySGBM.img_L);
		imshow("右边R", mySGBM.img_R);
		
		//key
		//char k = cvWaitKey(10);
		char k = waitKey(10);
		
		if (k == 'c')
		{
			mySGBM.compute();
			imshow("disparity", mySGBM.disp8);
			Mat DepthM(mySGBM.disp8.rows, mySGBM.disp8.cols, CV_16UC1, Scalar::all(0.0f));
			disp2Depth(mySGBM.disp8, DepthM, mySGBM.M); 
			
			imwrite("depth.bmp", mySGBM.disp8);
			DepthM.convertTo(depth8, CV_16UC1);//CV_8U
			//512*424转换
			CvMat Src =DepthM;
			CvMat *pMat = cvGetSubRect(&Src, cvCreateMatHeader(424, 512, CV_16UC1), cvRect(126, 0, 512, 424));
			Mat P0 = cvarrToMat(pMat);//	P
			
			//自己加的
			Mat P;
			P0.convertTo(P,CV_8U); //将图片转为cv_8u显示
			//
			imshow("depth.bmp", P);
			imwrite("depth2.bmp", depth8);
			//数据文件写入
			string fileName = str_Head + to_string(iCount) + str_End;
			FileStorage fs(fileName.c_str(), FileStorage::WRITE);
			fs << "DepthMat" << P;
			fs.release();
			//点云文件写入
			string fileName2 = str_Head2 + to_string(iCount) + str_End2;
			Mat xyz;
			reprojectImageTo3D(mySGBM.disp, xyz, mySGBM.Q, true);
			saveXYZ(fileName2.c_str(), xyz);
			cout << fileName << "写入" << endl;
			iCount++;
		}
		if (k == 'q')
			break;
		
	}
	
	return 0;
}






static void print_help()
{
	printf("\nDemo stereo matching converting L and R images into disparity and point clouds\n");
	printf("\nUsage: stereo_match <left_image> <right_image> [--algorithm=bm|sgbm|hh|sgbm3way] [--blocksize=<block_size>]\n"
		"[--max-disparity=<max_disparity>] [--scale=scale_factor>] [-i=<intrinsic_filename>] [-e=<extrinsic_filename>]\n"
		"[--no-display] [-o=<disparity_image>] [-p=<point_cloud_file>]\n");
}

static void saveXYZ(const char* filename, const Mat& mat)
{
	const double max_z = 1.0e4;
	FILE* fp = fopen(filename, "wt");
	for (int y = 0; y < mat.rows; y++)
	{
		for (int x = 0; x < mat.cols; x++)
		{
			Vec3f point = mat.at<Vec3f>(y, x);
			if (fabs(point[2] - max_z) < FLT_EPSILON || fabs(point[2]) > max_z) continue;
			fprintf(fp, "%f %f %f\n", point[0], point[1], point[2]);
		}
	}
	fclose(fp);
}
/*
int main(int argc, char** argv)
{
	argc = 9;
	char str0[] = "ex";
	char str1[] = "left0.jpg";
	char str2[] = "right0.jpg";
	char str3[] = "--algorithm=sgbm";
	char str4[] = "-i=intrinsics.yml";
	char str5[] = "-e=extrinsics.yml";
	char str6[] = "--max-disparity=160";
	char str7[] = "--blocksize=5";
	char str8[] = "-o=disparity.bmp";

	argv[0] = str0;
	argv[1] = str1;
	argv[2] = str2;
	argv[3] = str3;
	argv[4] = str4;
	argv[5] = str5;
	argv[6] = str6;
	argv[7] = str7;
	argv[8] = str8;

	Mat M;

	std::string img1_filename = "";
	std::string img2_filename = "";
	std::string intrinsic_filename = "";
	std::string extrinsic_filename = "";
	std::string disparity_filename = "";
	std::string point_cloud_filename = "";

	//enum { STEREO_BM = 0, STEREO_SGBM = 1, STEREO_HH = 2, STEREO_VAR = 3, STEREO_3WAY = 4 };
	int alg = STEREO_SGBM;
	int SADWindowSize, numberOfDisparities;
	bool no_display;
	float scale;

	Ptr<StereoBM> bm = StereoBM::create(16, 9);
	Ptr<StereoSGBM> sgbm = StereoSGBM::create(0, 16, 3);
	cv::CommandLineParser parser(argc, argv,
		"{@arg1||}{@arg2||}{help h||}{algorithm||}{max-disparity|0|}{blocksize|0|}{no-display||}{scale|1|}{i||}{e||}{o||}{p||}");
	if (parser.has("help"))
	{
		print_help();
		return 0;
	}
	img1_filename = parser.get<std::string>(0);
	img2_filename = parser.get<std::string>(1);
	if (parser.has("algorithm"))
	{
		std::string _alg = parser.get<std::string>("algorithm");
		alg = _alg == "bm" ? STEREO_BM :
			_alg == "sgbm" ? STEREO_SGBM :
			_alg == "hh" ? STEREO_HH :
			_alg == "var" ? STEREO_VAR :
			_alg == "sgbm3way" ? STEREO_3WAY : -1;
	}
	numberOfDisparities = parser.get<int>("max-disparity");
	SADWindowSize = parser.get<int>("blocksize");
	scale = parser.get<float>("scale");
	no_display = parser.has("no-display");
	if (parser.has("i"))
		intrinsic_filename = parser.get<std::string>("i");
	if (parser.has("e"))
		extrinsic_filename = parser.get<std::string>("e");
	if (parser.has("o"))
		disparity_filename = parser.get<std::string>("o");
	if (parser.has("p"))
		point_cloud_filename = parser.get<std::string>("p");
	if (!parser.check())
	{
		parser.printErrors();
		return 1;
	}
	if (alg < 0)
	{
		printf("Command-line parameter error: Unknown stereo algorithm\n\n");
		print_help();
		return -1;
	}
	if (numberOfDisparities < 1 || numberOfDisparities % 16 != 0)
	{
		printf("Command-line parameter error: The max disparity (--maxdisparity=<...>) must be a positive integer divisible by 16\n");
		print_help();
		return -1;
	}
	if (scale < 0)
	{
		printf("Command-line parameter error: The scale factor (--scale=<...>) must be a positive floating-point number\n");
		return -1;
	}
	if (SADWindowSize < 1 || SADWindowSize % 2 != 1)
	{
		printf("Command-line parameter error: The block size (--blocksize=<...>) must be a positive odd number\n");
		return -1;
	}
	if (img1_filename.empty() || img2_filename.empty())
	{
		printf("Command-line parameter error: both left and right images must be specified\n");
		return -1;
	}
	if ((!intrinsic_filename.empty()) ^ (!extrinsic_filename.empty()))
	{
		printf("Command-line parameter error: either both intrinsic and extrinsic parameters must be specified, or none of them (when the stereo pair is already rectified)\n");
		return -1;
	}

	if (extrinsic_filename.empty() && !point_cloud_filename.empty())
	{
		printf("Command-line parameter error: extrinsic and intrinsic parameters must be specified to compute the point cloud\n");
		return -1;
	}
	VideoCapture CamR(1);
	VideoCapture CamL(2);
	bool out = true;
	int color_mode = alg == STEREO_BM ? 0 : -1;
	Mat img1, img2;
	//Mat img1 = imread(img1_filename, color_mode);
	//Mat img2 = imread(img2_filename, color_mode);
	while (out)
	{
		CamL >> img1;
		CamR >> img2;
		imshow("left", img1);
		char k = cvWaitKey(10);
		if (k == 'c')
		{
			break;
		}

	}
	

	if (img1.empty())
	{
		printf("Command-line parameter error: could not load the first input image file\n");
		return -1;
	}
	if (img2.empty())
	{
		printf("Command-line parameter error: could not load the second input image file\n");
		return -1;
	}

	if (scale != 1.f)
	{
		Mat temp1, temp2;
		int method = scale < 1 ? INTER_AREA : INTER_CUBIC;
		resize(img1, temp1, Size(), scale, scale, method);
		img1 = temp1;
		resize(img2, temp2, Size(), scale, scale, method);
		img2 = temp2;
	}

	Size img_size = img1.size();

	Rect roi1, roi2;
	Mat Q;

	if (!intrinsic_filename.empty())
	{
		// reading intrinsic parameters
		FileStorage fs(intrinsic_filename, FileStorage::READ);
		if (!fs.isOpened())
		{
			printf("Failed to open file %s\n", intrinsic_filename.c_str());
			return -1;
		}

		Mat M1,D1, M2, D2;
		fs["M1"] >> M1;
		fs["D1"] >> D1;
		fs["M2"] >> M2;
		fs["D2"] >> D2;

		M1 *= scale;
		M2 *= scale;

		M1.copyTo(M);
		fs.open(extrinsic_filename, FileStorage::READ);
		if (!fs.isOpened())
		{
			printf("Failed to open file %s\n", extrinsic_filename.c_str());
			return -1;
		}

		Mat R, T, R1, P1, R2, P2;
		fs["R"] >> R;
		fs["T"] >> T;

		stereoRectify(M1, D1, M2, D2, img_size, R, T, R1, R2, P1, P2, Q, CALIB_ZERO_DISPARITY, -1, img_size, &roi1, &roi2);

		Mat map11, map12, map21, map22;
		initUndistortRectifyMap(M1, D1, R1, P1, img_size, CV_16SC2, map11, map12);
		initUndistortRectifyMap(M2, D2, R2, P2, img_size, CV_16SC2, map21, map22);

		Mat img1r, img2r;
		remap(img1, img1r, map11, map12, INTER_LINEAR);
		remap(img2, img2r, map21, map22, INTER_LINEAR);

		img1 = img1r;
		img2 = img2r;
	
	
	}

	numberOfDisparities = numberOfDisparities > 0 ? numberOfDisparities : ((img_size.width / 8) + 15) & -16;
	//numberOfDisparities= ((img_size.width / 8) + 15) & -16;
	bm->setROI1(roi1);
	bm->setROI2(roi2);
	bm->setPreFilterCap(63);
	bm->setBlockSize(SADWindowSize > 0 ? SADWindowSize : 9);
	bm->setMinDisparity(0);
	bm->setNumDisparities(numberOfDisparities);
	bm->setTextureThreshold(10);
	bm->setUniquenessRatio(10);
	bm->setSpeckleWindowSize(100);
	bm->setSpeckleRange(32);
	bm->setDisp12MaxDiff(1);

	sgbm->setPreFilterCap(63);
	int sgbmWinSize = SADWindowSize > 0 ? SADWindowSize : 3;
	sgbm->setBlockSize(sgbmWinSize);

	int cn = img1.channels();
	//8 32
	sgbm->setP1(8 * cn*sgbmWinSize*sgbmWinSize);
	sgbm->setP2(128 * cn*sgbmWinSize*sgbmWinSize);
	sgbm->setMinDisparity(0);
	sgbm->setNumDisparities(numberOfDisparities);
	sgbm->setUniquenessRatio(10);
	sgbm->setSpeckleWindowSize(100);
	sgbm->setSpeckleRange(32);
	sgbm->setDisp12MaxDiff(1);
	if (alg == STEREO_HH)
		sgbm->setMode(StereoSGBM::MODE_HH);
	else if (alg == STEREO_SGBM)
		sgbm->setMode(StereoSGBM::MODE_SGBM);
	else if (alg == STEREO_3WAY)
		sgbm->setMode(StereoSGBM::MODE_SGBM_3WAY);

	Mat disp, disp8;
//	Mat img1p, img2p, dispp;
//	copyMakeBorder(img1, img1p, 0, 0, numberOfDisparities, 0, IPL_BORDER_REPLICATE);
//	copyMakeBorder(img2, img2p, 0, 0, numberOfDisparities, 0, IPL_BORDER_REPLICATE);

	int64 t = getTickCount();
	if (alg == STEREO_BM)
		bm->compute(img1, img2, disp);
	else if (alg == STEREO_SGBM || alg == STEREO_HH || alg == STEREO_3WAY)
		sgbm->compute(img1, img2, disp);
	t = getTickCount() - t;
	printf("Time elapsed: %fms\n", t * 1000 / getTickFrequency());
	
//	disp = dispp.colRange(numberOfDisparities, img1p.cols);
//	disp = abs(disp);
	if (alg != STEREO_VAR)
		disp.convertTo(disp8, CV_8U, 255 / (numberOfDisparities*16.));
	else
		disp.convertTo(disp8, CV_8U);//CV_8U
	insertDepth32f(disp8);
	Mat DepthM(disp8.rows,disp8.cols, CV_16UC1);
	disp2Depth(disp8, DepthM, M);

	if (!no_display)
	{
		namedWindow("left", 1);
		imshow("left", img1);
		namedWindow("right", 1);
		imshow("right", img2);
		namedWindow("disparity", 0);
		namedWindow("depth", 0);
		imshow("disparity", disp8);
		imshow("depth", DepthM);
		printf("press any key to continue...");
		fflush(stdout);
		printf("\n");
	}

	if (!disparity_filename.empty())
		imwrite(disparity_filename, disp8);

	if (!point_cloud_filename.empty())
	{
		printf("storing the point cloud...");
		fflush(stdout);
		Mat xyz;
		reprojectImageTo3D(disp, xyz, Q, true);
		saveXYZ(point_cloud_filename.c_str(), xyz);
		printf("\n");
	}
	waitKey();
	//int i;
	//cin >> i;
	return 0;
}
*/

