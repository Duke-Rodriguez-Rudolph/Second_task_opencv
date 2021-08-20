#include "ArmorDetector.h"
#include "EnergyMechanism.h"


int main() {
	int mode;
	cout << "请选择模式：" << endl;
	cout << "输入数字1识别装甲板" << endl;
	cout << "输入数字2识别能量机关" << endl;
	cin >> mode;
	if (mode == 1) {
		cout << "请输入敌方装甲板颜色，0为蓝色，1为红色" << endl;
		int color;
		cin >> color;
		ArmorDetector detector;
		detector.color = color;
		cout << "请输入视频名称（需要带后缀）" << endl;
		string filename;
		cin >> filename;
		detector.video_detect(filename);
	}
	else if (mode == 2) {
		cout << "请输入需要击打的能量机关的颜色，0为红色，1为蓝色" << endl;
		int color;
		cin >> color;
		EnergyMechanism detector;
		detector.color = color;
		cout << "请输入视频名称（需要带后缀）" << endl;
		string filename;
		cin >> filename;
		detector.video_detect(filename);
	}
	else {
		cout << "模式选择错误，请重新运行此程序！" << endl;
	}
	
	return 0;
}
