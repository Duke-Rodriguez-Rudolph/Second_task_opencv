#include "LightBar.h"

/**
* @brief 选择灯条
*
* @param binaryImg 二值化图
*/
void LightBars::select_lightbar(Mat binaryImg) {	
	//灯条的旋转矩形参数
	vector<RotatedRect> rect_boxes;
	//储存灯条轮廓
	vector<vector<Point>> lightContours;	
	findContours(binaryImg, lightContours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);	//找轮廓

	for (int i = 0; i < lightContours.size(); i++) {
		//定义轮廓面积
		double contour_area = contourArea(lightContours[i]);	
		if (contour_area > 10) {
			if (lightContours[i].size() < 6) { continue; }	//椭圆拟合必须要六个点以上
			//定义最小旋转矩形
			RotatedRect rect = fitEllipse(lightContours[i]);	
			revise_oval(rect);
			//定义宽高比
			double aspect_ratio = rect.size.width / rect.size.height;
			//实际轮廓与最小矩形面积比：轮矩比
			double proportion = contour_area / rect.size.area();	

			//cout <<"all aspect_ratio:"<< aspect_ratio << endl;	//查看所有轮廓的宽高比
			//cout << "all proportion:" << proportion << endl;	//查看所有轮廓的轮矩比

			if (aspect_ratio < 0.1 || proportion < 0.58) { continue; }

			//cout << "light_bar aspect_ratio:" << aspect_ratio << endl;	//查看灯条的宽高比
			//cout << "light_bar proportion:" << proportion << endl;	//查看灯条轮矩比
			//cout << "light_bar angle:" << rect.angle << endl<<endl;	//查看灯条角度
			rect_boxes.push_back(rect);

		}
	}

	this->rect_boxes = rect_boxes;
}


/**
* @brief 将椭圆的angle由[0,180]转为[-90,90]
*
* @param rect 需要纠正的椭圆数据
*/
void LightBars::revise_oval(RotatedRect &rect) {	
	if (rect.angle > 90) {
		rect.angle -= 180;
	}
}


/**
* @brief 画灯条
*
* @param img 需要绘制的画布
*/
void LightBars::draw_lightbars(Mat &img) {	
	for (int i = 0; i < this->rect_boxes.size(); i++) {
		//矩形化四点
		Point2f rect_points[4];	
		this->rect_boxes[i].points(rect_points);


		for (int j = 0; j <= 3; j++)
		{
			line(img, rect_points[j], rect_points[(j + 1) % 4], Scalar(0, 0, 255), 2);
		}
	}
}

