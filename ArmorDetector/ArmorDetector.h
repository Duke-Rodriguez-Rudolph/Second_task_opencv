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
	//�������˲�
	KalmanFilter KF;
	//��һ�εĲ�����
	Mat measurement = (Mat_<float>(2, 1) << 0, 0);
	//�Ƿ�ƥ���־λ
	int match_flag=0;
	//û��ƥ�����
	int unmatch_num=0;
};

class ArmorDetector {
public:
	//�����ĵ���ͼƬ
	Mat img;
	//�����ĵ���ͼƬ�Ķ�ֵͼ
	Mat binaryImg;
	//����
	LightBars lightbars;
	//װ�װ�
	Armors armors;
	//����ĸ߶�
	int cap_height = 960;
	//����Ŀ��
	int cap_width = 600;
	//ÿ֮֡���ʱ��
	int wait_time = 50;
	//�Է���ɫ
	int color;

public:
	//���һ��ͼ�ϵ�װ�װ�
	void one_detect(Mat img);
	//���һ����Ƶ�е�װ�װ�
	void video_detect(const String &filename);

private:
	//���濨�����˲��Ķ���
	deque<kalman_store> kalman_deque;
	//����Ԥ���
	vector<Mat> predictions;
private:
	//ͼƬԤ����
	void image_preprocessing(Mat img);
	//��������֮�����
	float distance(Point2f first_point, Point2f second_point);
	//�������˲���ʼ��
	KalmanFilter kalman_init(void);
	//�������˲�����
	void kalman_clear(void);
	//�������˲�Ԥ��
	void kalman_predict(void);
	//�������˲���ƥ��
	void kalman_match(void);
	//�������˲�Ԥ�����
	void draw_kalman_predict(void);

};
