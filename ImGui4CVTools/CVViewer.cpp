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
	const char **n_itmes = MatViewerManager::Instance().GetAllTitle("Create New Viewer OR Use Default Value");

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
		AddHelpMarker(u8"选择出视图窗口的 InputArray 对象\n选择 [Create New Viewer ...] 或不选择，表示创建新的 InputArray 对象或是参数为空或是默认值");
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
		static Mat output_result;
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

				static int flags = ImreadModes::IMREAD_COLOR;
				try
				{
					output_result = imread(m_fileName, flags);
				}
				catch (Exception e)
				{
					AddLogger(e, "imread() error");
				}

				MatViewerManager::Instance().AddViewer(new MatViewer(title, output_result));
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
					MatViewer *viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, viewer->GetNextTitle("cvtColor"));

					try
					{
						cvtColor(viewer->GetMat(), output_viewer->GetMat(), CVAPI_ColorConversionCodes.ParseIndex2Enum(arg_code_index));
						output_viewer->is_open = true;
						output_viewer->UpdateMat();
						output_viewer->SetViewerPos(viewer->GetNextViewerPos());
					}
					catch (Exception e)
					{
						AddLogger(LogType::Info, "cvColor() %s\n", viewer->GetTitle());
					}

					try
					{


						if (arg_output_index == -1 || arg_output_index == items_count)
						{
							cvtColor(viewer->GetMat(), output_result, CVAPI_ColorConversionCodes.ParseIndex2Enum(arg_code_index));

							MatViewer *n_viewer = MatViewerManager::Instance().GetViewer(viewer->GetNextTitle("cvtColor"));
							n_viewer->SetViewerPos(viewer->GetNextViewerPos());
							n_viewer->is_open = true;
							n_viewer->LoadMat(output_result);

							AddLogger(LogType::Info, "cvColor() create viewer: %s\n", n_viewer->GetTitle());
						}
						else
						{
							MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index);

							cvtColor(viewer->GetMat(), output_viewer->GetMat(), CVAPI_ColorConversionCodes.ParseIndex2Enum(arg_code_index));
							viewer->is_open = true;
							viewer->UpdateMat();
							AddLogger(LogType::Info, "cvColor() %s\n", viewer->GetTitle());
						}
					}
					catch (Exception e)
					{
						AddLogger(e, "cvtColor() error");
					}
					viewer = NULL;
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
					MatViewer *viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					try
					{
						if (arg_output_index == -1 || arg_output_index == items_count)
						{
							flip(viewer->GetMat(), output_result, arg_flipCode_index);

							MatViewer *n_viewer = MatViewerManager::Instance().GetViewer(viewer->GetNextTitle("flip"));
							n_viewer->SetViewerPos(viewer->GetNextViewerPos());
							n_viewer->is_open = true;
							n_viewer->LoadMat(output_result);
							AddLogger(LogType::Info, "flip() create viewer: %s\n", n_viewer->GetTitle());
						}
						else
						{
							MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index);

							flip(viewer->GetMat(), output_viewer->GetMat(), arg_flipCode_index);
							output_viewer->is_open = true;
							output_viewer->UpdateMat();
							AddLogger(LogType::Info, "flip() %s\n", viewer->GetTitle());
						}
					}
					catch (Exception e)
					{
						AddLogger(e, "flip() error");
					}
					viewer = NULL;
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
					MatViewer *viewer1 = MatViewerManager::Instance().GetViewer(arg_input1_index);
					MatViewer *viewer2 = MatViewerManager::Instance().GetViewer(arg_input2_index);
					InputArray mask = arg_mask_index == -1 || arg_mask_index == items_count ? noArray() : MatViewerManager::Instance().GetViewer(arg_mask_index)->GetMat();

					try
					{
						if (arg_output_index ==  -1 || arg_output_index == items_count)
						{
							add(viewer1->GetMat(), viewer2->GetMat(), output_result, mask, dtype);

							MatViewer *n_viewer = MatViewerManager::Instance().GetViewer(viewer2->GetNextTitle("add"));
							n_viewer->SetViewerPos(viewer2->GetNextViewerPos());
							n_viewer->is_open = true;
							n_viewer->LoadMat(output_result);
							AddLogger(LogType::Info, "add() create viewer: %s\n", n_viewer->GetTitle());
						}
						else
						{
							MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index);

							add(viewer1->GetMat(), viewer2->GetMat(), output_viewer->GetMat(), mask, dtype);
							output_viewer->is_open = true;
							output_viewer->UpdateMat();
							AddLogger(LogType::Info, "add() %s (+) %s\n", viewer1->GetTitle(), viewer2->GetTitle());

							output_viewer = NULL;
						}
					}
					catch (Exception e)
					{
						AddLogger(e, "add() error");
					}

					viewer1 = NULL;
					viewer2 = NULL;
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
					MatViewer *viewer1 = MatViewerManager::Instance().GetViewer(arg_input1_index);
					MatViewer *viewer2 = MatViewerManager::Instance().GetViewer(arg_input2_index);
					InputArray mask = arg_mask_index == -1 || arg_mask_index == items_count ? noArray() : MatViewerManager::Instance().GetViewer(arg_mask_index)->GetMat();

					try
					{
						if (arg_output_index == -1 || arg_output_index == items_count)
						{
							subtract(viewer1->GetMat(), viewer2->GetMat(), output_result, mask, dtype);

							MatViewer *n_viewer = MatViewerManager::Instance().GetViewer(viewer2->GetNextTitle("subtract"));
							n_viewer->SetViewerPos(viewer2->GetNextViewerPos());
							n_viewer->is_open = true;
							n_viewer->LoadMat(output_result);
							AddLogger(LogType::Info, "subtract() create viewer: %s\n", n_viewer->GetTitle());
						}
						else
						{
							MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index);

							subtract(viewer1->GetMat(), viewer2->GetMat(), output_viewer->GetMat(), mask, dtype);
							output_viewer->is_open = true;
							output_viewer->UpdateMat();
							AddLogger(LogType::Info, "subtract() %s (-) %s\n", viewer1->GetTitle(), viewer2->GetTitle());

							output_viewer = NULL;
						}
					}
					catch (Exception e)
					{
						AddLogger(e, "subtract() error");
					}

					viewer1 = NULL;
					viewer2 = NULL;
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
				ImGui::SliderFloat("##scale", &scale, 0.1, 3.0, "%.1f");

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
					MatViewer *viewer1 = MatViewerManager::Instance().GetViewer(arg_input1_index);
					MatViewer *viewer2 = MatViewerManager::Instance().GetViewer(arg_input2_index);

					try
					{
						if (arg_output_index == -1 || arg_output_index == items_count)
						{
							multiply(viewer1->GetMat(), viewer2->GetMat(), output_result, scale, dtype);

							MatViewer *n_viewer = MatViewerManager::Instance().GetViewer(viewer2->GetNextTitle("multiply"));
							n_viewer->SetViewerPos(viewer2->GetNextViewerPos());
							n_viewer->is_open = true;
							n_viewer->LoadMat(output_result);
							AddLogger(LogType::Info, "multiply() create viewer: %s\n", n_viewer->GetTitle());
						}
						else
						{
							MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index);

							multiply(viewer1->GetMat(), viewer2->GetMat(), output_viewer->GetMat(), scale, dtype);
							output_viewer->is_open = true;
							output_viewer->UpdateMat();
							AddLogger(LogType::Info, "multiply() %s (*) %s\n", viewer1->GetTitle(), viewer2->GetTitle());

							output_viewer = NULL;
						}
					}
					catch (Exception e)
					{
						AddLogger(e, "multiply() error");
					}

					viewer1 = NULL;
					viewer2 = NULL;
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
				ImGui::SliderFloat("##scale", &scale, 0.1, 3.0, "%.1f");

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
					MatViewer *viewer1 = MatViewerManager::Instance().GetViewer(arg_input1_index);
					MatViewer *viewer2 = MatViewerManager::Instance().GetViewer(arg_input2_index);

					try
					{
						if (arg_output_index == -1 || arg_output_index == items_count)
						{
							divide(viewer1->GetMat(), viewer2->GetMat(), output_result, scale, dtype);

							MatViewer *n_viewer = MatViewerManager::Instance().GetViewer(viewer2->GetNextTitle("multiply"));
							n_viewer->SetViewerPos(viewer2->GetNextViewerPos());
							n_viewer->is_open = true;
							n_viewer->LoadMat(output_result);
							AddLogger(LogType::Info, "divide() create viewer: %s\n", n_viewer->GetTitle());
						}
						else
						{
							MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index);

							divide(viewer1->GetMat(), viewer2->GetMat(), output_viewer->GetMat(), scale, dtype);
							output_viewer->is_open = true;
							output_viewer->UpdateMat();
							AddLogger(LogType::Info, "divide() %s (/) %s\n", viewer1->GetTitle(), viewer2->GetTitle());

							output_viewer = NULL;
						}
					}
					catch (Exception e)
					{
						AddLogger(e, "divide() error");
					}

					viewer1 = NULL;
					viewer2 = NULL;
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
					MatViewer *viewer1 = MatViewerManager::Instance().GetViewer(arg_input1_index);
					MatViewer *viewer2 = MatViewerManager::Instance().GetViewer(arg_input2_index);
					InputArray mask = arg_mask_index == -1 || arg_mask_index == items_count ? noArray() : MatViewerManager::Instance().GetViewer(arg_mask_index)->GetMat();

					try
					{
						if (arg_output_index == -1 || arg_output_index == items_count)
						{
							bitwise_and(viewer1->GetMat(), viewer2->GetMat(), output_result, mask);

							MatViewer *n_viewer = MatViewerManager::Instance().GetViewer(viewer2->GetNextTitle("bitwise_and"));
							n_viewer->SetViewerPos(viewer2->GetNextViewerPos());
							n_viewer->is_open = true;
							n_viewer->LoadMat(output_result);
							AddLogger(LogType::Info, "bitwise_and() create viewer: %s\n", n_viewer->GetTitle());
						}
						else
						{
							MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index);

							bitwise_and(viewer1->GetMat(), viewer2->GetMat(), output_viewer->GetMat(), mask);
							output_viewer->is_open = true;
							output_viewer->UpdateMat();
							AddLogger(LogType::Info, "bitwise_and() %s (&) %s\n", viewer1->GetTitle(), viewer2->GetTitle());

							output_viewer = NULL;
						}
					}
					catch (Exception e)
					{
						AddLogger(e, "bitwise_and() error");
					}

					mask.~_InputArray();
					viewer1 = NULL;
					viewer2 = NULL;
				}
				ImGui::TreePop();
			}

			//bitwise_or()

			//bitwise_xor()

			//bitwise_not()
		}


		//release output
		output_result.release();
	}

	//end
	ImGui::End();
}