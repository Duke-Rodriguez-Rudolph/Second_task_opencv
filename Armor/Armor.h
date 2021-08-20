#pragma once
#include<iostream>
#include <opencv2/opencv.hpp> 
#include<math.h>

using namespace std;
using namespace cv;

struct Armor {	//装甲板结构体
	RotatedRect first_lightbar;
	RotatedRect second_lightbar;
	vector<Point2f> middlePoints;
};

enum HVState	//关于旋转矩形状态的枚举
{
	horizontal = 0,	//水平
	vertical = 1,	//竖直
	tilt = 2	//倾斜

};

class Armors {
public:
	//储存装甲板的仓库
	vector<Armor> armors_store;

public:
	//配对灯条为装甲板
	void match_lightbar(vector<RotatedRect> &rect_boxes);	
	//把装甲板画出来
	void draw_armors(Mat &img, int mode, int color);

private:
	//判断旋转矩形的角度
	int judge_rectstate(RotatedRect lightbar);	
	//得出需要绘制的四个点
	void four_middlepoint(vector<Point2f> &middlePoints, RotatedRect lightbar);	
	//计算两点之间的距离
	float distance(Point2f first_point, Point2f second_point);	

};