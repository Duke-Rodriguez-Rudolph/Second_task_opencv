#pragma once
#include<iostream>
#include <opencv2/opencv.hpp> 
#include<math.h>

using namespace std;
using namespace cv;


class LightBars {
public:
	//灯条的旋转矩形参数
	vector<RotatedRect> rect_boxes;	

public:
	//筛选灯条
	void select_lightbar(Mat binaryImg);	
	//画灯条
	void draw_lightbars(Mat &img);	

private:
	//将椭圆的angle由[0,180]转为[-90,90]
	void revise_oval(RotatedRect &rect);	
};