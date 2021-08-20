#pragma once
#include<iostream>
#include <opencv2/opencv.hpp> 
#include<math.h>

using namespace std;
using namespace cv;


class LightBars {
public:
	//��������ת���β���
	vector<RotatedRect> rect_boxes;	

public:
	//ɸѡ����
	void select_lightbar(Mat binaryImg);	
	//������
	void draw_lightbars(Mat &img);	

private:
	//����Բ��angle��[0,180]תΪ[-90,90]
	void revise_oval(RotatedRect &rect);	
};