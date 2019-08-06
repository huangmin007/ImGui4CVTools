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

	//渲染图像窗体
	void Render();

	//加载 CV Mat对象
	void LoadMat(Mat img);
	void LoadMat(Mat img, const char *title);
	//void LoadFile(const char *fn);

	void UpdateMat();

	//获取 Mat 对象
	Mat& GetMat();
	const char* GetTitle();
	const char* GetNextTitle(const char *title);

	ImVec2 GetViewerPos();
	ImVec2 GetNextViewerPos();
	void SetViewerPos(ImVec2 pos);

	ImVec2 GetViewerSize();


	//是否显示 cv imshow 窗体
	bool cv_imshow = false;

	//Mat视图是否是打开状态
	bool is_open = true;

private:
	char n_title[MAX_CHAR];
	//char n_filename[MAX_CHAR];

	Mat n_image;
	Mat texture;
	GLuint texture_id = -1;

	//默认按扭大小
	const ImVec2 btn_size = ImVec2(40, BTN_HEIGHT);
	//窗体允许缩放比例
	const double rate[6] = { 2.0, 1.5, 1.0, 0.8, 0.5, 0.2 };

	ImVec2 pos = ImVec2(40, 40);
	ImVec2 size;
};

