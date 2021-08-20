#pragma once
#include<iostream>
#include <opencv2/opencv.hpp> 
#include<math.h>

using namespace std;
using namespace cv;

struct Armor {	//װ�װ�ṹ��
	RotatedRect first_lightbar;
	RotatedRect second_lightbar;
	vector<Point2f> middlePoints;
};

enum HVState	//������ת����״̬��ö��
{
	horizontal = 0,	//ˮƽ
	vertical = 1,	//��ֱ
	tilt = 2	//��б

};

class Armors {
public:
	//����װ�װ�Ĳֿ�
	vector<Armor> armors_store;

public:
	//��Ե���Ϊװ�װ�
	void match_lightbar(vector<RotatedRect> &rect_boxes);	
	//��װ�װ廭����
	void draw_armors(Mat &img, int mode, int color);

private:
	//�ж���ת���εĽǶ�
	int judge_rectstate(RotatedRect lightbar);	
	//�ó���Ҫ���Ƶ��ĸ���
	void four_middlepoint(vector<Point2f> &middlePoints, RotatedRect lightbar);	
	//��������֮��ľ���
	float distance(Point2f first_point, Point2f second_point);	

};