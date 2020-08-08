#include"stdafx.h"
#include"CommonAlgorithms.h"

void insertDepth32f(cv::Mat& depth)
{
	const int width = depth.cols;
	const int height = depth.rows;
	uchar*  data = (uchar*)depth.data;
	cv::Mat integralMap = cv::Mat::zeros(height, width, CV_64F);
	cv::Mat ptsMap = cv::Mat::zeros(height, width, CV_32S);
	double* integral = (double*)integralMap.data;
	int* ptsIntegral = (int*)ptsMap.data;
	memset(integral, 0, sizeof(double) * width * height);
	memset(ptsIntegral, 0, sizeof(int) * width * height);

	for (int i = 0; i < height; ++i)
	{
		int id1 = i * width;
		for (int j = 0; j < width; ++j)
		{
			int id2 = id1 + j;

			if ((double)data[id2] > 1e-3)
			{
				integral[id2] = (double)data[id2];
				ptsIntegral[id2] = 1;
			}
		}
	}
	// 积分区间
	for (int i = 0; i < height; ++i)
	{
		int id1 = i * width;
		for (int j = 1; j < width; ++j)
		{
			int id2 = id1 + j;
			integral[id2] += integral[id2 - 1];
			ptsIntegral[id2] += ptsIntegral[id2 - 1];
		}
	}
	for (int i = 1; i < height; ++i)
	{
		int id1 = i * width;
		for (int j = 0; j < width; ++j)
		{
			int id2 = id1 + j;
			integral[id2] += integral[id2 - width];
			ptsIntegral[id2] += ptsIntegral[id2 - width];
		}
	}
	int wnd;
	double dWnd = 2;
	while (dWnd > 1)
	{
		wnd = int(dWnd);
		dWnd /= 2;
		for (int i = 0; i < height; ++i)
		{
			int id1 = i * width;
			for (int j = 0; j < width; ++j)
			{
				int id2 = id1 + j;
				int left = j - wnd - 1;
				int right = j + wnd;
				int top = i - wnd - 1;
				int bot = i + wnd;
				left = max(0, left);
				right = min(right, width - 1);
				top = max(0, top);
				bot = min(bot, height - 1);
				int dx = right - left;
				int dy = (bot - top) * width;
				int idLeftTop = top * width + left;
				int idRightTop = idLeftTop + dx;
				int idLeftBot = idLeftTop + dy;
				int idRightBot = idLeftBot + dx;
				int ptsCnt = ptsIntegral[idRightBot] + ptsIntegral[idLeftTop] - (ptsIntegral[idLeftBot] + ptsIntegral[idRightTop]);
				double sumGray = integral[idRightBot] + integral[idLeftTop] - (integral[idLeftBot] + integral[idRightTop]);
				if (ptsCnt <= 0)
				{
					continue;
				}
				data[id2] = float(sumGray / ptsCnt);
			}
		}
		int s = wnd / 2 * 2 + 1;
		if (s > 201)
		{
			s = 201;
		}
		//	cv::GaussianBlur(depth, depth, cv::Size(s, s), s, s);
	}
}

void disp2Depth(cv::Mat dispMap, cv::Mat &depthMap, cv::Mat K)
{
	int type = dispMap.type();

	float fx = K.at<double>(0, 0);
	float fy = K.at<double>(1, 1);
	float cx = K.at<double>(0, 2);
	float cy = K.at<double>(1, 2);


	float baseline = 100.0f; //基线距离65mm

	if (type == CV_8U)
	{
		const float PI = 3.14159265358;
		int height = dispMap.rows;
		int width = dispMap.cols;

		uchar*  dispData = (uchar*)dispMap.data;
		ushort* depthData = (ushort*)depthMap.data;

		int co = 0;
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				int id = i * width + j;
				if (!dispData[id])
				{
					float data = 0.0f;
					depthData[id] = ushort(data);
					continue;  //防止0除
				}
				float data = (float)fx *baseline / ((float)dispData[id])*10.0f;
				if (data >= 65535)
					data = 65535;
				depthData[id] = ushort(data);
			}
		}
	}
	else
	{
		cout << "please confirm dispImg's type!" << endl;
		cv::waitKey(0);
	}
}