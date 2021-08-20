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
	//�����ĵ���ͼƬ
	Mat img;
	//�����ĵ���ͼƬ�Ķ�ֵͼ
	Mat binaryImg;
	//�Ƿ���п������˲�Ԥ��
	int is_kalman = 1;
	//����������ɫ
	int color;
	//ÿ֮֡���ʱ��
	int wait_time = 50;
	

public:
	//ͼƬ�����������
	void image_detect(Mat img);
	//��Ƶ�����������
	void video_detect(const String &filename);

private:
	//�洢���������������
	vector<vector<Point>> strike_boards;
	//�洢R������������
	vector<vector<Point>> r_sign;
	//�������˲�����־λ
	int kalman_flag=0;
	//�������˲���
	KalmanFilter KF;
	//������������
	Mat measurement;
	//������Ԥ������Ҫ�õ��İ뾶
	float r;

private:
	//ͼƬԤ����
	void image_processing(Mat img);
	//Ѱ��������������
	void find_mechanism_center(void);
	//Ѱ�һ����
	void find_strike_board(void);
	//���������
	void correct_strike_board(void);
	//�������˲���ʼ��
	void kalman_init(void);
	//�������˲�Ԥ��
	void kalman_predict(void);
	//����������Ԥ���
	void draw_kalman_predict(void);
	//������ǿ�
	void draw_strike_board(void);
	//����������������
	void draw_mechanism_center(void);
	//��������֮��ľ���
	float distance(Point2f first_point, Point2f second_point);
	//����Ƕ�
	double calculate_angle(Point2f first_point, Point2f second_point);
};