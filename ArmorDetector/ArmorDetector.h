#pragma once
#include<iostream>
#include <opencv2/opencv.hpp> 
#include <math.h>

#include "LightBar.h"
#include "Armor.h"

using namespace std;
using namespace cv;

enum enemies_color {
	enemies_blue = 0,
	enemies_red = 1

};

struct kalman_store {
	//卡尔曼滤波
	KalmanFilter KF;
	//上一次的测量量
	Mat measurement = (Mat_<float>(2, 1) << 0, 0);
	//是否被匹配标志位
	int match_flag=0;
	//没被匹配次数
	int unmatch_num=0;
};

class ArmorDetector {
public:
	//最后处理的导入图片
	Mat img;
	//最后处理的导入图片的二值图
	Mat binaryImg;
	//灯条
	LightBars lightbars;
	//装甲板
	Armors armors;
	//相机的高度
	int cap_height = 960;
	//相机的宽度
	int cap_width = 600;
	//每帧之间的时间
	int wait_time = 50;
	//对方颜色
	int color;

public:
	//检测一张图上的装甲板
	void one_detect(Mat img);
	//检测一个视频中的装甲板
	void video_detect(const String &filename);

private:
	//储存卡尔曼滤波的队列
	deque<kalman_store> kalman_deque;
	//储存预测点
	vector<Mat> predictions;
private:
	//图片预处理
	void image_preprocessing(Mat img);
	//计算两点之间距离
	float distance(Point2f first_point, Point2f second_point);
	//卡尔曼滤波初始化
	KalmanFilter kalman_init(void);
	//卡尔曼滤波清算
	void kalman_clear(void);
	//卡尔曼滤波预测
	void kalman_predict(void);
	//卡尔曼滤波器匹配
	void kalman_match(void);
	//卡尔曼滤波预测绘制
	void draw_kalman_predict(void);

};
