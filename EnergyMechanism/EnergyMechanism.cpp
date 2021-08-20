#include "EnergyMechanism.h"


/**
* @brief 图片检测能量机关
*
*/
void EnergyMechanism::image_detect(Mat img) {
	//图片预处理
	this->image_processing(img);
	//寻找击打板
	this->find_strike_board();
	//修正击打板
	this->correct_strike_board();
	//寻找能量机关中心
	this->find_mechanism_center();
	if (this->kalman_flag == 1) {
		//卡尔曼滤波预测
		this->kalman_predict();
		//画出卡尔曼预测框
		this->draw_kalman_predict();
	}
	//画出标记框
	this->draw_strike_board();
	//画出能量机关中心
	this->draw_mechanism_center();
	//展示
	imshow("img", this->img);
}


/**
* @brief 视频检测能量机关
*
* @param filename 视频名称
*/
void EnergyMechanism::video_detect(const String &filename) {
	//实例化相机
	VideoCapture capture;
	//帧
	Mat frame;
	//读取视频
	frame = capture.open(filename);
	//resizeWindow("img", this->cap_height, this->cap_width);
	if (this->is_kalman == 1) {
		//卡尔曼滤波初始化
		this->kalman_init();
		this->kalman_flag = 1;
	}
	//对每一帧进行操作
	while (capture.isOpened()) {
		capture.read(frame);
		this->img = frame;
		image_detect(frame);
		waitKey(this->wait_time);
	}
}


/**
* @brief 图片预处理
*
* @param img 需要处理的图片
*/
void EnergyMechanism::image_processing(Mat img) {
	//二值图
	Mat binaryImg;
	//灰度图
	Mat grayImg;
	//存储通道
	vector<Mat> channels;
	//通道分离
	split(img, channels);

	//选取颜色
	if (this->color == 0) {
		grayImg = channels.at(2);
	}
	if (this->color == 1) {
		grayImg = channels.at(0);
	}
	
	//二值化
	threshold(grayImg, binaryImg, 120, 255, THRESH_BINARY);
	//定义膨胀操作的图片与内核
	Mat imgDial,element = getStructuringElement(MORPH_RECT, Size(4, 4));
	//膨胀
	dilate(binaryImg, imgDial, element, Point(-1, -1), 2);
	//imshow("twoValueImg", binaryImg);	//查看二值图
	//imshow("imgDial", imgDial);	//查看膨胀后图

	//存储二值图到属性中
	this->binaryImg = imgDial;
}


/**
* @brief 寻找能量机关中心
*
*/
void EnergyMechanism::find_mechanism_center(void) {
	//提取击打板轮廓
	vector<vector<Point>> strike_boards=this->strike_boards;
	//提取嫌疑R标轮廓
	vector<vector<Point>> r_sign = this->r_sign;
	//定义修正R标轮廓
	vector<vector<Point>> correct_r_sign;

	for (int i = 0; i < r_sign.size(); i++) {
		//将嫌疑R标取最小矩形与面积
		RotatedRect r_rect = minAreaRect(r_sign[i]);
		double r_area = contourArea(r_sign[i]);

		for (int j = 0; j < strike_boards.size(); j++) {
			//将击打板取最小矩形与面积
			RotatedRect board_rect = minAreaRect(strike_boards[j]);
			double board_area = contourArea(strike_boards[j]);

			//取击打板长度
			float scale = max(board_rect.size.width, board_rect.size.height);
			//计算R标中点到击打板中点距离
			float two_distance = distance(r_rect.center, board_rect.center);

			if (two_distance > 2 * scale && two_distance < 5 * scale) { 
				//如果R标比击打板大，则舍弃
				if (r_area > board_area) { continue; }
				correct_r_sign.push_back(r_sign[i]);
				break;
			}
		}
	}
	//更新R标数据
	this->r_sign = correct_r_sign;
}


/**
* @brief 寻找击打板
*
*/
void EnergyMechanism::find_strike_board(void) {
	//定义轮廓
	vector<vector<Point>> contours;
	//定义轮廓信息的存储器
	vector<Vec4i> hierarchy;

	//寻找轮廓
	findContours(this->binaryImg, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);

	//存储击打板轮廓的容器
	vector<vector<Point>> strike_boards;
	vector<vector<Point>> r_sign;
	for (int i = 0; i < hierarchy.size(); i++) {
		//定义轮廓面积
		double area = contourArea(contours[i]);
		//面积小的噪点全部全部舍弃
		if (area < 250) { continue; }
		//最小矩形拟合
		RotatedRect rect = minAreaRect(contours[i]);
		//定义宽长比
		float aspect_ratio = min(rect.size.height, rect.size.width) / max(rect.size.height, rect.size.width);
		//宽长比近似一比一的加入R标中
		if (aspect_ratio>0.75) {
			r_sign.push_back(contours[i]);
			continue;
		}

		//不存在子轮廓的轮廓全部舍弃
		if (hierarchy[i][3] != -1) { continue; }

		//定义子轮廓索引
		int son_index = hierarchy[i][2];
		
		//定义子轮廓数量
		int son_num = 0;

		//计算子轮廓数量
		while (son_index != -1) {
			son_num++;
			son_index = hierarchy[son_index][0];
		}
		//存储击打板的序列
		if (son_num == 1) {
			strike_boards.push_back(contours[hierarchy[i][2]]);
		}
	}

	this->strike_boards = strike_boards;
	this->r_sign = r_sign;
}


/**
* @brief 修正击打板
*
*/
void EnergyMechanism::correct_strike_board(void) {
	//从属性里拿出击打板轮廓
	vector<vector<Point>> contours = this->strike_boards;
	//定义修正轮廓仓库
	vector<vector<Point>> correct_contours;

	for (int i = 0; i < contours.size(); i++) {
		//定义轮廓面积
		double area = contourArea(contours[i]);
		//面积太小的噪点不要
		if (area < 300) { continue; }
		//最小矩形拟合
		RotatedRect rect = minAreaRect(contours[i]);
		//定义宽长比
		float aspect_ratio = min(rect.size.height, rect.size.width) / max(rect.size.height, rect.size.width);
		//宽长比小于一定值不要
		if (aspect_ratio < 0.3) { continue; }

		correct_contours.push_back(contours[i]);
	}

	//更新
	this->strike_boards = correct_contours;
}


/**
* @brief 画出标记框
*
*/
void EnergyMechanism::draw_strike_board(void) {
	//从属性里拿出击打板轮廓
	vector<vector<Point>> contours = this->strike_boards;

	//绘画标记框
	for (int i = 0; i < contours.size(); i++) {
		//最小矩形拟合
		RotatedRect rect = minAreaRect(contours[i]);

		//画中心
		if (this->color == mechanism_red) {
			circle(this->img, rect.center, 1, Scalar(255, 0, 0), 2);
		}
		if (this->color == mechanism_blue) {
			circle(this->img, rect.center, 1, Scalar(0, 0, 255), 2);
		}

		//旋转矩形化四点
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
* @brief 画出能量机关中心
*
*/
void EnergyMechanism::draw_mechanism_center(void) {
	//提取R标数据
	vector<vector<Point>> contours = this->r_sign;
	for (int i = 0; i < contours.size(); i++) {
		//最小矩形拟合
		RotatedRect rect = minAreaRect(contours[i]);
		//旋转矩形化四点
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
* @brief 计算两点之间的距离
*
* @param first_point 第一个点
* @param second_point 第二个点
* @return float distance 两点之间的距离
*/
float EnergyMechanism::distance(Point2f first_point, Point2f second_point) {
	return sqrt(pow((first_point.x - second_point.x), 2) + pow((first_point.y - second_point.y), 2));
}


/**
* @brief 计算两点形成的角度
*
* @param first_point 第一个点
* @param second_point 第二个点(圆心)
* @return double angle 形成的角度
*/
double EnergyMechanism::calculate_angle(Point2f first_point, Point2f second_point) {
	//x的差
	double detla_x = first_point.x - second_point.x;
	//y的差
	double detla_y = first_point.y - second_point.y;

	return atan2(detla_y ,detla_x);
}


/**
* @brief 卡尔曼滤波初始化
*
*/
void EnergyMechanism::kalman_init(void) {
	//状态量为角度与角速度，测量量为角度
	KalmanFilter KF(2, 1, 0);
	//转移矩阵A
	KF.transitionMatrix = (Mat_<float>(2, 2) << 1,1,0,1);  
	//测量矩阵H  
	setIdentity(KF.measurementMatrix);
	//系统噪声方差矩阵Q
	setIdentity(KF.processNoiseCov, Scalar::all(1));
	//测量噪声方差矩阵R
	setIdentity(KF.measurementNoiseCov, Scalar::all(1e-4));
	//后验错误估计协方差矩阵P
	setIdentity(KF.errorCovPost, Scalar::all(1));
	KF.statePost = (Mat_<float>(2, 1) << 0, 0);  //初始状态值x(0)  
	//定义测量量
	Mat measurement = (Mat_<float>(1, 1) << 0);

	//更新类属性
	this->KF = KF;
	this->measurement = measurement;
}


/**
* @brief 卡尔曼滤波预测
*
*/
void EnergyMechanism::kalman_predict(void) {
	if (this->strike_boards.size() != 0 && this->r_sign.size() != 0) {
		//击打板
		RotatedRect board_rect = minAreaRect(this->strike_boards[0]);
		//R标
		RotatedRect r_rect = minAreaRect(this->r_sign[0]);
		//定义半径
		float r = this->distance(board_rect.center, r_rect.center);
		//定义角度
		double angle = this->calculate_angle(board_rect.center, r_rect.center);
		//更新测量量与半径
		this->measurement.at<float>(0) = (float)angle;
		this->KF.correct(this->measurement);
		this->r = r;
	}
	
}


/**
* @brief 画出卡尔曼预测框
*
*/
void EnergyMechanism::draw_kalman_predict(void) {
	if (this->r_sign.size() != 0) {
		//R标
		RotatedRect r_rect = minAreaRect(this->r_sign[0]);
		//进行预测
		Mat prediction = this->KF.predict();
		float angle_predict = prediction.at<float>(0);

		//画出绿点
		Point2f point_predict;
		point_predict.x = cos(angle_predict)*this->r + r_rect.center.x;
		point_predict.y = sin(angle_predict)*this->r + r_rect.center.y;

		circle(this->img, point_predict, 1, Scalar(0, 255, 0), 2);
	}
	


}