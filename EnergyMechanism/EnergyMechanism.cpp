#include "EnergyMechanism.h"


/**
* @brief ͼƬ�����������
*
*/
void EnergyMechanism::image_detect(Mat img) {
	//ͼƬԤ����
	this->image_processing(img);
	//Ѱ�һ����
	this->find_strike_board();
	//���������
	this->correct_strike_board();
	//Ѱ��������������
	this->find_mechanism_center();
	if (this->kalman_flag == 1) {
		//�������˲�Ԥ��
		this->kalman_predict();
		//����������Ԥ���
		this->draw_kalman_predict();
	}
	//������ǿ�
	this->draw_strike_board();
	//����������������
	this->draw_mechanism_center();
	//չʾ
	imshow("img", this->img);
}


/**
* @brief ��Ƶ�����������
*
* @param filename ��Ƶ����
*/
void EnergyMechanism::video_detect(const String &filename) {
	//ʵ�������
	VideoCapture capture;
	//֡
	Mat frame;
	//��ȡ��Ƶ
	frame = capture.open(filename);
	//resizeWindow("img", this->cap_height, this->cap_width);
	if (this->is_kalman == 1) {
		//�������˲���ʼ��
		this->kalman_init();
		this->kalman_flag = 1;
	}
	//��ÿһ֡���в���
	while (capture.isOpened()) {
		capture.read(frame);
		this->img = frame;
		image_detect(frame);
		waitKey(this->wait_time);
	}
}


/**
* @brief ͼƬԤ����
*
* @param img ��Ҫ�����ͼƬ
*/
void EnergyMechanism::image_processing(Mat img) {
	//��ֵͼ
	Mat binaryImg;
	//�Ҷ�ͼ
	Mat grayImg;
	//�洢ͨ��
	vector<Mat> channels;
	//ͨ������
	split(img, channels);

	//ѡȡ��ɫ
	if (this->color == 0) {
		grayImg = channels.at(2);
	}
	if (this->color == 1) {
		grayImg = channels.at(0);
	}
	
	//��ֵ��
	threshold(grayImg, binaryImg, 120, 255, THRESH_BINARY);
	//�������Ͳ�����ͼƬ���ں�
	Mat imgDial,element = getStructuringElement(MORPH_RECT, Size(4, 4));
	//����
	dilate(binaryImg, imgDial, element, Point(-1, -1), 2);
	//imshow("twoValueImg", binaryImg);	//�鿴��ֵͼ
	//imshow("imgDial", imgDial);	//�鿴���ͺ�ͼ

	//�洢��ֵͼ��������
	this->binaryImg = imgDial;
}


/**
* @brief Ѱ��������������
*
*/
void EnergyMechanism::find_mechanism_center(void) {
	//��ȡ���������
	vector<vector<Point>> strike_boards=this->strike_boards;
	//��ȡ����R������
	vector<vector<Point>> r_sign = this->r_sign;
	//��������R������
	vector<vector<Point>> correct_r_sign;

	for (int i = 0; i < r_sign.size(); i++) {
		//������R��ȡ��С���������
		RotatedRect r_rect = minAreaRect(r_sign[i]);
		double r_area = contourArea(r_sign[i]);

		for (int j = 0; j < strike_boards.size(); j++) {
			//�������ȡ��С���������
			RotatedRect board_rect = minAreaRect(strike_boards[j]);
			double board_area = contourArea(strike_boards[j]);

			//ȡ����峤��
			float scale = max(board_rect.size.width, board_rect.size.height);
			//����R���е㵽������е����
			float two_distance = distance(r_rect.center, board_rect.center);

			if (two_distance > 2 * scale && two_distance < 5 * scale) { 
				//���R��Ȼ�����������
				if (r_area > board_area) { continue; }
				correct_r_sign.push_back(r_sign[i]);
				break;
			}
		}
	}
	//����R������
	this->r_sign = correct_r_sign;
}


/**
* @brief Ѱ�һ����
*
*/
void EnergyMechanism::find_strike_board(void) {
	//��������
	vector<vector<Point>> contours;
	//����������Ϣ�Ĵ洢��
	vector<Vec4i> hierarchy;

	//Ѱ������
	findContours(this->binaryImg, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);

	//�洢���������������
	vector<vector<Point>> strike_boards;
	vector<vector<Point>> r_sign;
	for (int i = 0; i < hierarchy.size(); i++) {
		//�����������
		double area = contourArea(contours[i]);
		//���С�����ȫ��ȫ������
		if (area < 250) { continue; }
		//��С�������
		RotatedRect rect = minAreaRect(contours[i]);
		//�������
		float aspect_ratio = min(rect.size.height, rect.size.width) / max(rect.size.height, rect.size.width);
		//���Ƚ���һ��һ�ļ���R����
		if (aspect_ratio>0.75) {
			r_sign.push_back(contours[i]);
			continue;
		}

		//������������������ȫ������
		if (hierarchy[i][3] != -1) { continue; }

		//��������������
		int son_index = hierarchy[i][2];
		
		//��������������
		int son_num = 0;

		//��������������
		while (son_index != -1) {
			son_num++;
			son_index = hierarchy[son_index][0];
		}
		//�洢����������
		if (son_num == 1) {
			strike_boards.push_back(contours[hierarchy[i][2]]);
		}
	}

	this->strike_boards = strike_boards;
	this->r_sign = r_sign;
}


/**
* @brief ���������
*
*/
void EnergyMechanism::correct_strike_board(void) {
	//���������ó����������
	vector<vector<Point>> contours = this->strike_boards;
	//�������������ֿ�
	vector<vector<Point>> correct_contours;

	for (int i = 0; i < contours.size(); i++) {
		//�����������
		double area = contourArea(contours[i]);
		//���̫С����㲻Ҫ
		if (area < 300) { continue; }
		//��С�������
		RotatedRect rect = minAreaRect(contours[i]);
		//�������
		float aspect_ratio = min(rect.size.height, rect.size.width) / max(rect.size.height, rect.size.width);
		//����С��һ��ֵ��Ҫ
		if (aspect_ratio < 0.3) { continue; }

		correct_contours.push_back(contours[i]);
	}

	//����
	this->strike_boards = correct_contours;
}


/**
* @brief ������ǿ�
*
*/
void EnergyMechanism::draw_strike_board(void) {
	//���������ó����������
	vector<vector<Point>> contours = this->strike_boards;

	//�滭��ǿ�
	for (int i = 0; i < contours.size(); i++) {
		//��С�������
		RotatedRect rect = minAreaRect(contours[i]);

		//������
		if (this->color == mechanism_red) {
			circle(this->img, rect.center, 1, Scalar(255, 0, 0), 2);
		}
		if (this->color == mechanism_blue) {
			circle(this->img, rect.center, 1, Scalar(0, 0, 255), 2);
		}

		//��ת���λ��ĵ�
		Point2f rect_points[4];
		rect.points(rect_points);
		for (int j = 0; j <= 3; j++)
		{	
			if (this->color == mechanism_red) {
				line(this->img, rect_points[j], rect_points[(j + 1) % 4], Scalar(255, 0, 0), 2);
			}
			if (this->color == mechanism_blue) {
				line(this->img, rect_points[j], rect_points[(j + 1) % 4], Scalar(0, 0, 255), 2);
			}
		}

	}
}


/**
* @brief ����������������
*
*/
void EnergyMechanism::draw_mechanism_center(void) {
	//��ȡR������
	vector<vector<Point>> contours = this->r_sign;
	for (int i = 0; i < contours.size(); i++) {
		//��С�������
		RotatedRect rect = minAreaRect(contours[i]);
		//��ת���λ��ĵ�
		Point2f rect_points[4];
		rect.points(rect_points);
		for (int j = 0; j <= 3; j++)
		{
			if (this->color == mechanism_red) {
				line(this->img, rect_points[j], rect_points[(j + 1) % 4], Scalar(255, 0, 0), 2);
			}
			if (this->color == mechanism_blue) {
				line(this->img, rect_points[j], rect_points[(j + 1) % 4], Scalar(0, 0, 255), 2);
			}
		}
	}
}


/**
* @brief ��������֮��ľ���
*
* @param first_point ��һ����
* @param second_point �ڶ�����
* @return float distance ����֮��ľ���
*/
float EnergyMechanism::distance(Point2f first_point, Point2f second_point) {
	return sqrt(pow((first_point.x - second_point.x), 2) + pow((first_point.y - second_point.y), 2));
}


/**
* @brief ���������γɵĽǶ�
*
* @param first_point ��һ����
* @param second_point �ڶ�����(Բ��)
* @return double angle �γɵĽǶ�
*/
double EnergyMechanism::calculate_angle(Point2f first_point, Point2f second_point) {
	//x�Ĳ�
	double detla_x = first_point.x - second_point.x;
	//y�Ĳ�
	double detla_y = first_point.y - second_point.y;

	return atan2(detla_y ,detla_x);
}


/**
* @brief �������˲���ʼ��
*
*/
void EnergyMechanism::kalman_init(void) {
	//״̬��Ϊ�Ƕ�����ٶȣ�������Ϊ�Ƕ�
	KalmanFilter KF(2, 1, 0);
	//ת�ƾ���A
	KF.transitionMatrix = (Mat_<float>(2, 2) << 1,1,0,1);  
	//��������H  
	setIdentity(KF.measurementMatrix);
	//ϵͳ�����������Q
	setIdentity(KF.processNoiseCov, Scalar::all(1));
	//���������������R
	setIdentity(KF.measurementNoiseCov, Scalar::all(1e-4));
	//����������Э�������P
	setIdentity(KF.errorCovPost, Scalar::all(1));
	KF.statePost = (Mat_<float>(2, 1) << 0, 0);  //��ʼ״ֵ̬x(0)  
	//���������
	Mat measurement = (Mat_<float>(1, 1) << 0);

	//����������
	this->KF = KF;
	this->measurement = measurement;
}


/**
* @brief �������˲�Ԥ��
*
*/
void EnergyMechanism::kalman_predict(void) {
	if (this->strike_boards.size() != 0 && this->r_sign.size() != 0) {
		//�����
		RotatedRect board_rect = minAreaRect(this->strike_boards[0]);
		//R��
		RotatedRect r_rect = minAreaRect(this->r_sign[0]);
		//����뾶
		float r = this->distance(board_rect.center, r_rect.center);
		//����Ƕ�
		double angle = this->calculate_angle(board_rect.center, r_rect.center);
		//���²�������뾶
		this->measurement.at<float>(0) = (float)angle;
		this->KF.correct(this->measurement);
		this->r = r;
	}
	
}


/**
* @brief ����������Ԥ���
*
*/
void EnergyMechanism::draw_kalman_predict(void) {
	if (this->r_sign.size() != 0) {
		//R��
		RotatedRect r_rect = minAreaRect(this->r_sign[0]);
		//����Ԥ��
		Mat prediction = this->KF.predict();
		float angle_predict = prediction.at<float>(0);

		//�����̵�
		Point2f point_predict;
		point_predict.x = cos(angle_predict)*this->r + r_rect.center.x;
		point_predict.y = sin(angle_predict)*this->r + r_rect.center.y;

		circle(this->img, point_predict, 1, Scalar(0, 255, 0), 2);
	}
	


}