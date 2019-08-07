#pragma once

#include "cvImGuiConfig.h"


class MatViewer
{
public:
	MatViewer();
	MatViewer(const char *title);
	MatViewer(const char *title, Mat image);
	//MatViewer(const char *title, const char *filename);
	~MatViewer();

	//��Ⱦͼ����
	void Render();

	//���� CV Mat����
	void LoadMat(Mat img);
	void LoadMat(Mat img, const char *title);
	//void LoadFile(const char *fn);

	void UpdateMat();

	//��ȡ Mat ����
	Mat& GetMat();
	const char* GetTitle();
	const char* GetNextTitle(const char *title);

	ImVec2 GetViewerPos();
	ImVec2 GetNextViewerPos();
	void SetViewerPos(ImVec2 pos);

	ImVec2 GetViewerSize();


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
	Mat texture;
	GLuint texture_id = -1;
	
	ImVec2 pos = ImVec2(30, 30);
};

