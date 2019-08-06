#include "stdafx.h"
#include "CVViewer.h"

#include <fstream>
#include <sstream>
#include <string>
#include <time.h>

#include "cvImGuiConfig.h";
#include "CVAPIUtils.h"

#include "MatViewer.h"
#include "MatViewerManager.h"

//窗体标题
const char window_title[] = "ImGui for OpenCV Tools Viewer";
//窗体背景颜色
const ImVec4 ClearColor(0.01f, 0.01f, 0.01f, 1.0f);

void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);

	std::ofstream errorLogger;
	errorLogger.open("k4aviewer.err", std::ofstream::out | std::ofstream::app);
	errorLogger << "Glfw error [" << error << "]: " << description << std::endl;
	errorLogger.close();
}


//	OpenCV Main Viewer 视图
CVViewer::CVViewer()
{
	m_showDemoWindow = false;
	glfwSetErrorCallback(glfw_error_callback);

	if (!glfwInit())
	{
		glfw_error_callback(0, "glfwInit failed!");
		exit(-1);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(1280, 720, window_title, nullptr, nullptr);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	gl3wInit();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 16.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	ImGui::StyleColorsDark();
	ImGui::GetStyle().WindowRounding = 0.0f;
	ImGui::GetStyle().FrameBorderSize = 1.0f;

	SetDisplayDPI(window, 1.0f);

	//禁用保存窗口布局
	//ImGui::GetIO().IniFilename = nullptr;
}

CVViewer::~CVViewer()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}

void CVViewer::Render()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		//开始 ImGui 帧
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ShowMainMenuBar(m_showDemoWindow, m_showCVAPIWindow, m_showLogWindow);

		ShowCVAPIWindow(window);
		if (m_showLogWindow)	ShowLoggerWindow(&m_showLogWindow);
		if (m_showDemoWindow)	ImGui::ShowDemoWindow(&m_showDemoWindow);

		//Render CV Mat Viewers
		MatViewerManager::Instance().RenderViewer();

		// 完成/渲染帧
		ImGui::Render();
		int displayW, displayH;
		glfwMakeContextCurrent(window);
		glfwGetFramebufferSize(window, &displayW, &displayH);
		glViewport(0, 0, displayW, displayH);
		glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}
}

//添加日志信息
static void AddLogger(LogType type, const char *msg, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, IM_ARRAYSIZE(buf), msg, args);
	buf[IM_ARRAYSIZE(buf) - 1] = 0;
	va_end(args);

	SYSTEMTIME sys;
	GetLocalTime(&sys);

	static const char *s_type[3] = { "info", "warn", "error" };
	logger.AddLog("[%02d:%02d:%02d.%03d][%s]  %s", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, s_type[type], buf);
}
//添加异常信息
static void AddLogger(Exception ex, const char *msg)
{
	AddLogger(LogType::Error, "%s: \n\t\t\t\t%s\n", msg, ex.what());
	//AddLogger(LogType::Error, "%s: code:%d func:%s msg:%s, line:%d\n \t\t\t\n%s\n", msg, e.code, e.func, e.msg, e.line, e.what());
}

//添加 help 标记
static void AddHelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

//设备显示 DPI 比率
static void SetDisplayDPI(GLFWwindow *window, float scale)
{
	float HighDpiScaleFactor = scale;

	ImGui::GetStyle().ScaleAllSizes(HighDpiScaleFactor);

	ImFontConfig fontConfig;
	constexpr float defaultFontSize = 13.0f;
	fontConfig.SizePixels = defaultFontSize * HighDpiScaleFactor;
	ImGui::GetIO().Fonts->AddFontDefault(&fontConfig);

	int w;
	int h;
	glfwGetWindowSize(window, &w, &h);
	w = static_cast<int>(w * HighDpiScaleFactor);
	h = static_cast<int>(h * HighDpiScaleFactor);
	glfwSetWindowSize(window, w, h);
}

//显示主菜单栏
static void ShowMainMenuBar(bool &m_showDemoWindow, bool &m_showCVAPIWindow, bool &m_showLogger)
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu(u8"Settings 设置"))
		{
			if (ImGui::MenuItem("Show ImGui Demo")) 
			{
				m_showDemoWindow = !m_showDemoWindow;
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Show CV API Demo", NULL, true))
			{
				m_showCVAPIWindow = !m_showCVAPIWindow;
			}
			if (ImGui::MenuItem("Quit", "Alt+F4"))
			{
				exit(1);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Devloper"))
		{
			if (ImGui::MenuItem("Logger"))
			{
				m_showLogger = !m_showLogger;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

//显示日志窗体
static void ShowLoggerWindow(bool* p_open)
{
	ImGui::SetNextWindowPos(ImVec2(1000, 25), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

	ImGui::Begin("Logger", p_open);	
	ImGui::End();

	logger.Draw("Logger", p_open);
}

//添加 Viewer List Guide Combo 
static void AddViewerCombo(const char* label, int *out_index, bool input = true, ...)
{
	const int items_count = MatViewerManager::Instance().GetCount();
	const char **items = MatViewerManager::Instance().GetAllTitle();
	const char **n_itmes = MatViewerManager::Instance().GetAllTitle("Create New Viewer");

	ImGui::Text(label);
	ImGui::SameLine(COL_LEFT_WIDTH);

	static char c_label[255] = {};
	sprintf_s(c_label, "%s%s", "##", label);
	
	if (input)
	{
		AddHelpMarker(u8"选择输入视图窗口的 InputArray 对象");
		ImGui::SameLine();
		ImGui::Combo(c_label, out_index, items, items_count);
	}
	else
	{
		AddHelpMarker(u8"选择出视图窗口的 InputArray 对象\n选择 [Create New Viewer] 或不选择，表示创建新的 InputArray 对象 或是默认参数 或是参数为空");
		ImGui::SameLine();
		ImGui::Combo(c_label, out_index, n_itmes, items_count + 1);
	}
}

//显示 cv api 操作
static void ShowCVAPIWindow(GLFWwindow *window)
{
	ImGui::SetNextWindowPos(ImVec2(10, 25), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(500, 700), ImGuiCond_FirstUseEver);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoFocusOnAppearing;

	//OpenCV API Tools Demo Start
	if (ImGui::Begin("OpenCV API Tools Demo", NULL, flags))
	{
		//static Mat output_result;
		const int items_count = MatViewerManager::Instance().GetCount();

		//API imread
		if (ImGui::Button("imread", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)))
		{
			char m_fileName[MAX_PATH];
			if (SelectImageFile(window, m_fileName))
			{
				//current time			
				char c_time[32];
				GetCurrentForamtTime(c_time, 32, "[%X]");

				//file name
				//const char *fn = strrchr(m_fileName, '\\') + 1;
				//printf("file:%s\n", fn);

				//api title
				char title[MAX_CHAR];
				sprintf_s(title, MAX_CHAR, "%s || %s", c_time, "imread");
				AddLogger(LogType::Info, "select file: %s\n", m_fileName);

				Mat source;
				static int flags = ImreadModes::IMREAD_COLOR;
				try
				{
					source = imread(m_fileName, flags);
					MatViewerManager::Instance().AddViewer(new MatViewer(title, source));
					AddLogger(LogType::Info, "imread() create viewer: %s\n", title);
				}
				catch (Exception e)
				{
					AddLogger(e, "imread() error");
				}

				source.release();
			}
		}

		//Base API
		if (ImGui::CollapsingHeader("Base API"))
		{
			//cvtColor()
			if (ImGui::TreeNode(u8"色彩空间转换 cvtColor()"))
			{
				static EnumParser<ColorConversionCodes> CVAPI_ColorConversionCodes;

				//input
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index, true);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//code
				ImGui::Text("code");
				ImGui::SameLine(COL_LEFT_WIDTH); 
				AddHelpMarker(u8"颜色空间转换代码(更多值见 #ColorConversionCodes)");
				ImGui::SameLine();
				static int arg_code_index = 0;
				ImGui::Combo("##code", &arg_code_index, CVAPI_ColorConversionCodes.ParseMap2Items(), CVAPI_ColorConversionCodes.GetMapCount());

				//dstCn
				ImGui::Text("dstCn");
				ImGui::SameLine(COL_LEFT_WIDTH); AddHelpMarker(u8"目标图像中的通道数；如果参数为0，则通道数将自动从源图和代码中派生。");
				ImGui::SameLine();
				static int arg_dstCn_index = 0;
				ImGui::SliderInt("##dstCn", &arg_dstCn_index, 0, 4);

				//执行 cvtColor()
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("cvtColor"));

					try
					{
						cvtColor(input_viewer->GetMat(), output_viewer->GetMat(), CVAPI_ColorConversionCodes.ParseIndex2Enum(arg_code_index));
						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						AddLogger(LogType::Info, "cvtColor() succeeded: %s\n", output_viewer->GetTitle());
					}
					catch (Exception e)
					{
						AddLogger(LogType::Info, "cvtColor() error", input_viewer->GetTitle());
					}

					input_viewer = NULL;
					output_viewer = NULL;
				}
				ImGui::TreePop();
			}

			//flip()
			if (ImGui::TreeNode(u8"图像翻转 flip()"))
			{
				//input
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//flipCode
				ImGui::Text("flipCode");
				ImGui::SameLine(COL_LEFT_WIDTH); AddHelpMarker(u8"0表示绕x轴翻转，1表示绕y轴翻转，-1表示在两个轴周围翻转");
				ImGui::SameLine();
				static int arg_flipCode_index = 0;
				ImGui::SliderInt("##dstCn", &arg_flipCode_index, -1, 1);

				//执行 flip()
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("flip"));

					try
					{
						flip(input_viewer->GetMat(), output_viewer->GetMat(), arg_flipCode_index);

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						AddLogger(LogType::Info, "flip() succeeded: %s\n", output_viewer->GetTitle());
					}
					catch (Exception e)
					{
						AddLogger(e, "flip() error");
					}
					input_viewer = NULL;
					output_viewer = NULL;
				}
				ImGui::TreePop();
			}

			//rotate()

		}

		//图像运算操作 API
		if (ImGui::CollapsingHeader(u8"图像运算操作"))
		{
			//add()
			if (ImGui::TreeNode(u8"add()"))
			{
				//input1
				static int arg_input1_index = -1;
				AddViewerCombo("input1", &arg_input1_index, true);

				//input2
				static int arg_input2_index = -1;
				AddViewerCombo("input2", &arg_input2_index, true);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//mask
				static int arg_mask_index = -1;
				AddViewerCombo("mask", &arg_output_index, false);

				//dtype
				ImGui::Text("dtype");
				ImGui::SameLine(COL_LEFT_WIDTH);
				AddHelpMarker(u8"输出可选数组深度，默认为 -1");
				ImGui::SameLine();
				static int dtype = -1;
				ImGui::SliderInt("##dtype", &dtype, -1, 4);

				//执行 add()
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) &&
					(arg_input1_index != -1 && arg_input2_index != -1))
				{
					MatViewer *input1_viewer = MatViewerManager::Instance().GetViewer(arg_input1_index);
					MatViewer *input2_viewer = MatViewerManager::Instance().GetViewer(arg_input2_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input2_viewer->GetNextTitle("add"));
					InputArray mask = arg_mask_index == -1 || arg_mask_index == items_count ? noArray() : MatViewerManager::Instance().GetViewer(arg_mask_index)->GetMat();

					try
					{
						add(input1_viewer->GetMat(), input2_viewer->GetMat(), output_viewer->GetMat(), mask, dtype);

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input2_viewer->GetNextViewerPos());

						AddLogger(LogType::Info, "add() succeeded: %s\n", output_viewer->GetTitle());
					}
					catch (Exception e)
					{
						AddLogger(e, "add() error");
					}

					mask.~_InputArray();
					input1_viewer = NULL;
					input2_viewer = NULL;
					output_viewer = NULL;
				}
				ImGui::TreePop();
			}

			//subtract()
			if (ImGui::TreeNode(u8"subtract()"))
			{
				//input1
				static int arg_input1_index = -1;
				AddViewerCombo("input1", &arg_input1_index, true);

				//input2
				static int arg_input2_index = -1;
				AddViewerCombo("input2", &arg_input2_index, true);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//mask
				static int arg_mask_index = -1;
				AddViewerCombo("mask", &arg_output_index, false);

				//dtype
				ImGui::Text("dtype");
				ImGui::SameLine(COL_LEFT_WIDTH);
				AddHelpMarker(u8"输出可选数组深度，默认为 -1");
				ImGui::SameLine();
				static int dtype = -1;
				ImGui::SliderInt("##dtype", &dtype, -1, 4);

				//执行 subtract()
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) &&
					(arg_input1_index != -1 && arg_input2_index != -1))
				{
					MatViewer *input1_viewer = MatViewerManager::Instance().GetViewer(arg_input1_index);
					MatViewer *input2_viewer = MatViewerManager::Instance().GetViewer(arg_input2_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input2_viewer->GetNextTitle("subtract"));
					InputArray mask = arg_mask_index == -1 || arg_mask_index == items_count ? noArray() : MatViewerManager::Instance().GetViewer(arg_mask_index)->GetMat();

					try
					{
						subtract(input1_viewer->GetMat(), input2_viewer->GetMat(), output_viewer->GetMat(), mask, dtype);

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input2_viewer->GetNextViewerPos());

						AddLogger(LogType::Info, "subtract() succeeded: %s\n", output_viewer->GetTitle());

					}
					catch (Exception e)
					{
						AddLogger(e, "subtract() error");
					}

					mask.~_InputArray();
					input1_viewer = NULL;
					input2_viewer = NULL;
					output_viewer = NULL;
				}
				ImGui::TreePop();
			}

			//multiply()
			if (ImGui::TreeNode(u8"multiply()"))
			{
				//input1
				static int arg_input1_index = -1;
				AddViewerCombo("input1", &arg_input1_index, true);

				//input2
				static int arg_input2_index = -1;
				AddViewerCombo("input2", &arg_input2_index, true);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//scale
				ImGui::Text("scale");
				ImGui::SameLine(COL_LEFT_WIDTH); 
				AddHelpMarker(u8"optional scale factor, default 1.0");
				ImGui::SameLine();
				static float scale = 1.0;
				ImGui::SliderFloat("##scale", &scale, 0.1f, 3.0f, "%.1f");

				//dtype
				//输出数组的可选深度
				ImGui::Text("dtype");
				ImGui::SameLine(COL_LEFT_WIDTH);
				AddHelpMarker(u8"输出可选数组深度，默认为 -1");
				ImGui::SameLine();
				static int dtype = -1;
				ImGui::SliderInt("##dtype", &dtype, -1, 4);

				//执行 multiply()
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) &&
					(arg_input1_index != -1 && arg_input2_index != -1))
				{
					MatViewer *input1_viewer = MatViewerManager::Instance().GetViewer(arg_input1_index);
					MatViewer *input2_viewer = MatViewerManager::Instance().GetViewer(arg_input2_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input2_viewer->GetNextTitle("multiply"));

					try
					{
						multiply(input1_viewer->GetMat(), input2_viewer->GetMat(), output_viewer->GetMat(), scale, dtype);

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input2_viewer->GetNextViewerPos());

						AddLogger(LogType::Info, "multiply() succeeded: %s\n", output_viewer->GetTitle());
					}
					catch (Exception e)
					{
						AddLogger(e, "multiply() error");
					}

					input1_viewer = NULL;
					input2_viewer = NULL;
					output_viewer = NULL;
				}
				ImGui::TreePop();
			}

			//divide()
			if (ImGui::TreeNode(u8"divide()"))
			{
				//input1
				static int arg_input1_index = -1;
				AddViewerCombo("input1", &arg_input1_index, true);

				//input2
				static int arg_input2_index = -1;
				AddViewerCombo("input2", &arg_input2_index, true);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//scale
				ImGui::Text("scale");
				ImGui::SameLine(COL_LEFT_WIDTH);
				AddHelpMarker(u8"optional scale factor, default 1.0");
				ImGui::SameLine();
				static float scale = 1.0;
				ImGui::SliderFloat("##scale", &scale, 0.1f, 3.0f, "%.1f");

				//dtype
				ImGui::Text("dtype");
				ImGui::SameLine(COL_LEFT_WIDTH);
				AddHelpMarker(u8"输出可选数组深度，默认为 -1");
				ImGui::SameLine();
				static int dtype = -1;
				ImGui::SliderInt("##dtype", &dtype, -1, 4);

				//执行 divide()
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) &&
					(arg_input1_index != -1 && arg_input2_index != -1))
				{
					MatViewer *input1_viewer = MatViewerManager::Instance().GetViewer(arg_input1_index);
					MatViewer *input2_viewer = MatViewerManager::Instance().GetViewer(arg_input2_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input2_viewer->GetNextTitle("divide"));

					try
					{
						divide(input1_viewer->GetMat(), input2_viewer->GetMat(), output_viewer->GetMat(), scale, dtype);

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input2_viewer->GetNextViewerPos());

						AddLogger(LogType::Info, "divide() succeeded: %s\n", output_viewer->GetTitle());
					}
					catch (Exception e)
					{
						AddLogger(e, "divide() error");
					}

					input1_viewer = NULL;
					input2_viewer = NULL;
					output_viewer = NULL;
				}
				ImGui::TreePop();
			}

			//add, subtract, divide, scaleAdd, addWeighted, accumulate, accumulateProduct, accumulateSquare,

			//bitwise_and()
			if (ImGui::TreeNode(u8"bitwise_and()"))
			{
				//input1
				static int arg_input1_index = -1;
				AddViewerCombo("input1", &arg_input1_index, true);

				//input2
				static int arg_input2_index = -1;
				AddViewerCombo("input2", &arg_input2_index, true);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//mask
				static int arg_mask_index = -1;
				AddViewerCombo("mask", &arg_mask_index, false);

				//执行 divide()
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) &&
					(arg_input1_index != -1 && arg_input2_index != -1))
				{
					MatViewer *input1_viewer = MatViewerManager::Instance().GetViewer(arg_input1_index);
					MatViewer *input2_viewer = MatViewerManager::Instance().GetViewer(arg_input2_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input2_viewer->GetNextTitle("bitwise_and"));

					InputArray mask = arg_mask_index == -1 || arg_mask_index == items_count ? noArray() : MatViewerManager::Instance().GetViewer(arg_mask_index)->GetMat();

					try
					{
						bitwise_and(input1_viewer->GetMat(), input2_viewer->GetMat(), output_viewer->GetMat(), mask);

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input2_viewer->GetNextViewerPos());

						AddLogger(LogType::Info, "bitwise_and() succeeded: %s\n", output_viewer->GetTitle());
					}
					catch (Exception e)
					{
						AddLogger(e, "bitwise_and() error");
					}

					mask.~_InputArray();
					input1_viewer = NULL;
					input2_viewer = NULL;
					output_viewer = NULL;
				}
				ImGui::TreePop();
			}

			//bitwise_or()

			//bitwise_xor()

			//bitwise_not()
		}

		//二值图像分析
		if (ImGui::CollapsingHeader(u8"二值图像分析"))
		{

		}

		//图像形态学
		if (ImGui::CollapsingHeader(u8"图像形态学"))
		{

		}
		
	}

	//end
	ImGui::End();
}