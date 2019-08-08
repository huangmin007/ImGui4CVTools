#pragma once

#include "cvImGuiConfig.h"


class MatViewer
{
public:
	MatViewer();
	MatViewer(const char *title);
	MatViewer(const char *title, Mat image);
	~MatViewer();

	//��Ⱦͼ����
	void Render();

	//���� CV Mat ����
	void LoadMat(Mat img);
	void LoadMat(Mat img, const char *title);

	//���� Mat ��ʾ����� ImGui ��ʾ
	//ֻ�� is_render = true ʱ��Ⱦ����Ч
	void UpdateMat();

	//��ȡ Mat ���󣬷��� Mat& ����
	//���ص��Ƿ� const Mat& ���������ⲿ�ǿ����޸ĵģ�Ȼ����� UpdateMat() ˢ����ʾ
	Mat& GetMat();

	//OpenCV Mat to glTexture
	static GLuint cvMat2glTexture(const Mat &image);

	//��ȡ��ͼ����
	const char* GetTitle();

	//��ȡ��һ����ͼ����
	//�Ǹ��ݴ���ͼ������� title ���ɳ�����һ���µ���ͼ�ı���
	const char* GetNextTitle(const char *title);

	ImVec2 GetViewerPos();
	ImVec2 GetNextViewerPos();
	void SetViewerPos(ImVec2 pos);

	//�Ƿ���ʾ cv imshow ����
	bool cv_imshow = false;

	//Mat��ͼ�Ƿ��Ǵ�״̬
	bool is_open = true;

	//�Ƿ���Ⱦ�����ʾͼƬ
	bool is_render = true;


private:
	char n_title[MAX_CHAR];

	vector<string> codes;

	Mat n_image;
	//Mat texture;
	GLuint texture_id = -1;
	
	ImVec2 pos = ImVec2(30, 30);
};

