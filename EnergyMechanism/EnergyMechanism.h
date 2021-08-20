#pragma once
#include<iostream>
#include <opencv2/opencv.hpp> 
#include<math.h>

using namespace std;
using namespace cv;


enum mechanism_color {
	mechanism_blue = 1,
	mechanism_red = 0

};

class EnergyMechanism {
public:
	//最后处理的导入图片
	Mat img;
	//最后处理的导入图片的二值图
	Mat binaryImg;
	//是否进行卡尔曼滤波预测
	int is_kalman = 1;
	//能量机关颜色
	int color;
	//每帧之间的时间
	int wait_time = 50;
	

public:
	//图片检测能量机关
	void image_detect(Mat img);
	//视频检测能量机关
	void video_detect(const String &filename);

private:
	//存储击打板轮廓的容器
	vector<vector<Point>> strike_boards;
	//存储R标轮廓的容器
	vector<vector<Point>> r_sign;
	//卡尔曼滤波器标志位
	int kalman_flag=0;
	//卡尔曼滤波器
	KalmanFilter KF;
	//卡尔曼测量量
	Mat measurement;
	//卡尔曼预测中需要用到的半径
	float r;

private:
	//图片预处理
	void image_processing(Mat img);
	//寻找能量机关中心
	void find_mechanism_center(void);
	//寻找击打板
	void find_strike_board(void);
	//修正击打板
	void correct_strike_board(void);
	//卡尔曼滤波初始化
	void kalman_init(void);
	//卡尔曼滤波预测
	void kalman_predict(void);
	//画出卡尔曼预测框
	void draw_kalman_predict(void);
	//画出标记框
	void draw_strike_board(void);
	//画出能量机关中心
	void draw_mechanism_center(void);
	//计算两点之间的距离
	float distance(Point2f first_point, Point2f second_point);
	//计算角度
	double calculate_angle(Point2f first_point, Point2f second_point);
};