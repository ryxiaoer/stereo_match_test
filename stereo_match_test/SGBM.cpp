#include"stdafx.h"
#include"SGBM.h"

stereo_SGBM::stereo_SGBM()
{
	intrinsic_filename = "";
	extrinsic_filename = "";
	scale = 1.0f;
	alg = 1;
	img_size.width  = 640;
	img_size.height = 480;
	cn = 3;
}

//初始化
int stereo_SGBM::init(int blocksize, int maxdis)
{
	sgbm = StereoSGBM::create(0, 16, 3);
	SADWindowSize       = blocksize;
	numberOfDisparities = maxdis;
	///////////////////////////////////////////////////////////////
	//读取标定文件
	if (intrinsic_filename.empty() || extrinsic_filename.empty())
		return -1;
	//-----------------------------------
	//intrinsic
	FileStorage fs(intrinsic_filename, FileStorage::READ);
	if (!fs.isOpened())
	{
		printf("Failed to open file %s\n", intrinsic_filename.c_str());
		return -1;
	}

	fs["M1"] >> M1;
	fs["D1"] >> D1;
	fs["M2"] >> M2;
	fs["D2"] >> D2;

	M1 *= scale;
	M2 *= scale;

	M1.copyTo(M);
	//-----------------------------------------
	//extrinsic
	fs.open(extrinsic_filename, FileStorage::READ);
	if (!fs.isOpened())
	{
		printf("Failed to open file %s\n", extrinsic_filename.c_str());
		return -1;
	}
	fs["R"] >> R;
	fs["T"] >> T;
	stereoRectify(M1, D1, M2, D2, img_size, R, T, R1, R2, P1, P2, Q, CALIB_ZERO_DISPARITY, -1, img_size, &roi1, &roi2);
	//////////////////////////////////////////////////////////////////////
	return 0;
}

//图像标定矫正
void stereo_SGBM::calibration()
{
	Mat map11, map12, map21, map22;
	initUndistortRectifyMap(M1, D1, R1, P1, img_size, CV_16SC2, map11, map12);
	initUndistortRectifyMap(M2, D2, R2, P2, img_size, CV_16SC2, map21, map22);

	Mat img1r, img2r;
	remap(img_L, img1r, map11, map12, INTER_LINEAR);
	remap(img_R, img2r, map21, map22, INTER_LINEAR);

	img_L = img1r;
	img_R = img2r;
}

void stereo_SGBM::setSGBM()
{
	//修正disparities参数
	int sgbmWinSize = SADWindowSize > 0 ? SADWindowSize : 3;
	numberOfDisparities = numberOfDisparities > 0 ? numberOfDisparities : ((img_size.width / 8) + 15) & -16;
	sgbm->setPreFilterCap(63);	
	sgbm->setBlockSize(sgbmWinSize);
	sgbm->setP1(8   * cn*sgbmWinSize*sgbmWinSize);
	sgbm->setP2(128 * cn*sgbmWinSize*sgbmWinSize);
	sgbm->setMinDisparity(0);
	sgbm->setNumDisparities(numberOfDisparities);
	sgbm->setUniquenessRatio(10);
	sgbm->setSpeckleWindowSize(100);
	sgbm->setSpeckleRange(32);
	sgbm->setDisp12MaxDiff(1);
	//
	if (alg == STEREO_HH)
		sgbm->setMode(StereoSGBM::MODE_HH);
	else if (alg == STEREO_SGBM)
		sgbm->setMode(StereoSGBM::MODE_SGBM);
	else if (alg == STEREO_3WAY)
		sgbm->setMode(StereoSGBM::MODE_SGBM_3WAY);
}

void stereo_SGBM::compute()
{
	sgbm->compute(img_L, img_R, disp);
	if (alg != STEREO_VAR)
		disp.convertTo(disp8, CV_8U, 255 / (numberOfDisparities*16.));
	else
		disp.convertTo(disp8, CV_8U);//CV_8U
	insertDepth32f(disp8);
}