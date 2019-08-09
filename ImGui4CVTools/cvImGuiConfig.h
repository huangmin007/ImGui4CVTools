#pragma once

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>

#include <GL/gl3w.h> 
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


using namespace cv;
using namespace std;

#define MAX_CHAR 0xFF

//ImGui::Buggon 按扭高
static float BTN_HEIGHT = 22.0;

#define BUTTON_HEIGHT			22.0
#define EXCE_BUTTON_SIZE		(ImVec2(ImGui::GetContentRegionAvail().x,BUTTON_HEIGHT))


//ImGui CV API Argument Left width
static float COL_LEFT_WIDTH = 110.0;

static int ARG_SHIFT_MIN_VALUE = 0;
static int ARG_SHIFT_MAX_VALUE = 8;

static int ARG_PT_MIN_VALUE = 0;
static int ARG_PT_MAX_VALUE = 1920;

static int ARG_THICK_MIN_VALUE = -1;
static int ARG_THICK_MAX_VALUE = 10;

static float ARG_TIPLEN_MIN_VALUE = -3.0;
static float ARG_TIPLEN_MAX_VALUE = -3.0;

//cv data type char
static const char* cv_dtype_char[40] = {};
static map <const char*, int>::const_iterator cv_dtype_it;
static const map<const char*, int> cv_dtype_map = 
{
	pair<const char*, int>("CV_0U", -1),
	pair<const char*, int>("CV_8U", CV_8U),
	pair<const char*, int>("CV_8S", CV_8S),
	pair<const char*, int>("CV_16U", CV_16U),
	pair<const char*, int>("CV_16S", CV_16S),
	pair<const char*, int>("CV_32S", CV_32S),
	pair<const char*, int>("CV_32F", CV_32F),
	pair<const char*, int>("CV_64F", CV_64F),
	pair<const char*, int>("CV_16F", CV_16F),

	pair<const char*, int>("CV_8UC1", CV_8UC1),
	pair<const char*, int>("CV_8UC2", CV_8UC2),
	pair<const char*, int>("CV_8UC3", CV_8UC3),
	pair<const char*, int>("CV_8UC4", CV_8UC4),

	pair<const char*, int>("CV_8SC1", CV_8SC1),
	pair<const char*, int>("CV_8SC2", CV_8SC2),
	pair<const char*, int>("CV_8SC3", CV_8SC3),
	pair<const char*, int>("CV_8SC4", CV_8SC4),

	pair<const char*, int>("CV_16UC1", CV_16UC1),
	pair<const char*, int>("CV_16UC2", CV_16UC2),
	pair<const char*, int>("CV_16UC3", CV_16UC3),
	pair<const char*, int>("CV_16UC4", CV_16UC4),

	pair<const char*, int>("CV_16SC1", CV_16SC1),
	pair<const char*, int>("CV_16SC2", CV_16SC2),
	pair<const char*, int>("CV_16SC3", CV_16SC3),
	pair<const char*, int>("CV_16SC4", CV_16SC4),

	pair<const char*, int>("CV_32SC1", CV_32SC1),
	pair<const char*, int>("CV_32SC2", CV_32SC2),
	pair<const char*, int>("CV_32SC3", CV_32SC3),
	pair<const char*, int>("CV_32SC4", CV_32SC4),

	pair<const char*, int>("CV_32FC1", CV_32FC1),
	pair<const char*, int>("CV_32FC2", CV_32FC2),
	pair<const char*, int>("CV_32FC3", CV_32FC3),
	pair<const char*, int>("CV_32FC4", CV_32FC4),

	pair<const char*, int>("CV_64FC1", CV_64FC1),
	pair<const char*, int>("CV_64FC2", CV_64FC2),
	pair<const char*, int>("CV_64FC3", CV_64FC3),
	pair<const char*, int>("CV_64FC4", CV_64FC4),

	pair<const char*, int>("CV_16FC1", CV_16FC1),
	pair<const char*, int>("CV_16FC2", CV_16FC2),
	pair<const char*, int>("CV_16FC3", CV_16FC3),
	pair<const char*, int>("CV_16FC4", CV_16FC4),
};

static int get_cv_dtype(int index)
{
	cv_dtype_it = cv_dtype_map.begin();
	for (int i = 0; cv_dtype_it != cv_dtype_map.end(); cv_dtype_it++, i++)
		if (index == i)	return cv_dtype_it->second;

	return -1;
}

enum LogType
{
	Info,
	Warn,
	Error,
	Code,
};


struct CVAPIAppLog
{
	ImGuiTextFilter     Filter;
	bool                AutoScroll;   

	ImVector<char*>			Items;
	ImVector<LogType>		Types;
	
	SYSTEMTIME				sys;
	const char *s_type[4] = { "info", "warn", "error", "code" };

	CVAPIAppLog()
	{
		AutoScroll = true;
		Clear();
	}

	static char* Strdup(const char *str) 
	{ 
		size_t len = strlen(str) + 1; 
		void* buf = malloc(len); 
		IM_ASSERT(buf); 

		return (char*)memcpy(buf, (const void*)str, len); 
	}

	void    Clear()
	{
		Types.clear();
		Items.clear();
	}

	//添加日志信息
	void    AddLog(LogType type, const char* fmt, ...) IM_FMTARGS(2)
	{
		GetLocalTime(&sys);
		
		char buf[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
		buf[IM_ARRAYSIZE(buf) - 1] = 0;
		va_end(args);

		char head[1024];
		sprintf_s(head, 1024, "[%02d:%02d:%02d.%03d] [%s] %s", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, s_type[type], buf);

		Types.push_back(type);
		Items.push_back(Strdup(head));
	}
	//添加异常信息
	void    AddLog(Exception ex, const char* fmt, ...) IM_FMTARGS(2)
	{
		GetLocalTime(&sys);
		LogType type = LogType::Error;

		char buf[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
		buf[IM_ARRAYSIZE(buf) - 1] = 0;
		va_end(args);

		char head[1024];
		sprintf_s(head, 1024, "[%02d:%02d:%02d.%03d] [%s] %s%s", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, s_type[type], buf, ex.what());

		Types.push_back(type);
		Items.push_back(Strdup(head));
	}

	void    Draw(const char* title, bool* p_open = NULL)
	{
		if (!ImGui::Begin(title, p_open))
		{
			ImGui::End();
			return;
		}

		// Options menu
		if (ImGui::BeginPopup("Options"))
		{
			ImGui::Checkbox("Auto-scroll", &AutoScroll);
			ImGui::EndPopup();
		}

		// Main window
		if (ImGui::Button("Options"))
			ImGui::OpenPopup("Options");
		ImGui::SameLine();

		bool clear = ImGui::Button("Clear");
		ImGui::SameLine();

		bool copy = ImGui::Button("Copy");
		ImGui::SameLine();

		Filter.Draw("Filter", -100.0f);

		ImGui::Separator();
		ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

		if (clear)			Clear();
		if (copy)			ImGui::LogToClipboard();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

		for (int i = 0; i < Items.Size; i++)
		{
			static ImVec4 color;
			const LogType type = Types[i];
			const char* item = Items[i];
			
			if (!Filter.PassFilter(item))	continue;

			color = type == LogType::Error	? ImVec4(1.0f, 0.0f, 0.0f, 1.0f) :
					type == LogType::Warn	? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) :
					type == LogType::Code	? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) :
					type == LogType::Info	? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) :
					ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

			ImGui::PushStyleColor(ImGuiCol_Text, color);
			ImGui::TextUnformatted(item);
			ImGui::PopStyleColor();
		}

		if (copy)			ImGui::LogFinish();

		if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);

		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::End();
	}
};




static CVAPIAppLog Logger;
static void AddMarker(const char* desc);
static void AddHelpMarker(const char* desc);
static void AddLeftLabel(const char *label, const char *helpDes = NULL);

static void ShowLoggerWindow(bool* p_open);
static void ShowCVAPIWindow(GLFWwindow *window);
static void ShowCVAPIHelpWindow(bool *p_open);
static void SetDisplayDPI(GLFWwindow *window, float scale);

//i:inputArray
//o:outputArray
//d:int/uint
//f:float/double
//_:下划线之后表示可选参数，或使用默认参数
typedef void(*iio_id)(InputArray src1, InputArray src2, OutputArray dst, InputArray mask, int dtype);
typedef void(*iio_fd)(InputArray src1, InputArray src2, OutputArray dst, double scale, int dtype);

typedef void(*iio_i)(InputArray src1, InputArray src2, OutputArray dst, InputArray mask);
typedef void(*io_i)(InputArray src1, OutputArray src2, InputArray mask);

typedef void(*iods_pbd)(InputArray src, OutputArray dst, int ddepth, Size ksize, Point anchor, bool normalize,int borderType);