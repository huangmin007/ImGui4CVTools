#pragma once

#include "cvImGuiConfig.h"
#include "MatViewer.h"

//	OpenCV Main Viewer ��ͼ
//
class CVViewer
{
public:
	CVViewer();
	~CVViewer();

	//��Ⱦ
	void Render();

private:

	GLFWwindow *window;

	bool m_showLogWindow = true;
	bool m_showDemoWindow = false;
	bool m_showCVAPIWindow = true;
	bool m_showCVAPIHelp = true;

};

