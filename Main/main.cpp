#include "ArmorDetector.h"
#include "EnergyMechanism.h"


int main() {
	int mode;
	cout << "��ѡ��ģʽ��" << endl;
	cout << "��������1ʶ��װ�װ�" << endl;
	cout << "��������2ʶ����������" << endl;
	cin >> mode;
	if (mode == 1) {
		cout << "������з�װ�װ���ɫ��0Ϊ��ɫ��1Ϊ��ɫ" << endl;
		int color;
		cin >> color;
		ArmorDetector detector;
		detector.color = color;
		cout << "��������Ƶ���ƣ���Ҫ����׺��" << endl;
		string filename;
		cin >> filename;
		detector.video_detect(filename);
	}
	else if (mode == 2) {
		cout << "��������Ҫ������������ص���ɫ��0Ϊ��ɫ��1Ϊ��ɫ" << endl;
		int color;
		cin >> color;
		EnergyMechanism detector;
		detector.color = color;
		cout << "��������Ƶ���ƣ���Ҫ����׺��" << endl;
		string filename;
		cin >> filename;
		detector.video_detect(filename);
	}
	else {
		cout << "ģʽѡ��������������д˳���" << endl;
	}
	
	return 0;
}
