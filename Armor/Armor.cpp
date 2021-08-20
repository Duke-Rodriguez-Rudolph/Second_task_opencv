#include "Armor.h"

/**
* @brief ��Ե���Ϊװ�װ�
*
* @param rect_boxes ����������ݵĲֿ�
*/
void Armors::match_lightbar(vector<RotatedRect> &rect_boxes) {	
	//�����ֿ�
	vector<Armor> armors;	
	if (rect_boxes.size() >= 2) {
		for (int i = 0; i < rect_boxes.size() - 1; i++) {
			for (int j = i + 1; j < rect_boxes.size(); j++) {
				//��ת���ε���һ
				RotatedRect first_lightbar = rect_boxes[i];	
				//��ת���ε�����
				RotatedRect second_lightbar = rect_boxes[j];	
				//cout << "first_lightbar.angle:" << first_lightbar.angle << endl;	//�鿴����һ�ĽǶ�
				//cout << "second_lightbar.angle:" << second_lightbar.angle << endl;	//�鿴�������ĽǶ�
				//����һ���ε�״̬
				int first_rectstate = judge_rectstate(first_lightbar);	
				//���������ε�״̬
				int second_rectstate = judge_rectstate(second_lightbar);	
				if (first_rectstate == horizontal || second_rectstate == horizontal) {	//�������ˮƽ���õģ��ų�
					continue;
				}


				//����ƽ���ʣ��Ƕ�֮��
				float parallel_rate = abs(first_lightbar.angle - second_lightbar.angle);	
				if (parallel_rate > 7.0) { continue; }	//ƽ���ʴ����޶�ֵ���ų�


				//���ҵƳ�����
				float length_ratio = abs(first_lightbar.size.height - second_lightbar.size.height) /
					max(first_lightbar.size.height, second_lightbar.size.height);	
				//cout << "length_ratio:" << length_ratio << endl;	//�鿴���еƳ��ı���
				if (length_ratio > 0.2) { continue; }	//���ҵƳ����ʳ���һ��ֵ���ų�

				//���ĵ����
				float center_distance = distance(first_lightbar.center, second_lightbar.center);	
				//��������ƽ��ֵ
				float length_mean = (first_lightbar.size.height + second_lightbar.size.height) / 2;	
				//����x�������
				float x_distance = abs(first_lightbar.center.x - second_lightbar.center.x);
				//����y�������
				float y_distance = abs(first_lightbar.center.y - second_lightbar.center.y);	
				//x�����
				float x_diffratio = x_distance / length_mean;	
				//y�����
				float y_diffratio = y_distance / length_mean;	
				//װ�װ峤���
				float armor_aspect_ratio = center_distance / length_mean;	

				if (x_diffratio<1.5 || y_diffratio>0.7 || armor_aspect_ratio > 4.2 || armor_aspect_ratio < 1.1) {
					continue;
				}

				//cout << "x_diffratio:" << x_diffratio << endl;	//x�����
				//cout << "y_diffratio:" << y_diffratio << endl;	//y�����
				//cout << "armor_aspect_ratio:" << armor_aspect_ratio << endl;	//װ�װ峤���


				vector<Point2f> middlePoints;
				//�ó�����һ��������
				four_middlepoint(middlePoints, first_lightbar);	
				//�ó���������������
				four_middlepoint(middlePoints, second_lightbar);	

				//�Խ��ʣ��Խ������������ƽ��ֵ�ı�
				float diagonal_ratio = abs(distance(middlePoints[0], middlePoints[2]) - distance(middlePoints[1], middlePoints[3])) / length_mean;;

				//����������Ϊ��ֱ����£��Խ��ʴ����ض�ֵ���ų�
				if (first_rectstate == vertical && second_rectstate == vertical && diagonal_ratio > 0.2) {
					continue;
				}

				//����װ�װ�
				Armor armor;	
				armor.first_lightbar = first_lightbar;
				armor.second_lightbar = second_lightbar;
				armor.middlePoints = middlePoints;
				armors.push_back(armor);
			}
		}
	}
	this->armors_store = armors;
}


/**
* @brief ��װ�װ廭����
*
* @param img �䵱������ͼƬ
* @param mode ����ģʽ��0Ϊ�����ģʽ��1Ϊ���ģʽ
*/
void Armors::draw_armors(Mat &img, int mode, int color) {
	vector<Armor> armors = this->armors_store;
	if (armors.size() > 0) {
		cout << "there " << armors.size() << " armors" << endl;
		if (mode == 0) { //�����ģʽ
			double color_scale = 255 / armors.size();
			for (int i = 0; i < armors.size(); i++) {
				Point2f first_points[4];	//���λ��ĵ�
				Point2f second_points[4];
				armors[i].first_lightbar.points(first_points);
				armors[i].second_lightbar.points(second_points);


				for (int j = 0; j <= 3; j++)
				{
					line(img, first_points[j], first_points[(j + 1) % 4], Scalar(0, 0, color_scale*(i + 1)), 2);
				}

				for (int j = 0; j <= 3; j++)
				{
					line(img, second_points[j], second_points[(j + 1) % 4], Scalar(0, 0, color_scale*(i + 1)), 2);
				}
			}
		}

		if (mode == 1) {
			for (int i = 0; i < armors.size(); i++) {
				for (int j = 0; j < 2; j++) {
					if (color == 0) {
						line(img, armors[i].middlePoints[j], armors[i].middlePoints[3 - j], Scalar(0, 0, 255), 2);
					}
					if (color == 1) {
						line(img, armors[i].middlePoints[j], armors[i].middlePoints[3 - j], Scalar(255, 0, 0), 2);
					}
					
				}
				for (int j = 0; j < 4; j++) { circle(img, armors[i].middlePoints[j], 1, Scalar(0, 255, 0), 3); }
				Point2f armor_middle = (armors[i].middlePoints[0] + armors[i].middlePoints[3]) / 2;
				circle(img, armor_middle, 1, Scalar(0, 255, 0), 3);

			}
		}
	}
	else {
		cout << "can't find armor!" << endl;
	}

}


/**
* @brief �ж���ת���ε�״̬
*
* @param lightbar ����
* @return int state ��ת���ε�״̬
*/
int Armors::judge_rectstate(RotatedRect lightbar) {	
	if (lightbar.angle<15 && lightbar.angle>-15) {
		return vertical;
	}
	else if (lightbar.angle < -75 || lightbar.angle>75) {
		return horizontal;
	}
	else {
		return tilt;
	}
}


/**
* @brief �ó���Ҫ���Ƶ��ĸ��㣬��Ҫʹ������
*
* @param middlePoints �����ĵ�Ĳֿ�
* @param lightbar ��������
*/
void Armors::four_middlepoint(vector<Point2f> &middlePoints, RotatedRect lightbar) {	
	Point2f four_points[4];
	lightbar.points(four_points);
	//�����·���
	middlePoints.push_back((four_points[0] + four_points[3]) / 2);
	//�����Ϸ���
	middlePoints.push_back((four_points[1] + four_points[2]) / 2);
}


/**
* @brief ��������֮��ľ���
*
* @param first_point ��һ������
* @param second_point �ڶ�������
* @return float distance ����֮��ľ���
*/
float Armors::distance(Point2f first_point, Point2f second_point) {	
	return sqrt(pow((first_point.x - second_point.x), 2) + pow((first_point.y - second_point.y), 2));
}
