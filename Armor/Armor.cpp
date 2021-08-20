#include "Armor.h"

/**
* @brief 配对灯条为装甲板
*
* @param rect_boxes 储存灯条数据的仓库
*/
void Armors::match_lightbar(vector<RotatedRect> &rect_boxes) {	
	//灯条仓库
	vector<Armor> armors;	
	if (rect_boxes.size() >= 2) {
		for (int i = 0; i < rect_boxes.size() - 1; i++) {
			for (int j = i + 1; j < rect_boxes.size(); j++) {
				//旋转矩形灯条一
				RotatedRect first_lightbar = rect_boxes[i];	
				//旋转矩形灯条二
				RotatedRect second_lightbar = rect_boxes[j];	
				//cout << "first_lightbar.angle:" << first_lightbar.angle << endl;	//查看灯条一的角度
				//cout << "second_lightbar.angle:" << second_lightbar.angle << endl;	//查看灯条二的角度
				//灯条一矩形的状态
				int first_rectstate = judge_rectstate(first_lightbar);	
				//灯条二矩形的状态
				int second_rectstate = judge_rectstate(second_lightbar);	
				if (first_rectstate == horizontal || second_rectstate == horizontal) {	//如果灯条水平放置的，排除
					continue;
				}


				//定义平行率，角度之差
				float parallel_rate = abs(first_lightbar.angle - second_lightbar.angle);	
				if (parallel_rate > 7.0) { continue; }	//平行率大于限定值的排除


				//左右灯长比率
				float length_ratio = abs(first_lightbar.size.height - second_lightbar.size.height) /
					max(first_lightbar.size.height, second_lightbar.size.height);	
				//cout << "length_ratio:" << length_ratio << endl;	//查看所有灯长的比率
				if (length_ratio > 0.2) { continue; }	//左右灯长比率超出一定值的排除

				//中心点距离
				float center_distance = distance(first_lightbar.center, second_lightbar.center);	
				//灯条长度平均值
				float length_mean = (first_lightbar.size.height + second_lightbar.size.height) / 2;	
				//定义x坐标距离
				float x_distance = abs(first_lightbar.center.x - second_lightbar.center.x);
				//定义y坐标距离
				float y_distance = abs(first_lightbar.center.y - second_lightbar.center.y);	
				//x差比率
				float x_diffratio = x_distance / length_mean;	
				//y差比率
				float y_diffratio = y_distance / length_mean;	
				//装甲板长宽比
				float armor_aspect_ratio = center_distance / length_mean;	

				if (x_diffratio<1.5 || y_diffratio>0.7 || armor_aspect_ratio > 4.2 || armor_aspect_ratio < 1.1) {
					continue;
				}

				//cout << "x_diffratio:" << x_diffratio << endl;	//x差比率
				//cout << "y_diffratio:" << y_diffratio << endl;	//y差比率
				//cout << "armor_aspect_ratio:" << armor_aspect_ratio << endl;	//装甲板长宽比


				vector<Point2f> middlePoints;
				//得出灯条一的两个点
				four_middlepoint(middlePoints, first_lightbar);	
				//得出灯条二的两个点
				four_middlepoint(middlePoints, second_lightbar);	

				//对角率：对角线与灯条长度平均值的比
				float diagonal_ratio = abs(distance(middlePoints[0], middlePoints[2]) - distance(middlePoints[1], middlePoints[3])) / length_mean;;

				//两个灯条都为竖直情况下，对角率大于特定值的排除
				if (first_rectstate == vertical && second_rectstate == vertical && diagonal_ratio > 0.2) {
					continue;
				}

				//定义装甲板
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
* @brief 把装甲板画出来
*
* @param img 充当画布的图片
* @param mode 绘制模式，0为画框框模式，1为标记模式
*/
void Armors::draw_armors(Mat &img, int mode, int color) {
	vector<Armor> armors = this->armors_store;
	if (armors.size() > 0) {
		cout << "there " << armors.size() << " armors" << endl;
		if (mode == 0) { //画框框模式
			double color_scale = 255 / armors.size();
			for (int i = 0; i < armors.size(); i++) {
				Point2f first_points[4];	//矩形化四点
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
* @brief 判断旋转矩形的状态
*
* @param lightbar 灯条
* @return int state 旋转矩形的状态
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
* @brief 得出需要绘制的四个点，需要使用两次
*
* @param middlePoints 储存四点的仓库
* @param lightbar 灯条数据
*/
void Armors::four_middlepoint(vector<Point2f> &middlePoints, RotatedRect lightbar) {	
	Point2f four_points[4];
	lightbar.points(four_points);
	//灯条下方点
	middlePoints.push_back((four_points[0] + four_points[3]) / 2);
	//灯条上方点
	middlePoints.push_back((four_points[1] + four_points[2]) / 2);
}


/**
* @brief 计算两点之间的距离
*
* @param first_point 第一个灯条
* @param second_point 第二个灯条
* @return float distance 两点之间的距离
*/
float Armors::distance(Point2f first_point, Point2f second_point) {	
	return sqrt(pow((first_point.x - second_point.x), 2) + pow((first_point.y - second_point.y), 2));
}
