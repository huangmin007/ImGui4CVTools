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
static void AddLogger(LogType type, const char *log, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, log);
	vsnprintf(buf, IM_ARRAYSIZE(buf), log, args);
	buf[IM_ARRAYSIZE(buf) - 1] = 0;
	va_end(args);

	SYSTEMTIME sys;
	GetLocalTime(&sys);

	static const char *s_type[3] = { "info", "warn", "error" };
	logger.AddLog("[%02d:%02d:%02d.%03d][%s]  %s", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, s_type[type], buf);
}

static void AddCVException(const char *msg, Exception e)
{
	AddLogger(LogType::Error, "%s: \n\t\t\t\t%s\n", msg, e.what());
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
static void AddViewerCombo(const char* label, int *out_index, bool input = true)
{
	const int item_count = MatViewerManager::Instance().GetCount();
	const char **items = MatViewerManager::Instance().GetAllTitle();
	const char **n_itmes = MatViewerManager::Instance().GetAllTitle("Create New Viewer");

	ImGui::Text(label);
	ImGui::SameLine(COL_LEFT_WIDTH);

	if (input)
	{
		AddHelpMarker(u8"选择视图窗口的 InputArray 对象");
		ImGui::SameLine();
		ImGui::Combo("##src", out_index, items, item_count);
	}
	else
	{
		AddHelpMarker(u8"选择视图窗口的 InputArray 对象\n不选择或选择 Create New Viewer 表示创建新的 InputArray 对象");
		ImGui::SameLine();
		ImGui::Combo("##dst", out_index, n_itmes, item_count + 1);
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
		static Mat result;
		const int item_count = MatViewerManager::Instance().GetCount();
		const char **items = MatViewerManager::Instance().GetAllTitle();
		const char **n_items = MatViewerManager::Instance().GetAllTitle("Create New Viewer");

		//API imread
		if (ImGui::Button("imread", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)))
		{
			char m_fileName[MAX_PATH];
			if (SelectImageFile(window, m_fileName))
			{
				//current time			
				char c_time[32];
				GetCurretnTime(c_time, 32, "[%X]");

				//file name
				//const char *fn = strrchr(m_fileName, '\\') + 1;
				//printf("file:%s\n", fn);

				//api title
				char title[MAX_CHAR];
				sprintf_s(title, MAX_CHAR, "%s || %s", c_time, "imread");
				AddLogger(LogType::Info, "select file: %s\n", m_fileName);

				static int flags = ImreadModes::IMREAD_COLOR;
				try
				{
					result = imread(m_fileName, flags);
				}
				catch (Exception e)
				{
					AddCVException("imread() error", e);
				}

				MatViewerManager::Instance().AddViewer(new MatViewer(title, result));
				AddLogger(LogType::Info, "imread() create viewer: %s\n", title);
			}
		}

		//Base API
		if (ImGui::CollapsingHeader("Base"))
		{
			//cvtColor()
			if (ImGui::TreeNode(u8"色彩空间转换 cvtColor()"))
			{
				static EnumParser<ColorConversionCodes> CVAPI_ColorConversionCodes;

				//input
				//ImGui::Text("input");
				//ImGui::SameLine(COL_LEFT_WIDTH);  AddHelpMarker(u8"选择视图窗口的 InputArray 对象");
				//ImGui::SameLine();
				static int arg_input_index = -1;
				//ImGui::Combo("##src", &arg_input_index, items, item_count);
				AddViewerCombo("input", &arg_input_index);

				//output
				//ImGui::Text("output");
				//ImGui::SameLine(COL_LEFT_WIDTH);  
				//AddHelpMarker(u8"选择视图窗口的 InputArray 对象\n不选择或选择 Create New Viewer 表示创建新的 InputArray 对象");
				//ImGui::SameLine();
				static int arg_output_index = -1;
				//ImGui::Combo("##dst", &arg_output_index, n_items, item_count + 1);

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

				/*
				//是否创建新视图对象
				static bool n_viewer = false;
				ImGui::Checkbox("Create New Viewer", &n_viewer);
				ImGui::SameLine();
				AddHelpMarker(u8"是否在新的视图中显示输出结果");
				*/

				//执行 cvtColor()
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) && arg_input_index != -1)
				{
					MatViewer *viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					try
					{
						if (arg_output_index == -1 || arg_output_index == item_count)
						{
							cvtColor(viewer->GetMat(), result, CVAPI_ColorConversionCodes.ParseIndex2Enum(arg_code_index));

							MatViewer *n_viewer = MatViewerManager::Instance().GetViewer(viewer->GetNextTitle("cvtColor"));
							n_viewer->SetViewerPos(viewer->GetNextViewerPos());
							n_viewer->is_open = true;
							n_viewer->LoadMat(result);

							AddLogger(LogType::Info, "cvColor() create viewer: %s\n", n_viewer->GetTitle());
						}
						else
						{
							Mat image = viewer->GetMat();
							cvtColor(image, image, CVAPI_ColorConversionCodes.ParseIndex2Enum(arg_code_index));
							viewer->is_open = true;
							viewer->LoadMat(image);
							AddLogger(LogType::Info, "cvColor() %s\n", viewer->GetTitle());
						}
					}
					catch (Exception e)
					{
						AddCVException("cvtColor() error", e);
					}
				}
				ImGui::TreePop();
			}

			//flip()
			if (ImGui::TreeNode(u8"图像翻转 flip()"))
			{
				//input
				//ImGui::Text("src");
				//ImGui::SameLine(COL_LEFT_WIDTH);  AddHelpMarker(u8"选择视图窗口的 InputArray 对象");
				//ImGui::SameLine();
				static int arg_input_index = -1;
				//ImGui::Combo("##src", &arg_input_index, items, item_count);
				AddViewerCombo("input", &arg_input_index);

				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);


				//flipCode
				ImGui::Text("flipCode");
				ImGui::SameLine(COL_LEFT_WIDTH); AddHelpMarker(u8"0表示绕x轴翻转，1表示绕y轴翻转，-1表示在两个轴周围翻转");
				ImGui::SameLine();
				static int arg_flipCode_index = 0;
				ImGui::SliderInt("##dstCn", &arg_flipCode_index, -1, 1);


				//是否创建新视图对象
				static bool n_viewer = false;
				ImGui::Checkbox("Create New Viewer", &n_viewer);
				ImGui::SameLine();
				AddHelpMarker(u8"是否在新的视图中显示输出结果");

				//执行 flip()
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) && arg_input_index != -1)
				{
					printf("len:%d len:%d\n", sizeof(*items));
					MatViewer *viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					try
					{
						if (arg_output_index == -1 || arg_output_index == item_count)
						{
							flip(viewer->GetMat(), result, arg_flipCode_index);

							MatViewer *n_viewer = MatViewerManager::Instance().GetViewer(viewer->GetNextTitle("flip"));
							n_viewer->SetViewerPos(viewer->GetNextViewerPos());
							n_viewer->is_open = true;
							n_viewer->LoadMat(result);
							AddLogger(LogType::Info, "flip() create viewer: %s\n", n_viewer->GetTitle());
						}
						else
						{
							Mat image = viewer->GetMat();
							flip(image, image, arg_flipCode_index);
							viewer->is_open = true;
							viewer->LoadMat(image);
							AddLogger(LogType::Info, "flip() %s\n", viewer->GetTitle());
						}
					}
					catch (Exception e)
					{
						AddCVException("flip() error", e);
					}
				}
				ImGui::TreePop();
			}

			//rotate()
		}

		//算术操作 API
		if (ImGui::CollapsingHeader(u8"图像运算操作"))
		{
			//add()
			if (ImGui::TreeNode(u8"add()"))
			{
				//input1
				ImGui::Text("src1");
				ImGui::SameLine(COL_LEFT_WIDTH);  AddHelpMarker(u8"选择视图窗口的 InputArray 对象");
				ImGui::SameLine();
				static int arg_input1_index = -1;
				ImGui::Combo("##src1", &arg_input1_index, items, item_count);

				//input2
				ImGui::Text("src2");
				ImGui::SameLine(COL_LEFT_WIDTH);  AddHelpMarker(u8"选择视图窗口的 InputArray 对象");
				ImGui::SameLine();
				static int arg_input2_index = -1;
				ImGui::Combo("##src2", &arg_input2_index, items, item_count);

				//是否创建新视图对象
				static bool n_viewer = false;
				ImGui::Checkbox("Create New Viewer", &n_viewer);
				ImGui::SameLine();
				AddHelpMarker(u8"是否在新的视图中显示输出结果\n如果不在新视图中显示，将会在第二个输入视图中显示结果");

				//执行 add()
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) &&
					(arg_input1_index != -1 || arg_input2_index == -1))
				{
					MatViewer *viewer1 = MatViewerManager::Instance().GetViewer(arg_input1_index);
					MatViewer *viewer2 = MatViewerManager::Instance().GetViewer(arg_input2_index);

					try
					{
						if (n_viewer)
						{
							add(viewer1->GetMat(), viewer2->GetMat(), result);

							MatViewer *n_viewer = MatViewerManager::Instance().GetViewer(viewer2->GetNextTitle("add"));
							n_viewer->SetViewerPos(viewer2->GetNextViewerPos());
							n_viewer->is_open = true;
							n_viewer->LoadMat(result);
							AddLogger(LogType::Info, "add() create viewer: %s\n", n_viewer->GetTitle());
						}
						else
						{
							add(viewer1->GetMat(), viewer2->GetMat(), viewer2->GetMat());
							viewer2->is_open = true;
							viewer2->UpdateMat();
							AddLogger(LogType::Info, "add() %s (+) %s\n", viewer1->GetTitle(), viewer2->GetTitle());
						}
					}
					catch (Exception e)
					{
						AddCVException("add() error", e);
					}
				}
				ImGui::TreePop();
			}

			//subtract()
			if (ImGui::TreeNode(u8"subtract()"))
			{
				//input1
				ImGui::Text("src1");
				ImGui::SameLine(COL_LEFT_WIDTH);  AddHelpMarker(u8"选择视图窗口的 InputArray 对象");
				ImGui::SameLine();
				static int arg_input1_index = -1;
				ImGui::Combo("##src1", &arg_input1_index, items, item_count);

				//input2
				ImGui::Text("src2");
				ImGui::SameLine(COL_LEFT_WIDTH);  AddHelpMarker(u8"选择视图窗口的 InputArray 对象");
				ImGui::SameLine();
				static int arg_input2_index = -1;
				ImGui::Combo("##src2", &arg_input2_index, items, item_count);

				//是否创建新视图对象
				static bool n_viewer = false;
				ImGui::Checkbox("Create New Viewer", &n_viewer);
				ImGui::SameLine();
				AddHelpMarker(u8"是否在新的视图中显示输出结果\n如果不在新视图中显示，将会在第二个输入视图中显示结果");

				//执行 subtract()
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) &&
					(arg_input1_index != -1 || arg_input2_index == -1))
				{
					MatViewer *viewer1 = MatViewerManager::Instance().GetViewer(arg_input1_index);
					MatViewer *viewer2 = MatViewerManager::Instance().GetViewer(arg_input2_index);

					try
					{
						if (n_viewer)
						{
							subtract(viewer1->GetMat(), viewer2->GetMat(), result);

							MatViewer *n_viewer = MatViewerManager::Instance().GetViewer(viewer2->GetNextTitle("subtract"));
							n_viewer->SetViewerPos(viewer2->GetNextViewerPos());
							n_viewer->is_open = true;
							n_viewer->LoadMat(result);
							AddLogger(LogType::Info, "subtract() create viewer: %s\n", n_viewer->GetTitle());
						}
						else
						{
							subtract(viewer1->GetMat(), viewer2->GetMat(), viewer2->GetMat());
							viewer2->is_open = true;
							viewer2->UpdateMat();
							AddLogger(LogType::Info, "subtract() %s (-) %s\n", viewer1->GetTitle(), viewer2->GetTitle());
						}
					}
					catch (Exception e)
					{
						AddCVException("subtract() error", e);
					}
				}
				ImGui::TreePop();
			}

			//multiply()
			if (ImGui::TreeNode(u8"multiply()"))
			{
				//input1
				ImGui::Text("src1");
				ImGui::SameLine(COL_LEFT_WIDTH);  AddHelpMarker(u8"选择视图窗口的 InputArray 对象");
				ImGui::SameLine();
				static int arg_input1_index = -1;
				ImGui::Combo("##src1", &arg_input1_index, items, item_count);

				//input2
				ImGui::Text("src2");
				ImGui::SameLine(COL_LEFT_WIDTH);  AddHelpMarker(u8"选择视图窗口的 InputArray 对象");
				ImGui::SameLine();
				static int arg_input2_index = -1;
				ImGui::Combo("##src2", &arg_input2_index, items, item_count);

				//scale
				ImGui::Text("scale");
				ImGui::SameLine(COL_LEFT_WIDTH); 
				AddHelpMarker(u8"optional scale factor, default 1.0");
				ImGui::SameLine();
				static float scale = 1.0;
				ImGui::SliderFloat("##scale", &scale, 0.1, 3.0, "%.1f");

				//dtype
				//输出数组的可选深度
				ImGui::Text("dtype");
				ImGui::SameLine(COL_LEFT_WIDTH);
				AddHelpMarker(u8"输出可选数组深度，默认为 -1");
				ImGui::SameLine();
				static int dtype = -1;
				ImGui::SliderInt("##dtype", &dtype, -1, 4);

				//是否创建新视图对象
				static bool n_viewer = false;
				ImGui::Checkbox("Create New Viewer", &n_viewer);
				ImGui::SameLine();
				AddHelpMarker(u8"是否在新的视图中显示输出结果\n如果不在新视图中显示，将会在第二个输入视图中显示结果");

				//执行 multiply()
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) &&
					(arg_input1_index != -1 || arg_input2_index == -1))
				{
					MatViewer *viewer1 = MatViewerManager::Instance().GetViewer(arg_input1_index);
					MatViewer *viewer2 = MatViewerManager::Instance().GetViewer(arg_input2_index);

					try
					{
						if (n_viewer)
						{
							multiply(viewer1->GetMat(), viewer2->GetMat(), result, scale, dtype);

							MatViewer *n_viewer = MatViewerManager::Instance().GetViewer(viewer2->GetNextTitle("add"));
							n_viewer->SetViewerPos(viewer2->GetNextViewerPos());
							n_viewer->is_open = true;
							n_viewer->LoadMat(result);
							AddLogger(LogType::Info, "multiply() create viewer: %s\n", n_viewer->GetTitle());
						}
						else
						{
							multiply(viewer1->GetMat(), viewer2->GetMat(), viewer2->GetMat(), scale, dtype);
							viewer2->is_open = true;
							viewer2->UpdateMat();
							AddLogger(LogType::Info, "multiply() %s (*) %s\n", viewer1->GetTitle(), viewer2->GetTitle());
						}
					}
					catch (Exception e)
					{
						AddCVException("multiply() error", e);
					}
				}
				ImGui::TreePop();
			}

			//divide()
			if (ImGui::TreeNode(u8"divide()"))
			{
				//input1
				ImGui::Text("src1");
				ImGui::SameLine(COL_LEFT_WIDTH);  AddHelpMarker(u8"选择视图窗口的 InputArray 对象");
				ImGui::SameLine();
				static int arg_input1_index = -1;
				ImGui::Combo("##src1", &arg_input1_index, items, item_count);

				//input2
				ImGui::Text("src2");
				ImGui::SameLine(COL_LEFT_WIDTH);  AddHelpMarker(u8"选择视图窗口的 InputArray 对象");
				ImGui::SameLine();
				static int arg_input2_index = -1;
				ImGui::Combo("##src2", &arg_input2_index, items, item_count);

				//scale
				ImGui::Text("scale");
				ImGui::SameLine(COL_LEFT_WIDTH);
				AddHelpMarker(u8"optional scale factor, default 1.0");
				ImGui::SameLine();
				static float scale = 1.0;
				ImGui::SliderFloat("##scale", &scale, 0.1, 3.0, "%.1f");

				//dtype
				//输出数组的可选深度
				ImGui::Text("dtype");
				ImGui::SameLine(COL_LEFT_WIDTH);
				AddHelpMarker(u8"输出可选数组深度，默认为 -1");
				ImGui::SameLine();
				static int dtype = -1;
				ImGui::SliderInt("##dtype", &dtype, -1, 4);

				//是否创建新视图对象
				static bool n_viewer = false;
				ImGui::Checkbox("Create New Viewer", &n_viewer);
				ImGui::SameLine();
				AddHelpMarker(u8"是否在新的视图中显示输出结果\n如果不在新视图中显示，将会在第二个输入视图中显示结果");

				//执行 divide()
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) &&
					(arg_input1_index != -1 || arg_input2_index == -1))
				{
					MatViewer *viewer1 = MatViewerManager::Instance().GetViewer(arg_input1_index);
					MatViewer *viewer2 = MatViewerManager::Instance().GetViewer(arg_input2_index);

					try
					{
						if (n_viewer)
						{
							divide(viewer1->GetMat(), viewer2->GetMat(), result, scale, dtype);

							MatViewer *n_viewer = MatViewerManager::Instance().GetViewer(viewer2->GetNextTitle("add"));
							n_viewer->SetViewerPos(viewer2->GetNextViewerPos());
							n_viewer->is_open = true;
							n_viewer->LoadMat(result);
							AddLogger(LogType::Info, "divide() create viewer: %s\n", n_viewer->GetTitle());
						}
						else
						{
							divide(viewer1->GetMat(), viewer2->GetMat(), viewer2->GetMat(), scale, dtype);
							viewer2->is_open = true;
							viewer2->UpdateMat();
							AddLogger(LogType::Info, "divide() %s (*) %s\n", viewer1->GetTitle(), viewer2->GetTitle());
						}
					}
					catch (Exception e)
					{
						AddCVException("divide() error", e);
					}
				}
				ImGui::TreePop();
			}

			//add, subtract, divide, scaleAdd, addWeighted, accumulate, accumulateProduct, accumulateSquare,

			//bitwise_and()

			//bitwise_or()

			//bitwise_xor()

			//bitwise_not()
		}

		result.release();
	}
	ImGui::End();
}