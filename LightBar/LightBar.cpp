#include "LightBar.h"

/**
* @brief ѡ�����
*
* @param binaryImg ��ֵ��ͼ
*/
void LightBars::select_lightbar(Mat binaryImg) {	
	//��������ת���β���
	vector<RotatedRect> rect_boxes;
	//�����������
	vector<vector<Point>> lightContours;	
	findContours(binaryImg, lightContours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);	//������

	for (int i = 0; i < lightContours.size(); i++) {
		//�����������
		double contour_area = contourArea(lightContours[i]);	
		if (contour_area > 10) {
			if (lightContours[i].size() < 6) { continue; }	//��Բ��ϱ���Ҫ����������
			//������С��ת����
			RotatedRect rect = fitEllipse(lightContours[i]);	
			revise_oval(rect);
			//�����߱�
			double aspect_ratio = rect.size.width / rect.size.height;
			//ʵ����������С��������ȣ��־ر�
			double proportion = contour_area / rect.size.area();	

			//cout <<"all aspect_ratio:"<< aspect_ratio << endl;	//�鿴���������Ŀ�߱�
			//cout << "all proportion:" << proportion << endl;	//�鿴�����������־ر�

			if (aspect_ratio < 0.1 || proportion < 0.58) { continue; }

			//cout << "light_bar aspect_ratio:" << aspect_ratio << endl;	//�鿴�����Ŀ�߱�
			//cout << "light_bar proportion:" << proportion << endl;	//�鿴�����־ر�
			//cout << "light_bar angle:" << rect.angle << endl<<endl;	//�鿴�����Ƕ�
			rect_boxes.push_back(rect);

		}
	}

	this->rect_boxes = rect_boxes;
}


/**
* @brief ����Բ��angle��[0,180]תΪ[-90,90]
*
* @param rect ��Ҫ��������Բ����
*/
void LightBars::revise_oval(RotatedRect &rect) {	
	if (rect.angle > 90) {
		rect.angle -= 180;
	}
}


/**
* @brief ������
*
* @param img ��Ҫ���ƵĻ���
*/
void LightBars::draw_lightbars(Mat &img) {	
	for (int i = 0; i < this->rect_boxes.size(); i++) {
		//���λ��ĵ�
		Point2f rect_points[4];	
		this->rect_boxes[i].points(rect_points);


		for (int j = 0; j <= 3; j++)
		{
			line(img, rect_points[j], rect_points[(j + 1) % 4], Scalar(0, 0, 255), 2);
		}
	}
}

