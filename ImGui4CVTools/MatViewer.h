#pragma once

#include "cvImGuiConfig.h"


class MatViewer
{
public:
	MatViewer();
	MatViewer(const char *title);
	MatViewer(const char *title, Mat image);
	~MatViewer();

	//渲染图像窗体
	void Render();

	//加载 CV Mat 对象
	void LoadMat(Mat img);
	void LoadMat(Mat img, const char *title);

	//更新 Mat 显示，针对 ImGui 显示
	//只有 is_render = true 时渲染才有效
	void UpdateMat();

	//获取 Mat 对象，返回 Mat& 对象
	//返回的是非 const Mat& 对象，所以外部是可以修改的，然后调用 UpdateMat() 刷新显示
	Mat& GetMat();

	//OpenCV Mat to glTexture
	static GLuint cvMat2glTexture(const Mat &image);

	//获取视图标题
	const char* GetTitle();

	//获取下一个视图标题
	//是跟据此视图标题加上 title 生成出来另一个新的视图的标题
	const char* GetNextTitle(const char *title);

	ImVec2 GetViewerPos();
	ImVec2 GetNextViewerPos();
	void SetViewerPos(ImVec2 pos);

	//是否显示 cv imshow 窗体
	bool cv_imshow = false;

	//Mat视图是否是打开状态
	bool is_open = true;

	//是否渲染输出显示图片
	bool is_render = true;


private:
	char n_title[MAX_CHAR];

	vector<string> codes;

	Mat n_image;
	//Mat texture;
	GLuint texture_id = -1;
	
	ImVec2 pos = ImVec2(30, 30);
};

