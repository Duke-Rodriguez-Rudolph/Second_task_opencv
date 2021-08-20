#include "ArmorDetector.h"

/**
* @brief 图片预处理
*
* @param img 需要处理的图片
* @param color 敌方颜色，0为蓝色，1为红色
*/
void ArmorDetector::image_preprocessing(Mat img) {
	//定义地方颜色
	int color = this->color;
	//二值图
	Mat binaryImg;
	//色彩通道容器
	vector<Mat> channels;	
	//定义灰度图
	Mat grayImg;	
	split(img, channels);	//分离色彩通道
	if (color == 0) {	//对方蓝色为0，对方红色为1
		grayImg = channels.at(0);
	}
	if (color == 1) {
		grayImg = channels.at(2);
	}
	//imshow("grayImg", grayImg);	//查看灰度图
	threshold(grayImg, binaryImg, 0, 255, THRESH_OTSU);	//二值化，使用大津法
	//imshow("binaryImg", binaryImg);	//查看二值图
	this->binaryImg = binaryImg;
}


/**
* @brief 检测一张图上的装甲板
*
* @param img 需要被检测的图片
*/
void ArmorDetector::one_detect(Mat img) {
	this->img = img;	//储存到属性中
	this->image_preprocessing(img);	//图片预处理

	this->lightbars.select_lightbar(this->binaryImg);	//筛选灯条
	//this->lightbars.draw_lightbars(img);	//绘制所有灯条轮廓

	this->armors.match_lightbar(lightbars.rect_boxes);	//配对灯条
	this->armors.draw_armors(img, 1,color);	//绘制所有装甲板

	this->kalman_match();
	this->kalman_clear();
	this->kalman_predict();
	this->draw_kalman_predict();
	imshow("img", img);	//展示图片
}


/**
* @brief 检测一个视频中的装甲板
*
* @param filename 视频的名称
*/
void ArmorDetector::video_detect(const String & filename){
	//实例化相机
	VideoCapture capture;
	//帧
	Mat frame;
	//读取视频
	frame = capture.open(filename);
	namedWindow("img", 0);
	resizeWindow("img", this->cap_height, this->cap_width);
	//对每一帧进行操作
	while (capture.isOpened()) {
		capture.read(frame);
		this->img = frame;	//储存到属性中
		one_detect(frame);
		waitKey(this->wait_time);
	}
}


/**
* @brief 卡尔曼滤波初始化
*
* @return KalmanFilter KF 卡尔曼滤波器
*/
KalmanFilter ArmorDetector::kalman_init(void) {
	//状态量为x,y,▲x,▲y，测量量为x,y
	KalmanFilter KF(4, 2, 0);
	//转移矩阵A
	KF.transitionMatrix = (Mat_<float>(4, 4) << 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1);
	//测量矩阵H  
	setIdentity(KF.measurementMatrix);
	//系统噪声方差矩阵Q
	setIdentity(KF.processNoiseCov, Scalar::all(1e-3));
	//测量噪声方差矩阵R
	setIdentity(KF.measurementNoiseCov, Scalar::all(1e-5));
	//后验错误估计协方差矩阵P
	setIdentity(KF.errorCovPost, Scalar::all(1));
	KF.statePost = (Mat_<float>(4, 1) << 0, 0, 0, 0);  //初始状态值x(0)  
	

	return KF;
}


/**
* @brief 卡尔曼滤波预测
*
*/
void ArmorDetector::kalman_predict(void) {
	//储存卡尔曼滤波的队列
	deque<kalman_store> kalman_deque = this->kalman_deque;
	//储存预测点
	vector<Mat> predictions;
	//遍历卡尔曼滤波队列
	for (int i = 0; i < kalman_deque.size(); i++) {
		//如果匹配位为1，则进行预测
		if (kalman_deque[i].match_flag == 1) {
			//进行预测
			Mat prediction = kalman_deque[i].KF.predict();
			predictions.push_back(prediction);
			//清空标志位
			this->kalman_deque[i].match_flag = 0;
		}
		
	}

	//更新到属性
	this->predictions = predictions;
}


/**
* @brief 卡尔曼滤波器匹配
*
*/
void ArmorDetector::kalman_match(void) {
	//储存卡尔曼滤波的队列
	deque<kalman_store> kalman_deque = this->kalman_deque;
	//取出装甲板的四点
	vector<Armor> armors_store = this->armors.armors_store;

	for (int i = 0; i < armors_store.size(); i++) {
		//装甲板的中点
		Point2f armors_middle = (armors_store[i].middlePoints[0] + armors_store[i].middlePoints[3]) / 2;
		//是否匹配到的标志位
		int is_match = 0;

		//遍历卡尔曼滤波队列
		for (int j = 0; j < kalman_deque.size(); j++) {
			//测量到的坐标
			Point2f measurement_point;
			measurement_point.x = kalman_deque[j].measurement.at<float>(0);
			measurement_point.y = kalman_deque[j].measurement.at<float>(1);

			//装甲板的中点与测量到的点的距离
			float distance = this->distance(armors_middle, measurement_point);
			//小于一定距离则匹配正确
			if (distance < 30) {
				//把匹配标志位设为是
				kalman_deque[j].match_flag = 1;
				//更新卡尔曼滤波
				kalman_deque[j].measurement.at<float>(0) = armors_middle.x;
				kalman_deque[j].measurement.at<float>(1) = armors_middle.y;
				kalman_deque[j].KF.correct(kalman_deque[j].measurement);
				//是否匹配到的标志位定为1
				is_match = 1;
			}
		}
		//如果没匹配到就初始化一个滤波器
		if (is_match == 0) {
			//定义一个卡尔曼滤波器仓库
			kalman_store store;
			//初始化卡尔曼滤波器
			store.KF = this->kalman_init();
			//更新卡尔曼滤波器
			store.measurement.at<float>(0) = armors_middle.x;
			store.measurement.at<float>(1) = armors_middle.y;
			store.KF.correct(store.measurement);
			//将这个仓库加入队列
			kalman_deque.push_back(store);

		}
	}
	//更新到属性
	this->kalman_deque = kalman_deque;

}


/**
* @brief 卡尔曼滤波器清算
*
*/
void ArmorDetector::kalman_clear(void) {
	//储存卡尔曼滤波的队列
	deque<kalman_store> kalman_deque = this->kalman_deque;
	//更正的卡尔曼滤波的队列
	deque<kalman_store> correct_kalman_deque;

	//遍历队列
	for (int i = 0; i < kalman_deque.size(); i++) {
		//如果匹配的标志位为是，则匹配次数加一
		if (kalman_deque[i].match_flag == 1) {
			kalman_deque[i].unmatch_num++;
		}
		//如果匹配次数小于5次，就加入更正的队列
		if (kalman_deque[i].unmatch_num < 5) {
			correct_kalman_deque.push_back(kalman_deque[i]);
		}

	}
	//重新更新更正队列到属性
	this->kalman_deque = correct_kalman_deque;
}


/**
* @brief 卡尔曼滤波预测绘制
*
*/
void ArmorDetector::draw_kalman_predict(void) {
	vector<Mat> predictions = this->predictions;
	for (int i = 0; i < predictions.size(); i++) {
		Point2f predict_pt;
		predict_pt.x = predictions[i].at<float>(0);
		predict_pt.y = predictions[i].at<float>(1);
		circle(this->img, predict_pt, 12, Scalar(255, 255, 0), 3);
	}
}


/**
* @brief 计算两点之间的距离
*
* @param first_point 第一个点
* @param second_point 第二个点
* @return float distance 两点之间的距离
*/
float ArmorDetector::distance(Point2f first_point, Point2f second_point) {
	return sqrt(pow((first_point.x - second_point.x), 2) + pow((first_point.y - second_point.y), 2));
}