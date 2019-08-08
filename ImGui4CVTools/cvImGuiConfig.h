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

//cv rtype char
static const char* cv_rtype_char[40] = {};
static map <const char*, int>::const_iterator cv_rtype_it;
static const map<const char*, int> cv_rtype_map = 
{ 
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

static int GetCVRtype(int index)
{
	cv_rtype_it = cv_rtype_map.begin();
	for (int i = 0; cv_rtype_it != cv_rtype_map.end(); cv_rtype_it++, i++)
		if (index == i)	return cv_rtype_it->second;

	return -1;
}


// Usage:
//  static ExampleAppLog my_log;
//  my_log.AddLog("Hello %d world\n", 123);
//  my_log.Draw("title");
struct ExampleAppLog
{
	ImGuiTextBuffer     Buf;
	ImGuiTextFilter     Filter;
	ImVector<int>       LineOffsets;        // Index to lines offset. We maintain this with AddLog() calls, allowing us to have a random access on lines
	bool                AutoScroll;     // Keep scrolling if already at the bottom

	ExampleAppLog()
	{
		AutoScroll = true;
		Clear();
	}

	void    Clear()
	{
		Buf.clear();
		LineOffsets.clear();
		LineOffsets.push_back(0);
	}

	void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
	{
		int old_size = Buf.size();
		va_list args;
		va_start(args, fmt);
		Buf.appendfv(fmt, args);
		va_end(args);
		for (int new_size = Buf.size(); old_size < new_size; old_size++)
			if (Buf[old_size] == '\n')
				LineOffsets.push_back(old_size + 1);
	}

	void    Draw(const char* title, bool* p_open = NULL)
	{
		if (!ImGui::Begin(title, p_open))
		{
			ImGui::End();
			return;
		}
		printf("draw....\n");

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

		if (clear)
			Clear();
		if (copy)
			ImGui::LogToClipboard();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		const char* buf = Buf.begin();
		const char* buf_end = Buf.end();
		if (Filter.IsActive())
		{
			// In this example we don't use the clipper when Filter is enabled.
			// This is because we don't have a random access on the result on our filter.
			// A real application processing logs with ten of thousands of entries may want to store the result of search/filter.
			// especially if the filtering function is not trivial (e.g. reg-exp).
			for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
			{
				const char* line_start = buf + LineOffsets[line_no];
				const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
				if (Filter.PassFilter(line_start, line_end))
				{
					bool pop_color = false;
					printf("test strstr::%s\n", strstr(line_start, "[error]"));
					if (strstr(line_start, "[error]")) 
					{
						printf("error......\n");
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f)); 
						pop_color = true;
					}

					ImGui::TextUnformatted(line_start, line_end);
					if (pop_color)	ImGui::PopStyleColor();
				}
			}
		}
		else
		{
			// The simplest and easy way to display the entire buffer:
			//   ImGui::TextUnformatted(buf_begin, buf_end);
			// And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward to skip non-visible lines.
			// Here we instead demonstrate using the clipper to only process lines that are within the visible area.
			// If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them on your side is recommended.
			// Using ImGuiListClipper requires A) random access into your data, and B) items all being the  same height,
			// both of which we can handle since we an array pointing to the beginning of each line of text.
			// When using the filter (in the block of code above) we don't have random access into the data to display anymore, which is why we don't use the clipper.
			// Storing or skimming through the search result would make it possible (and would be recommended if you want to search through tens of thousands of entries)
			ImGuiListClipper clipper;
			clipper.Begin(LineOffsets.Size);
			while (clipper.Step())
			{
				for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
				{
					const char* line_start = buf + LineOffsets[line_no];
					const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
					printf("aaaa\n");
					ImGui::TextUnformatted(line_start, line_end);
				}
			}
			clipper.End();
		}
		ImGui::PopStyleVar();

		if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);

		ImGui::EndChild();
		ImGui::End();
	}
};

enum LogType
{
	Info,
	Warn,
	Error,
	Codes,
};


static ExampleAppLog logger;
static void AddLogger(Exception ex, const char *msg, ...);
static void AddLogger(LogType type, const char *msg, ...);
static void AddMarker(const char* desc);
static void AddHelpMarker(const char* desc);
static void AddLeftLabel(const char *label, const char *helpDes = NULL);

static void ShowLoggerWindow(bool* p_open);
static void ShowCVAPIWindow(GLFWwindow *window);
static void ShowCVAPIHelpWindow(bool *p_open);
static void ShowMainMenuBar(bool &m_showDemoWindow, bool &m_showCVAPIWindow, bool &m_showLogger, bool &m_showCVAPIHelp);
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
//typedef void(*iod_d)(InputArray src1, OutputArray src2, int code, int dstCn);
