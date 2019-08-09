#include "stdafx.h"
#include "CVViewer.h"

#include <fstream>
#include <sstream>
#include <string>
#include <time.h>

#include "cvImGuiConfig.h"
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
	
	cv_dtype_it = cv_dtype_map.begin();
	for (int i = 0; cv_dtype_it != cv_dtype_map.end(); cv_dtype_it++, i++)
		cv_dtype_char[i] = cv_dtype_it->first;

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

		ShowMainMenuBar();

		ShowCVAPIWindow(window);
		if (m_showLogWindow)	ShowLoggerWindow(&m_showLogWindow);
		if (m_showDemoWindow)	ImGui::ShowDemoWindow(&m_showDemoWindow);
		if (m_showCVAPIHelp)	ShowCVAPIHelpWindow(&m_showCVAPIHelp);

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

//显示主菜单栏
void CVViewer::ShowMainMenuBar()
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
				m_showLogWindow = !m_showLogWindow;
			}
			if (ImGui::MenuItem("OpenCV API Help"))
				m_showCVAPIHelp = !m_showCVAPIHelp;
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}


//添加 Marker 标记
static void AddMarker(const char* desc)
{
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

//添加 (?) help 标记
static void AddHelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	AddMarker(desc);
}

//添加左侧标签
static void AddLeftLabel(const char *label, const char *helpDes)
{
	ImGui::Text(label);
	ImGui::SameLine(COL_LEFT_WIDTH);

	if (helpDes != NULL)
		AddHelpMarker(helpDes);

	ImGui::SameLine();
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

//显示日志窗口
static void ShowLoggerWindow(bool* p_open)
{
	ImGui::SetNextWindowPos(ImVec2(400, 25), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

	//ImGui::Begin("Logger", p_open);	
	//ImGui::End();

	Logger.Draw("Logger", p_open);
}
//CV API 行信息
static void AddTableRow(const char *label1, const char *label2, const char *label3)
{
	ImGui::Text(label1);
	ImGui::SameLine(50);
	ImGui::Text(label2);
	ImGui::SameLine(200);
	ImGui::Text(label3);
}
//显示 OpenCV API Help 窗口
static void ShowCVAPIHelpWindow(bool *p_open)
{
	ImGui::SetNextWindowPos(ImVec2(500, 300), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(400, 380), ImGuiCond_FirstUseEver);
	
	ImGui::Begin("OpenCV(v410) API Help", p_open);

	ImGui::Text(u8"API参数说明：('_'之后表示可使用默认值)");

	AddTableRow("i", "InputArray", u8"表示为输入数据类型");
	AddTableRow("o", "OutputArray", u8"表示为输出数据类型");
	AddTableRow("m", "InputOutputArray", u8"表示为输入/出数据类型");
	AddTableRow("d", "int,uint ...", u8"整型数字");
	AddTableRow("f", "float,double ...", u8"浮点点数字");
	AddTableRow("p", "Point", u8"");
	AddTableRow("r", "Rect", u8"");
	AddTableRow("s", "Scalar", u8"Scalar,Size");
	AddTableRow("v", "ImVec(n)", u8"");
	AddTableRow("t", "Text", u8"");
	AddTableRow("b", "Boolean", u8"");

	ImGui::Text(u8"注意：OpenCV API 浮点类型使用的都是 double 类型，\n\t\tImGui 浮点类型使用的都是 float 类型，会有一点的精度损失。");

	ImGui::End();
}

//添加 Viewer List Guide Combo 
static void AddViewerCombo(const char *label, int *out_index, bool input = true, const char *des = NULL, ...)
{
	const int items_count = MatViewerManager::Instance().GetCount();
	const char **items = MatViewerManager::Instance().GetAllTitle();
	const char **n_itmes = MatViewerManager::Instance().GetAllTitle("Create New Viewer");

	ImGui::Text(label);
	ImGui::SameLine(COL_LEFT_WIDTH);

	static char c_des[255] = {};
	static char c_label[255] = {};
	sprintf_s(c_label, "%s%s", "##", label);

	if (input)
	{
		sprintf_s(c_des, "%s\n%s", u8"选择输入视图窗口的 InputArray 对象", des != NULL ? des : "");

		AddHelpMarker(c_des);
		ImGui::SameLine();
		ImGui::Combo(c_label, out_index, items, items_count);
	}
	else
	{
		sprintf_s(c_des, "%s\n%s", u8"选择出视图窗口的 InputArray 对象\n选择 [Create New Viewer] 或不选择，表示创建新的 InputArray 对象 或是默认参数 或是参数为空", des != NULL ? des : "");

		AddHelpMarker(c_des);
		ImGui::SameLine();
		ImGui::Combo(c_label, out_index, n_itmes, items_count + 1);
	}
}

//显示 cv api 窗口
static void ShowCVAPIWindow(GLFWwindow *window)
{
	ImGui::SetNextWindowPos(ImVec2(10, 25), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(380, 700), ImGuiCond_FirstUseEver);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoFocusOnAppearing;
	
	//OpenCV API Tools Demo Start
	if (ImGui::Begin("OpenCV API Tools Demo", NULL, flags))
	{
		//static Mat output_result;
		const int items_count = MatViewerManager::Instance().GetCount();

		//API imread
		if (ImGui::Button("imread()", EXCE_BUTTON_SIZE))
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
				Logger.AddLog(LogType::Info, "select file: %s\n", m_fileName);

				Mat source;
				static int flags = ImreadModes::IMREAD_COLOR;
				try
				{
					source = imread(m_fileName, flags);
					MatViewerManager::Instance().AddViewer(new MatViewer(title, source));
					Logger.AddLog(LogType::Info, "imread() create viewer: %s\n", title);
				}
				catch (Exception e)
				{
					Logger.AddLog(e, "imread() error");
				}

				source.release();
			}
		}

		//Base API
		if (ImGui::CollapsingHeader("Base API"))
		{
			//cvtColor()
			if (ImGui::TreeNode(u8"cvtColor(iod_d)"))
			{
				//cv Color Conversion Codes
				static EnumParser<ColorConversionCodes> cv_cccodes;

				//input
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index, true);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//code
				static int arg_code_index = 0;
				AddLeftLabel(u8"code", u8"颜色空间转换代码(更多值见 #ColorConversionCodes)");
				ImGui::Combo("##code", &arg_code_index, cv_cccodes.ParseMap2Items(), cv_cccodes.GetMapCount());

				//dstCn
				static int arg_dstCn_index = 0;
				AddLeftLabel(u8"dstCn", u8"目标图像中的通道数；如果参数为0，则通道数将自动从源图和代码中派生。");
				ImGui::SliderInt("##dstCn", &arg_dstCn_index, 0, 4);

				//执行 cvtColor()
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("cvtColor()"));

					Logger.AddLog(LogType::Code, "cvtColor(%s, %s, %s)", input_viewer->GetTitle(), output_viewer->GetTitle(), cv_cccodes.ParseIndex2Char(arg_code_index));
					try
					{
						cvtColor(input_viewer->GetMat(), output_viewer->GetMat(), cv_cccodes.ParseIndex2Enum(arg_code_index));
						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						Logger.AddLog(LogType::Info, "cvtColor() succeeded: %s\n", output_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "cvtColor() error: %s\n", output_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(output_viewer);
					}

					input_viewer = NULL;
					output_viewer = NULL;
				}
				ImGui::TreePop();
			}
			AddMarker(u8"色彩空间转换");

			//convertTo()
			if (ImGui::TreeNode(u8"convertTo(od_ff)"))
			{
				//input
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index, true);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);
				
				//rtype
				static int arg_rtype_index = 1;
				AddLeftLabel("rtype", "rtype");
				ImGui::Combo("##rtype", &arg_rtype_index, cv_dtype_char, cv_dtype_map.size());
				
				//alpha
				static float arg_alpha_value = 1.0;
				AddLeftLabel("alpha", "alpha default 1.0");
				ImGui::SliderFloat("##alpha", &arg_alpha_value, 0.0, 1.0, "%.1f");

				//beta
				static float arg_beta_value = 0.0;
				AddLeftLabel("beta", "beta default 0.0");
				ImGui::SliderFloat("##beta", &arg_beta_value, 0.0, 1.0, "%.1f");

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) &&
					arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("convertTo()"));
					arg_output_index = arg_output_index == items_count ? -1 : arg_output_index;

					try
					{
						input_viewer->GetMat().convertTo(output_viewer->GetMat(), get_cv_dtype(arg_rtype_index), arg_alpha_value, arg_beta_value);
						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						Logger.AddLog(LogType::Info, "%s succeeded: %s\n", "convertTo()", output_viewer->GetTitle());
					}
					catch (Exception e)
					{
						arg_input_index = -1;
						Logger.AddLog(e, "%s error, remove viewer:%s", "convertTo()", output_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(output_viewer);
					}

					input_viewer = NULL;
					output_viewer = NULL;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"Mat.convertTo(od_ff)");

			//convertScaleAbs()
			if (ImGui::TreeNode(u8"convertScaleAbs(io_ff)"))
			{
				//input
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index, true);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//alpha
				static float arg_alpha_value = 1.0;
				AddLeftLabel("alpha", u8"可选比例因子");
				ImGui::DragFloat("##alpha", &arg_alpha_value, 1.0f, 0.0, 1.0, "%.1f");

				//beta
				static float arg_beta_value = 0.0;
				AddLeftLabel("beta", u8"可选delta添加到缩放值");
				ImGui::DragFloat("##beta", &arg_beta_value, 1.0f, 0.0, 1.0, "%.1f");

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("convertScaleAbs()"));

					Logger.AddLog(LogType::Code, "convertScaleAbs(%s, %s, %.1f, %.1f)", input_viewer->GetTitle(), output_viewer->GetTitle(), arg_alpha_value, arg_beta_value);

					try
					{
						convertScaleAbs(input_viewer->GetMat(), output_viewer->GetMat(), arg_alpha_value, arg_beta_value);

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						Logger.AddLog(LogType::Info, "convertScaleAbs() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "convertScaleAbs() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
					output_viewer = NULL;
					if (arg_output_index == items_count) arg_output_index = -1;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"缩放、计算取绝对值，并将结果转换为8位");

			//flip()
			if (ImGui::TreeNode(u8"flip(iod)"))
			{
				//input
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//flipCode
				static int arg_flipCode_index = 0;
				AddLeftLabel(u8"flipCode", u8"0表示绕x轴翻转，1表示绕y轴翻转，-1表示在两个轴周围翻转");
				ImGui::SliderInt("##dstCn", &arg_flipCode_index, -1, 1);

				//执行 flip()
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("flip"));

					try
					{
						flip(input_viewer->GetMat(), output_viewer->GetMat(), arg_flipCode_index);

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						Logger.AddLog(LogType::Info, "flip() succeeded: %s\n", output_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "flip() error: %s\n", output_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(output_viewer);
					}
					input_viewer = NULL;
					output_viewer = NULL;
				}
				ImGui::TreePop();
			}
			AddMarker(u8"图像翻转");

			//resize()
			if (ImGui::TreeNode(u8"resize(ios_ffd)"))
			{
				//InterpolationFlags Enum
				static EnumParser<InterpolationFlags> ifs;

				//input
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index, true);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//dsize
				static int arg_dsize_value[2] = {500, 500};
				AddLeftLabel("dsize", "Size(width,height) 输出图像大小");
				ImGui::DragInt2("##dsize", arg_dsize_value, 1.0f, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE * 4, "%d");

				//fx fy
				static float arg_fxy_value[2] = {0.0, 0.0};
				AddLeftLabel("fx.fy", "fx/fy 沿水平/垂直轴的比例因子");
				ImGui::SliderFloat2("##fxy", arg_fxy_value, 0.0, 1.0, "%.1f");

				//interpolation
				static int arg_interpolation_index = 1;
				AddLeftLabel("interpolation", u8"插值方法/类型");
				ImGui::Combo("##interpolation", &arg_interpolation_index, ifs.ParseMap2Items(), ifs.GetMapCount());

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("resize()"));
					
					try
					{
						resize(input_viewer->GetMat(), output_viewer->GetMat(), Size(arg_dsize_value[0], arg_dsize_value[1]),
							arg_fxy_value[0], arg_fxy_value[1], ifs.ParseIndex2Enum(arg_interpolation_index));

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						Logger.AddLog(LogType::Info, "resize() succeeded: %s\n", output_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "resize() error: %s", output_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(output_viewer);
					}
					input_viewer = NULL;
					output_viewer = NULL;
					if(arg_output_index == items_count) arg_output_index = -1;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"图像缩放/插值");

			//inRange()
			if (ImGui::TreeNode(u8"inRange(iiio)"))
			{
				//ImGuiColorEditFlags_HDR
				static ImGuiColorEditFlags color_flags = ImGuiColorEditFlags_InputHSV | ImGuiColorEditFlags_DisplayHSV |
					ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreviewHalf | 
					ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoOptions ;

				//input
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index, true);

				//lowerb
				static ImVec4 lowerb;
				AddLeftLabel(u8"lowerb", u8"包含下边界数组或标量");
				ImGui::ColorEdit3("##lowerb", (float*)&lowerb, color_flags);

				//upperb
				static ImVec4 upperb;
				AddLeftLabel(u8"upperb", u8"包含上边界数组或标量");
				ImGui::ColorEdit3("##upperb", (float*)&upperb, color_flags);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//执行 inRange()
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("inRange()"));

					try
					{
						inRange(input_viewer->GetMat(), 
							Scalar(lowerb.x * 255, lowerb.y * 255, lowerb.z * 255), 
							Scalar(upperb.x * 255, upperb.y * 255, upperb.z * 255), 
							output_viewer->GetMat());

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						Logger.AddLog(LogType::Info, "inRange() succeeded: %s\n", output_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "inRange() error: %s", output_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(output_viewer);
					}
					input_viewer = NULL;
					output_viewer = NULL;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"检查数组元素是否位于另外两个数组的元素之间");

			//normalize()
			if (ImGui::TreeNode(u8"normalize(im_ffddi)"))
			{
				//input
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index, true);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, true);

				//alpha
				static float arg_alpha_value = 1.0;
				AddLeftLabel("alpha", u8"NORM_MINMAX时候低值");
				ImGui::SliderFloat("##alpha", &arg_alpha_value, 0.0, 1.0, "%.1f");

				//beta
				static float arg_beta_value = 0.0;
				AddLeftLabel("beta", u8"NORM_MINMAX时候高值");
				ImGui::SliderFloat("##beta", &arg_beta_value, 0.0, 1.0, "%.1f");

				//norm_type
				static EnumParser<NormTypes> nt;
				static int arg_type_index = 2;
				AddLeftLabel("norm_type", u8"归一化类型");
				ImGui::Combo("##norm_type", &arg_type_index, nt.ParseMap2Items(), nt.GetMapCount());

				//dtype
				static int arg_dtype_value = -1;
				AddLeftLabel("dtype", u8"如果为负，则输出数组与 src(input) 具有相同的类型；否则，输出数组与 src(input) 具有相同的通道数");
				ImGui::SliderInt("##dtype", &arg_dtype_value, -1, 1, "%d");

				//mask
				static int arg_mask_index = -1;
				AddViewerCombo("mask", &arg_mask_index, false);

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && 
					arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("normalize()"));
					
					InputArray mask = arg_mask_index == -1 || arg_mask_index == items_count ? noArray() : MatViewerManager::Instance().GetViewer(arg_mask_index)->GetMat();

					arg_mask_index = arg_mask_index == items_count ? -1 : arg_mask_index;
					arg_output_index = arg_output_index == items_count ? -1 : arg_output_index;

					try
					{
						normalize(input_viewer->GetMat(), output_viewer->GetMat(), arg_alpha_value, arg_beta_value, nt.ParseIndex2Enum(arg_type_index), arg_dtype_value, mask);
						output_viewer->UpdateMat();
						output_viewer->is_open = true;

						Logger.AddLog(LogType::Info, "%s succeeded: %s\n", "normalize()", output_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "%s error, remove viewer:%s", "normalize()", output_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(output_viewer);
					}

					mask.~_InputArray();
					input_viewer = NULL;
					output_viewer = NULL;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"数据归一化");

			//applyColorMap()
			if (ImGui::TreeNode(u8"applyColorMap(iod)"))
			{
				static EnumParser<ColormapTypes> ct;
				//input
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index, true);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//colormap
				static int arg_colormap_index = -1;
				AddLeftLabel("colormap", u8"应用 OpenCV 自带的颜色表");
				ImGui::Combo("##colormap", &arg_colormap_index, ct.ParseMap2Items(), ct.GetMapCount());

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("applyColorMap()"));

					try
					{
						applyColorMap(input_viewer->GetMat(), output_viewer->GetMat(), ct.ParseIndex2Enum(arg_colormap_index));

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						Logger.AddLog(LogType::Info, "applyColorMap() succeeded: %s\n", output_viewer->GetTitle());
					}
					catch (Exception e)
					{
						MatViewerManager::Instance().RemoveViewer(output_viewer);
						Logger.AddLog(e, "applyColorMap() error: %s\n", output_viewer->GetTitle());
					}
					input_viewer = NULL;
					output_viewer = NULL;
					if (arg_output_index == items_count) arg_output_index = -1;
				}
				ImGui::TreePop();
			}
			AddMarker(u8"Look Up Table(LUT)查找表");

			//applyColorMap(ioi)
			if (ImGui::TreeNode(u8"applyColorMap(ioi)"))
			{
				ImGui::Text("Waiting ... ");
				
				ImGui::TreePop();
			}
			AddMarker(u8"Look Up Table(LUT)查找表");

			//rotate()
		}

		//图像数据运算操作 API
		if (ImGui::CollapsingHeader(u8"图像数据运算操作"))
		{
			//[iio_id] add() subtract()
			{
				static const int count = 2;
				static const iio_id func_addr[count] = { add, subtract };
				static const char *func_name[count] = { u8"add(iio_id)", u8"subtract(iio_id)" };

				static int arg_input1_index[count] = { -1, -1 };
				static int arg_input2_index[count] = { -1, -1 };
				static int arg_output_index[count] = { -1, -1 };
				static int arg_mask_index[count] = { -1, -1 };
				static int arg_dtype_index[count] = { -1,-1 };

				for (int i = 0; i < count; i++)
				{
					if (ImGui::TreeNode(func_name[i]))
					{
						//input1
						AddViewerCombo("input1", &arg_input1_index[i], true);

						//input2
						AddViewerCombo("input2", &arg_input2_index[i], true);

						//output
						AddViewerCombo("output", &arg_output_index[i], false);

						//mask
						AddViewerCombo("mask", &arg_mask_index[i], false);

						//dtype
						AddLeftLabel(u8"dtype", u8"输出可选数组深度，默认为 -1");
						ImGui::Combo("##dtype", &arg_dtype_index[i], cv_dtype_char, cv_dtype_map.size());

						//执行
						if (ImGui::Button("exce", EXCE_BUTTON_SIZE) &&
							(arg_input1_index[i] != -1 && arg_input2_index[i] != -1))
						{
							MatViewer *input1_viewer = MatViewerManager::Instance().GetViewer(arg_input1_index[i]);
							MatViewer *input2_viewer = MatViewerManager::Instance().GetViewer(arg_input2_index[i]);
							MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index[i], input2_viewer->GetNextTitle(func_name[i]));
							InputArray mask = arg_mask_index[i] == -1 || arg_mask_index[i] == items_count ? noArray() : MatViewerManager::Instance().GetViewer(arg_mask_index[i])->GetMat();

							arg_mask_index[i] = arg_mask_index[i] == items_count ? -1 : arg_mask_index[i];
							arg_output_index[i] = arg_output_index[i] == items_count ? -1 : arg_output_index[i];

							try
							{
								func_addr[i](input1_viewer->GetMat(), input2_viewer->GetMat(), output_viewer->GetMat(), mask, get_cv_dtype(arg_dtype_index[i]));

								output_viewer->UpdateMat();
								output_viewer->is_open = true;
								output_viewer->SetViewerPos(input2_viewer->GetNextViewerPos());

								Logger.AddLog(LogType::Info, "%s succeeded: %s\n", func_name[i], output_viewer->GetTitle());
							}
							catch (Exception e)
							{
								Logger.AddLog(e, "%s error, remove viewer:%s", func_name[i], output_viewer->GetTitle());
								MatViewerManager::Instance().RemoveViewer(output_viewer);
							}

							mask.~_InputArray();
							input1_viewer = NULL;
							input2_viewer = NULL;
							output_viewer = NULL;
						}
						ImGui::TreePop();
					}

					AddMarker(func_name[i]);
				}
			}
			
			//[iio_fd] multiply() divide()
			{
				static const int count = 2;
				static const iio_fd func_addr[count] = { multiply , divide };
				static const char *func_name[count] = { u8"multiply(iio_fd)" , u8"divide(iio_fd)" };

				static int arg_input1_index[count] = { -1, -1 };
				static int arg_input2_index[count] = { -1, -1 };
				static int arg_output_index[count] = { -1, -1 };
				static float arg_scale_value[count] = { 1.0, 1.0 };
				static int arg_dtype_index[count] = { 0, 0 };

				for (int i = 0; i < count; i++)
				{
					if (ImGui::TreeNode(func_name[i]))
					{
						//input1
						AddViewerCombo("input1", &arg_input1_index[i], true);

						//input2
						AddViewerCombo("input2", &arg_input2_index[i], true);

						//output
						AddViewerCombo("output", &arg_output_index[i], false);

						//scale
						ImGui::Text("scale");
						ImGui::SameLine(COL_LEFT_WIDTH);
						AddHelpMarker(u8"optional scale factor, default 1.0");
						ImGui::SameLine();
						ImGui::SliderFloat("##scale", &arg_scale_value[i], 0.1f, 3.0f, "%.1f");

						//dtype
						AddLeftLabel(u8"dtype", u8"输出可选数组深度，默认为 -1");
						ImGui::Combo("##dtype", &arg_dtype_index[i], cv_dtype_char, cv_dtype_map.size());

						//执行
						if (ImGui::Button("exce", EXCE_BUTTON_SIZE) &&
							(arg_input1_index[i] != -1 && arg_input2_index[i] != -1))
						{
							MatViewer *input1_viewer = MatViewerManager::Instance().GetViewer(arg_input1_index[i]);
							MatViewer *input2_viewer = MatViewerManager::Instance().GetViewer(arg_input2_index[i]);
							MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index[i], input2_viewer->GetNextTitle(func_name[i]));
							arg_output_index[i] = arg_output_index[i] == items_count ? -1 : arg_output_index[i];

							try
							{
								func_addr[i](input1_viewer->GetMat(), input2_viewer->GetMat(), output_viewer->GetMat(), arg_scale_value[i], get_cv_dtype(arg_dtype_index[i]));

								output_viewer->UpdateMat();
								output_viewer->is_open = true;
								output_viewer->SetViewerPos(input2_viewer->GetNextViewerPos());

								Logger.AddLog(LogType::Info, "%s succeeded: %s\n", func_name[i], output_viewer->GetTitle());
							}
							catch (Exception e)
							{
								Logger.AddLog(e, "%s error, remove viewer:%s", func_name[i], output_viewer->GetTitle());
								MatViewerManager::Instance().RemoveViewer(output_viewer);
							}

							input1_viewer = NULL;
							input2_viewer = NULL;
							output_viewer = NULL;
						}
						ImGui::TreePop();
					}
					AddMarker(func_name[i]);
				}
			}

			//[iio_i] bitwise_and(), bitwise_or(), bitwise_xor()
			{
				static const int count = 3;
				static const iio_i func_addr[count] = { bitwise_and , bitwise_or, bitwise_xor };
				static const char *func_name[count] = { u8"bitwise_and(iio_i)" , u8"bitwise_or(iio_i)", u8"bitwise_xor(iio_i)" };

				static int arg_input1_index[count] = { -1, -1, -1 };
				static int arg_input2_index[count] = { -1, -1, -1 };
				static int arg_output_index[count] = { -1, -1, -1 };
				static int arg_mask_index[count] = { -1, -1, -1 };

				for (int i = 0; i < count; i++)
				{
					if (ImGui::TreeNode(func_name[i]))
					{
						//input1
						AddViewerCombo("input1", &arg_input1_index[i], true);

						//input2
						AddViewerCombo("input2", &arg_input2_index[i], true);

						//output
						AddViewerCombo("output", &arg_output_index[i], false);

						//mask
						AddViewerCombo("mask", &arg_mask_index[i], false);

						//执行
						if (ImGui::Button("exce", EXCE_BUTTON_SIZE) &&
							(arg_input1_index[i] != -1 && arg_input2_index[i] != -1))
						{
							MatViewer *input1_viewer = MatViewerManager::Instance().GetViewer(arg_input1_index[i]);
							MatViewer *input2_viewer = MatViewerManager::Instance().GetViewer(arg_input2_index[i]);
							MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index[i], input2_viewer->GetNextTitle(func_name[i]));

							InputArray mask = arg_mask_index[i] == -1 || arg_mask_index[i] == items_count ? noArray() : MatViewerManager::Instance().GetViewer(arg_mask_index[i])->GetMat();

							arg_mask_index[i] = arg_mask_index[i] == items_count ? -1 : arg_mask_index[i];
							arg_output_index[i] = arg_output_index[i] == items_count ? -1 : arg_output_index[i];

							try
							{
								func_addr[i](input1_viewer->GetMat(), input2_viewer->GetMat(), output_viewer->GetMat(), mask);

								output_viewer->UpdateMat();
								output_viewer->is_open = true;
								output_viewer->SetViewerPos(input2_viewer->GetNextViewerPos());

								Logger.AddLog(LogType::Info, "%s succeeded: %s\n", func_name[i], output_viewer->GetTitle());
							}
							catch (Exception e)
							{
								Logger.AddLog(e, "%s error. remove viewer:", func_name[i], output_viewer->GetTitle());
								MatViewerManager::Instance().RemoveViewer(output_viewer);
							}

							mask.~_InputArray();
							input1_viewer = NULL;
							input2_viewer = NULL;
							output_viewer = NULL;
						}
						ImGui::TreePop();
					}
					AddMarker(func_name[i]);
				}
			}

			//[io_i] bitwise_not()
			{
				static const int count = 1;
				static const io_i func_addr[count] = { bitwise_not };
				static const char *func_name[count] = { u8"bitwise_not(io_i)" };

				static int arg_input_index[count] = { -1 };
				static int arg_output_index[count] = { -1 };
				static int arg_mask_index[count] = { -1 };

				for (int i = 0; i < count; i++)
				{
					if (ImGui::TreeNode(func_name[i]))
					{
						//input
						AddViewerCombo("input", &arg_input_index[i], true);

						//output
						AddViewerCombo("output", &arg_output_index[i], false);

						//mask
						AddViewerCombo("mask", &arg_mask_index[i], false);

						//执行
						if (ImGui::Button("exce", EXCE_BUTTON_SIZE) &&
							(arg_input_index[i] != -1))
						{
							MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index[i]);
							MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index[i], input_viewer->GetNextTitle(func_name[i]));

							InputArray mask = arg_mask_index[i] == -1 || arg_mask_index[i] == items_count ? noArray() : MatViewerManager::Instance().GetViewer(arg_mask_index[i])->GetMat();

							arg_mask_index[i] = arg_mask_index[i] == items_count ? -1 : arg_mask_index[i];
							arg_output_index[i] = arg_output_index[i] == items_count ? -1 : arg_output_index[i];

							try
							{
								func_addr[i](input_viewer->GetMat(), output_viewer->GetMat(), mask);

								output_viewer->UpdateMat();
								output_viewer->is_open = true;
								output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

								Logger.AddLog(LogType::Info, "%s succeeded: %s\n", func_name[i], output_viewer->GetTitle());
							}
							catch (Exception e)
							{
								Logger.AddLog(e, "%s error. remove viewer:", func_name[i], output_viewer->GetTitle());
								MatViewerManager::Instance().RemoveViewer(output_viewer);
							}

							mask.~_InputArray();
							input_viewer = NULL;
							output_viewer = NULL;
						}
						ImGui::TreePop();
					}
					AddMarker(func_name[i]);
				}
			}
		}

		//几何形状绘制 API
		if (ImGui::CollapsingHeader(u8"绘制几何形状"))
		{			
			static EnumParser<LineTypes> lt;	//LineTypes Enum			
			static EnumParser<MarkerTypes> mt;	//MarkerTypes Enum

			static const char *arg_color_marker		= u8"形状颜色";
			static const char *arg_thickness_marker = u8"形状线条组细，FILLED类型表示该函数必须绘制一个填充形状";
			static const char *arg_lineType_marker	= u8"线段的类型，可选8(8邻接连接线)、4(4邻接连接线)和LINE_AA(antialiased 线条)";
			static const char *arg_shift_marker		= u8"表示形状中心点的小数位数";
			
			//line()
			if (ImGui::TreeNode(u8"line(mpps_ddd)"))
			{
				//input/output
				static int arg_input_index = -1;
				AddViewerCombo("input/output", &arg_input_index, true);

				//pt2
				static int arg_pt1_value[2] = {};
				AddLeftLabel("point1", u8"第一个点");
				ImGui::DragInt2("##pt1", arg_pt1_value, 1.0f, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE, "%d");

				//pt2
				static int arg_pt2_value[2] = {};
				AddLeftLabel("point2", u8"第二个点");
				ImGui::DragInt2("##pt2", arg_pt2_value, 1.0f, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE, "%d");

				//color
				static ImVec4 color;
				AddLeftLabel("color", arg_color_marker);
				ImGui::ColorEdit4("##color", (float*)&color);

				//thickness
				static int arg_thickness_value = 1;
				AddLeftLabel("thickness", arg_thickness_marker);
				ImGui::SliderInt("##thickness", &arg_thickness_value, ARG_THICK_MIN_VALUE, ARG_THICK_MAX_VALUE, "%d");

				//lineType
				static int arg_lineType_index = 2;
				AddLeftLabel("lineType", arg_lineType_marker);
				ImGui::Combo("##lineType", &arg_lineType_index, lt.ParseMap2Items(), lt.GetMapCount());

				//shift
				static int arg_shift_value = 0;
				AddLeftLabel("shift", arg_shift_marker);
				ImGui::SliderInt("##shift", &arg_shift_value, ARG_SHIFT_MIN_VALUE, ARG_SHIFT_MAX_VALUE, "%d");

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);

					try
					{

						line(input_viewer->GetMat(), Point(arg_pt1_value[0], arg_pt1_value[1]), Point(arg_pt2_value[0], arg_pt2_value[1]),
							Scalar(color.z * 255, color.y * 255, color.x * 255, color.w * 255), arg_thickness_value,
							lt.ParseIndex2Enum(arg_lineType_index), arg_shift_value);

						input_viewer->UpdateMat();
						input_viewer->is_open = true;

						Logger.AddLog(LogType::Info, "line() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "line() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"绘制线段符号");

			//arrowedLine()
			if (ImGui::TreeNode(u8"arrowedLine(mpps_dddf)"))
			{
				//input/output
				static int arg_input_index = -1;
				AddViewerCombo("input/output", &arg_input_index, true);

				//pt2
				static int arg_pt1_value[2] = {};
				AddLeftLabel("point1", u8"第一个点");
				ImGui::DragInt2("##pt1", arg_pt1_value, 1.0f, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE, "%d");

				//pt2
				static int arg_pt2_value[2] = {};
				AddLeftLabel("point2", u8"第二个点");
				ImGui::DragInt2("##pt2", arg_pt2_value, 1.0f, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE, "%d");

				//color
				static ImVec4 color;
				AddLeftLabel("color", arg_color_marker);
				ImGui::ColorEdit4("##color", (float*)&color);

				//thickness
				static int arg_thickness_value = 1;
				AddLeftLabel("thickness", arg_thickness_marker);
				ImGui::SliderInt("##thickness", &arg_thickness_value, ARG_THICK_MIN_VALUE, ARG_THICK_MAX_VALUE, "%d");

				//lineType
				static int arg_lineType_index = 2;
				AddLeftLabel("lineType", arg_lineType_marker);
				ImGui::Combo("##lineType", &arg_lineType_index, lt.ParseMap2Items(), lt.GetMapCount());

				//shift
				static int arg_shift_value = 0;
				AddLeftLabel("shift", arg_shift_marker);
				ImGui::SliderInt("##shift", &arg_shift_value, ARG_SHIFT_MIN_VALUE, ARG_SHIFT_MAX_VALUE, "%d");

				//tipLength
				static float arg_tipLength_value = 0.1;
				AddLeftLabel("tipLength", u8"箭头尖端相对于箭头长度的长度");
				ImGui::SliderFloat("##tipLength", &arg_tipLength_value, ARG_TIPLEN_MIN_VALUE, ARG_TIPLEN_MAX_VALUE, "%.1f");

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);

					try
					{

						arrowedLine(input_viewer->GetMat(), Point(arg_pt1_value[0], arg_pt1_value[1]), Point(arg_pt2_value[0], arg_pt2_value[1]),
							Scalar(color.z * 255, color.y * 255, color.x * 255, color.w * 255), arg_thickness_value,
							lt.ParseIndex2Enum(arg_lineType_index), arg_shift_value, arg_tipLength_value);

						input_viewer->UpdateMat();
						input_viewer->is_open = true;

						Logger.AddLog(LogType::Info, "arrowedLine() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "arrowedLine() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
				}
				ImGui::TreePop();
			}
			AddMarker(u8"绘制方向箭头符号");

			//rectangle()
			if (ImGui::TreeNode(u8"rectangle(mrs_ddd)"))
			{
				//input
				static int arg_input_index = -1;
				AddViewerCombo("input/output", &arg_input_index, true);

				//rect
				static int arg_rect_value[4] = {};
				AddLeftLabel("rect", u8"矩形区域");
				ImGui::DragInt4("##rect", arg_rect_value, 1.0f, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE, "%d");

				//color
				static ImVec4 color;
				AddLeftLabel("color", arg_color_marker);
				ImGui::ColorEdit4("##color", (float*)&color);

				//thickness
				static int arg_thickness_value = 1;
				AddLeftLabel("thickness", arg_thickness_marker);
				ImGui::SliderInt("##thickness", &arg_thickness_value, ARG_THICK_MIN_VALUE, ARG_THICK_MAX_VALUE, "%d");

				//lineType
				static int arg_lineType_index = 2;
				AddLeftLabel("lineType", arg_lineType_marker);
				ImGui::Combo("##lineType", &arg_lineType_index, lt.ParseMap2Items(), lt.GetMapCount());

				//shift
				static int arg_shift_value = 0;
				AddLeftLabel("shift", arg_shift_marker);
				ImGui::SliderInt("##shift", &arg_shift_value, ARG_SHIFT_MIN_VALUE, ARG_SHIFT_MAX_VALUE, "%d");

				//执行 rectangle()
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);

					try
					{
						Rect rect(arg_rect_value[0], arg_rect_value[1], arg_rect_value[2], arg_rect_value[3]);

						rectangle(input_viewer->GetMat(), rect,
							Scalar(color.z * 255, color.y * 255, color.x * 255, color.w * 255), arg_thickness_value,
							lt.ParseIndex2Enum(arg_lineType_index), arg_shift_value);

						input_viewer->UpdateMat();
						input_viewer->is_open = true;

						Logger.AddLog(LogType::Info, "rectangle() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "rectangle() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"绘制矩形符号");

			//circle()
			if (ImGui::TreeNode(u8"circle(mpds_ddd)"))
			{
				//input/output
				static int arg_input_index = -1;
				AddViewerCombo("input/output", &arg_input_index, true);

				//center
				static int arg_center_value[2] = {};
				AddLeftLabel("center", u8"中心点坐标");
				ImGui::DragInt2("##center", arg_center_value, 1.0f, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE, "%d");

				//radius
				static int arg_radius_value = 5;
				AddLeftLabel("radius", u8"圆半径");
				ImGui::SliderInt("##radius", &arg_radius_value, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE * 0.5, "%d");

				//color
				static ImVec4 color;
				AddLeftLabel("color", arg_color_marker);
				ImGui::ColorEdit4("##color", (float*)&color);

				//thickness
				static int arg_thickness_value = 1;
				AddLeftLabel("thickness", arg_thickness_marker);
				ImGui::SliderInt("##thickness", &arg_thickness_value, ARG_THICK_MIN_VALUE, ARG_THICK_MAX_VALUE, "%d");

				//lineType
				static int arg_lineType_index = 2;
				AddLeftLabel("lineType", arg_lineType_marker);
				ImGui::Combo("##lineType", &arg_lineType_index, lt.ParseMap2Items(), lt.GetMapCount());

				//shift
				static int arg_shift_value = 0;
				AddLeftLabel("shift", arg_shift_marker);
				ImGui::SliderInt("##shift", &arg_shift_value, ARG_SHIFT_MIN_VALUE, ARG_SHIFT_MAX_VALUE, "%d");

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);

					try
					{
						circle(input_viewer->GetMat(), Point(arg_center_value[0], arg_center_value[1]), arg_radius_value,
							Scalar(color.z * 255, color.y * 255, color.x * 255, color.w * 255), arg_thickness_value,
							lt.ParseIndex2Enum(arg_lineType_index), arg_shift_value);

						input_viewer->UpdateMat();
						input_viewer->is_open = true;

						Logger.AddLog(LogType::Info, "circle() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "circle() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
				}
				ImGui::TreePop();
			}
			AddMarker(u8"绘制圆形符号");

			//ellipse()
			if (ImGui::TreeNode(u8"ellipse(mpsfffs_ddd)"))
			{
				//input/output
				static int arg_input_index = -1;
				AddViewerCombo("input/output", &arg_input_index, true);

				//center
				static int arg_center_value[2] = {};
				AddLeftLabel("center", u8"中心点坐标");
				ImGui::DragInt2("##center", arg_center_value, 1.0f, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE, "%d");

				//axes
				static int arg_axes_value[2] = {};
				AddLeftLabel("axes", u8"Size width height");
				ImGui::SliderInt2("##axes", arg_axes_value, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE * 0.5, "%d");

				//angle, startAngle, endAngle
				static float arg_angle_value[3] = {};
				AddLeftLabel("angle", u8"angle, startAngle, endAngle");
				ImGui::SliderFloat3("##angle", arg_angle_value, -180.0, 180.0, "%.1f");

				//color
				static ImVec4 color;
				AddLeftLabel("color", arg_color_marker);
				ImGui::ColorEdit4("##color", (float*)&color);

				//thickness
				static int arg_thickness_value = 1;
				AddLeftLabel("thickness", arg_thickness_marker);
				ImGui::SliderInt("##thickness", &arg_thickness_value, ARG_THICK_MIN_VALUE, ARG_THICK_MAX_VALUE, "%d");

				//lineType
				static int arg_lineType_index = 2;
				AddLeftLabel("lineType", arg_lineType_marker);
				ImGui::Combo("##lineType", &arg_lineType_index, lt.ParseMap2Items(), lt.GetMapCount());

				//shift
				static int arg_shift_value = 0;
				AddLeftLabel("shift", arg_shift_marker);
				ImGui::SliderInt("##shift", &arg_shift_value, ARG_SHIFT_MIN_VALUE, ARG_SHIFT_MAX_VALUE, "%d");

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);

					try
					{
						ellipse(input_viewer->GetMat(), Point(arg_center_value[0], arg_center_value[1]), Size(arg_axes_value[0], arg_axes_value[1]),
							arg_angle_value[0], arg_angle_value[1], arg_angle_value[2],
							Scalar(color.z * 255, color.y * 255, color.x * 255, color.w * 255), 
							arg_thickness_value, lt.ParseIndex2Enum(arg_lineType_index), arg_shift_value);

						input_viewer->UpdateMat();
						input_viewer->is_open = true;

						Logger.AddLog(LogType::Info, "ellipse() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "ellipse() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
				}
				ImGui::TreePop();
			}
			AddMarker(u8"绘制弧形或椭圆符号");

			//drawMarker()
			if (ImGui::TreeNode(u8"drawMarker(mps_dddd)"))
			{
				//input/output
				static int arg_input_index = -1;
				AddViewerCombo("input/output", &arg_input_index, true);

				//position
				static int arg_position_value[2] = {};
				AddLeftLabel("position", u8"位置坐标");
				ImGui::DragInt2("##position", arg_position_value, 1.0f, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE, "%d");

				//color
				static ImVec4 color;
				AddLeftLabel("color", arg_color_marker);
				ImGui::ColorEdit4("##color", (float*)&color);

				//markerType
				static int arg_markerType_index = 0;
				AddLeftLabel("markerType", u8"标记类型");
				ImGui::Combo("##markerType", &arg_markerType_index, mt.ParseMap2Items(), mt.GetMapCount());

				//markerSize
				static int arg_markerSize_value = 20;
				AddLeftLabel("markerSize", u8"标记大小");
				ImGui::SliderInt("##markerSize", &arg_markerSize_value, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE * 0.5, "%d");

				//thickness
				static int arg_thickness_value = 1;
				AddLeftLabel("thickness", arg_thickness_marker);
				ImGui::SliderInt("##thickness", &arg_thickness_value, ARG_THICK_MIN_VALUE, ARG_THICK_MAX_VALUE, "%d");

				//lineType
				static int arg_lineType_index = 2;
				AddLeftLabel("lineType", arg_lineType_marker);
				ImGui::Combo("##lineType", &arg_lineType_index, lt.ParseMap2Items(), lt.GetMapCount());

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);

					try
					{
						drawMarker(input_viewer->GetMat(), Point(arg_position_value[0], arg_position_value[1]),
							Scalar(color.z * 255, color.y * 255, color.x * 255, color.w * 255), mt.ParseIndex2Enum(arg_markerType_index),
							arg_markerSize_value, arg_thickness_value, lt.ParseIndex2Enum(arg_lineType_index));

						input_viewer->UpdateMat();
						input_viewer->is_open = true;

						Logger.AddLog(LogType::Info, "drawMarker() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "drawMarker() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
				}
				ImGui::TreePop();
			}
			AddMarker(u8"绘制标记符号");

			//putText()
			if (ImGui::TreeNode(u8"putText(mtpdfs_ddb)"))
			{
				static EnumParser<HersheyFonts> hf;		//HersheyFonts Enum

				//input/output
				static int arg_input_index = -1;
				AddViewerCombo("input/output", &arg_input_index, true);

				//text
				static char arg_text_value[64] = "";
				AddLeftLabel("text", u8"要绘制的文本字符串");
				ImGui::InputText("##text", arg_text_value, IM_ARRAYSIZE(arg_text_value));

				//org
				static int arg_org_value[2] = {};
				AddLeftLabel("org", u8"图像中文本字符串的左下角");
				ImGui::DragInt2("##org", arg_org_value, 1.0f, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE, "%d");

				//fontFace
				static int arg_fontFace_index = 0;
				AddLeftLabel("fontFace", u8"字体类型");
				ImGui::Combo("##fontFace", &arg_fontFace_index, hf.ParseMap2Items(), hf.GetMapCount());

				//fontScale
				static float arg_fontScale_value = 1.0;
				AddLeftLabel("fontScale", u8"字体比例因子乘以特定于字体的基本大小");
				ImGui::SliderFloat("##fontScale", &arg_fontScale_value, 0.5, 10.0, "%.1f");

				//color
				static ImVec4 color;
				AddLeftLabel("color", u8"字体颜色");
				ImGui::ColorEdit4("##color", (float*)&color);

				//thickness
				static int arg_thickness_value = 1;
				AddLeftLabel("thickness", arg_thickness_marker);
				ImGui::SliderInt("##thickness", &arg_thickness_value, 1, ARG_THICK_MAX_VALUE, "%d");

				//lineType
				static int arg_lineType_index = 2;
				AddLeftLabel("lineType", arg_lineType_marker);
				ImGui::Combo("##lineType", &arg_lineType_index, lt.ParseMap2Items(), lt.GetMapCount());

				//bottomLeftOrigin
				static bool arg_bottomLeftOrigin_value = false;
				AddLeftLabel("BLOrigin", u8"如果为true，则图像数据原点位于左下角。否则，它位于左上角。");
				ImGui::Checkbox("bottomLeftOrigin", &arg_bottomLeftOrigin_value);

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);

					try
					{
						putText(input_viewer->GetMat(), arg_text_value, Point(arg_org_value[0], arg_org_value[1]),
							hf.ParseIndex2Enum(arg_fontFace_index), arg_fontScale_value, 
							Scalar(color.z * 255, color.y * 255, color.x * 255, color.w * 255),
							arg_thickness_value, lt.ParseIndex2Enum(arg_lineType_index), arg_bottomLeftOrigin_value);

						input_viewer->UpdateMat();
						input_viewer->is_open = true;

						Logger.AddLog(LogType::Info, "putText() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "putText() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"绘制文本符号");
		}

		//图像直方图分析
		if (ImGui::CollapsingHeader(u8"图像直方图分析"))
		{
			//calcHist
			if (ImGui::TreeNode(u8"calcHist() (drawCalcHist)"))
			{
				//input
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index, true);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("drawHist()"));
					Mat result;
					try
					{
						result = drawCalcHist(input_viewer->GetMat());
						output_viewer->LoadMat(result);
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						Logger.AddLog(LogType::Info, "calcHist() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "calcHist() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					result.release();
					input_viewer = NULL;
					output_viewer = NULL;
					if (arg_output_index == items_count) arg_output_index = -1;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"绘制直方图曲线");

			//equalizeHist(io)
			if (ImGui::TreeNode(u8"equalizeHist(io)"))
			{
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index, true, u8"8位单通道输入数据组");

				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false, u8"与 input 具有相同大小和类型的目标图像");

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("equalizeHist()"));
					
					try
					{
						equalizeHist(input_viewer->GetMat(), output_viewer->GetMat());
						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						Logger.AddLog(LogType::Info, "equalizeHist() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "equalizeHist() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
					output_viewer = NULL;
					if (arg_output_index == items_count) arg_output_index = -1;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"图像直方图均衡化");

			
			//图像直方图比较
			//图像直方图反向投影

			//calcBackProject()
			//calcBackProject
		}

		//图像卷积操作
		if (ImGui::CollapsingHeader(u8"图像卷积/滤波/噪声操作"))
		{
			static EnumParser<BorderTypes> bt;	//BorderTypes Enum

			//blur()
			if (ImGui::TreeNode(u8"blur(ios_pd)"))
			{
				//input
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index, true);

				//ouput
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//ksize
				static int arg_ksize_value[2] = {3, 3};
				AddLeftLabel("ksize", u8"卷积的窗口大小(尽量选择奇数，且宽高相等的正方形)");
				ImGui::DragInt2("##ksize", arg_ksize_value, 2.0f, 1, 255, "%d");

				//anchor
				static int arg_anchor_value[2] = {-1, -1};
				AddLeftLabel("anchor", u8"默认值 Point(-1,-1) 表示锚点位于卷积窗口的中心");
				ImGui::DragInt2("##anchor", arg_anchor_value, 1.0f, -1, 1, "%d");

				//borderType
				static int arg_borderType_index = 7;
				AddLeftLabel("borderType", u8"");
				ImGui::Combo("##borderType", &arg_borderType_index, bt.ParseMap2Items(), bt.GetMapCount());

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("blur()"));

					try
					{
						blur(input_viewer->GetMat(), output_viewer->GetMat(), 
							Size(arg_ksize_value[0], arg_ksize_value[1]),
							Point(arg_anchor_value[0], arg_anchor_value[1]),
							bt.ParseIndex2Enum(arg_borderType_index));

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						Logger.AddLog(LogType::Info, "blur() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "blur() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
					output_viewer = NULL;
					if (arg_output_index == items_count) arg_output_index = -1;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"图像卷积操作(均值卷积/滤波)\n图像卷积可以看成是一个窗口区域在另外一个大的图像上移动，对每个窗口覆盖的区域都进行点乘得到的值作为中心像素点的输出值。\n窗口的移动是从左到右，从上到下，窗口可以理解成一个指定大小的二维矩阵，里面有预先指定的值。");
			
			//medianBlur()
			if (ImGui::TreeNode(u8"medianBlur(iod)"))
			{
				//input
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index, true);

				//ouput
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//ksize
				static int arg_ksize_value = 0;
				AddLeftLabel("ksize", u8"卷积的窗口大小(选择奇数)");
				ImGui::DragInt("##ksize", &arg_ksize_value, 2.0f, 1, 255, "%d");

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("medianBlur()"));

					try
					{
						medianBlur(input_viewer->GetMat(), output_viewer->GetMat(),	arg_ksize_value);

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						Logger.AddLog(LogType::Info, "medianBlur() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "medianBlur() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
					output_viewer = NULL;
					if (arg_output_index == items_count) arg_output_index = -1;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"中值滤波器\n中值滤波本质上是统计排序滤波器的一种，中值滤波对图像特定噪声类型（椒盐噪声）会取得比较好的去噪效果，也是常见的图像去噪声与增强的方法之一。\n中值滤波也是窗口在图像上移动，其覆盖的对应ROI区域下，所有像素值排序，取中值作为中心像素点的输出值");

			//GaussianBlur()
			if (ImGui::TreeNode(u8"gaussianBlur(iosf_fd)"))
			{
				//input
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index, true);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//ksize
				static int arg_ksize_value[2] = { 3, 3 };
				AddLeftLabel("ksize", u8"高斯卷积的窗口大小(尽量选择奇数，且宽高相等的正方形)");
				ImGui::DragInt2("##ksize", arg_ksize_value, 2.0f, 1, 255, "%d");

				//sigmaX sigmaY
				static float arg_sigma_value[2] = {0.0, 0.0};
				AddLeftLabel("sigma", u8"sigmaX sigmaY xy方向的高斯核标准偏差\n如果sigmaY为零，则将其设置为等于sigmaX，如果两个sigma均为零，则分别从ksize.width和ksize.height计算");
				ImGui::SliderFloat2("##sigma", arg_sigma_value, 0.0, 128.0, "%.1f");

				//borderType
				static int arg_borderType_index = 7;
				AddLeftLabel("borderType", u8"");
				ImGui::Combo("##borderType", &arg_borderType_index, bt.ParseMap2Items(), bt.GetMapCount());

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("GaussianBlur()"));

					try
					{
						GaussianBlur(input_viewer->GetMat(), output_viewer->GetMat(),
							Size(arg_ksize_value[0], arg_ksize_value[1]),
							arg_sigma_value[0], arg_sigma_value[1],
							bt.ParseIndex2Enum(arg_borderType_index));

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						Logger.AddLog(LogType::Info, "GaussianBlur() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "GaussianBlur() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
					output_viewer = NULL;
					if (arg_output_index == items_count) arg_output_index = -1;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"高斯滤波器\n高斯模糊考虑了中心像素距离的影响，对距离中心像素使用高斯分布公式生成不同的权重系数给卷积核，然后用此卷积核完成图像卷积得到输出结果就是图像高斯模糊之后的输出。");
		
			//boxFilter() sqrBoxFilter()
			{
				static const int count = 2;
				static iods_pbd func_addr[count] = { boxFilter, sqrBoxFilter };
				static char *func_name[count] = { "boxFilter(iods_pbd)", "sqrBoxFilter(iods_pbd)" };
				static char *func_marker[count] = { u8"Blurs an image using the box filter.",
					u8"计算与过滤器重叠的像素值的标准化平方和"
				};

				static int arg_input_index[count] = { -1, -1 };
				static int arg_output_index[count] = { -1, -1 };
				static int arg_ddepth_value[count] = { -1, -1 };
				static int arg_ksize_value[count][2] = { {3, 3},{3,3} };

				static int arg_anchor_value[count][2] = { {-1, -1}, { -1, -1 } };
				static bool arg_normalize_value[count] = { true, true };
				static int arg_borderType_index[count] = { 7, 7 };

				for (int i = 0; i < count; i++)
				{
					if (ImGui::TreeNode(func_name[i]))
					{
						//input
						AddViewerCombo("input", &arg_input_index[i], true);

						//ouput
						AddViewerCombo("output", &arg_output_index[i], false);

						//ddepth
						AddLeftLabel("ddepth", u8"the output image depth (-1 to use src.depth())");
						ImGui::SliderInt("##ddepth", &arg_ddepth_value[i], -1, 4, "%d");

						//ksize
						//static int arg_ksize_value[2] = { 3, 3 };
						AddLeftLabel("ksize", u8"卷积的窗口大小(尽量选择奇数，且宽高相等的正方形)");
						ImGui::DragInt2("##ksize", arg_ksize_value[i], 2.0f, 1, 255, "%d");

						//anchor
						//static int arg_anchor_value[2] = { -1, -1 };
						AddLeftLabel("anchor", u8"默认值 Point(-1,-1) 表示锚点位于卷积窗口的中心");
						ImGui::DragInt2("##anchor", arg_anchor_value[i], 1.0f, -1, 1, "%d");

						//normalize
						//static bool arg_normalize_value = true;
						AddLeftLabel("normalize", u8"是否按卷积区域规一化，默认为 true");
						ImGui::Checkbox("##normalize", &arg_normalize_value[i]);

						//borderType
						//static int arg_borderType_index = 7;
						AddLeftLabel("borderType", u8"在图像外部外推像素的边框模式");
						ImGui::Combo("##borderType", &arg_borderType_index[i], bt.ParseMap2Items(), bt.GetMapCount());

						//执行
						if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index[i] != -1)
						{
							MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index[i]);
							MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index[i], input_viewer->GetNextTitle(func_name[i]));

							try
							{
								func_addr[i](input_viewer->GetMat(), output_viewer->GetMat(), arg_ddepth_value[i],
									Size(arg_ksize_value[i][0], arg_ksize_value[i][1]),
									Point(arg_anchor_value[i][0], arg_anchor_value[i][1]),
									arg_normalize_value[i], bt.ParseIndex2Enum(arg_borderType_index[i]));

								output_viewer->UpdateMat();
								output_viewer->is_open = true;
								output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

								Logger.AddLog(LogType::Info, "%s succeeded: %s\n", func_name[i], input_viewer->GetTitle());
							}
							catch (Exception e)
							{
								Logger.AddLog(e, "%s error: %s", func_name[i], input_viewer->GetTitle());
								MatViewerManager::Instance().RemoveViewer(input_viewer);
							}
							input_viewer = NULL;
							output_viewer = NULL;
							if (arg_output_index[i] == items_count) arg_output_index[i] = -1;
						}
						ImGui::TreePop();
					}
					AddMarker(func_marker[i]);
				}
			}

			//fastNlMeansDenoisingColored()
			if (ImGui::TreeNode(u8"fastNlMeansDenoisingColored(io_ffdd)"))
			{
				//input
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index, true);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//h
				static float arg_h_value = 3;
				AddLeftLabel("h", u8"调节亮度分量的滤波器强度，一般在10-15");
				ImGui::SliderFloat("#h", &arg_h_value, 0, 32, "%.1f");

				//hColor
				static float arg_hColor_value = 3;
				AddLeftLabel("hColor", u8"调节颜色分量的滤波器强度，一般在10-15");
				ImGui::SliderFloat("#hColor", &arg_hColor_value, 0, 32, "%.1f");

				//templateWindowSize
				static int arg_templateWindowSize_value = 7;
				AddLeftLabel("template", u8"templateWindowSize 模块窗口大小(templateWindowSize < searchWindowSize) 一般 templateWindowSize:searchWindowSize = 1:3");
				ImGui::SliderInt("##templateWindowSize", &arg_templateWindowSize_value, 1, 128, "%d");

				//searchWindowSize
				static int arg_searchWindowSize_value = 21;
				AddLeftLabel("search", u8"searchWindowSize 搜索窗口大小(templateWindowSize < searchWindowSize)，一般 templateWindowSize:searchWindowSize = 1:3");
				ImGui::SliderInt("##searchWindowSize", &arg_searchWindowSize_value, 1, 512, "%d");

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("fastNlMeansDenoisingColored()"));

					try
					{
						fastNlMeansDenoisingColored(input_viewer->GetMat(), output_viewer->GetMat(),
							arg_h_value, arg_hColor_value, arg_templateWindowSize_value, arg_searchWindowSize_value);

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						Logger.AddLog(LogType::Info, "fastNlMeansDenoisingColored() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "fastNlMeansDenoisingColored() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
					output_viewer = NULL;
					if (arg_output_index == items_count) arg_output_index = -1;
				}
				ImGui::TreePop();
			}
			AddMarker(u8"非局部均值去噪");

			//bilateralFilter()
			if (ImGui::TreeNode(u8"bilateralFilter(iodff_d)"))
			{
				//input
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index, true);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//d
				static int arg_d_value = 0;
				AddLeftLabel("d", u8"过滤期间使用的每个像素邻域的直径。如果它是非正数，则从sigmaSpace计算");
				ImGui::SliderInt("#h", &arg_d_value, -1, 32, "%d");

				//sigmaColor
				static float arg_sigmaColor_value = 100.0;
				AddLeftLabel("sigmaColor", u8"在颜色空间中过滤sigma。参数的值越大意味着像素邻域内的更远的颜色（参见sigmaSpace）将混合在一起，从而产生更大的半等颜色区域");
				ImGui::SliderFloat("##sigmaColor", &arg_sigmaColor_value, 0, 512, "%.1f");

				//sigmaSpace
				static float arg_sigmaSpace_value = 10.0;
				AddLeftLabel("sigmaSpace", u8"在坐标空间中过滤sigma。较大的参数值意味着只要它们的颜色足够接近，更远的像素就会相互影响\n当 d > 0 时，无论sigmaSpace如何，它都指定邻域大小。否则，d与sigmaSpace成比例");
				ImGui::SliderFloat("##sigmaSpace", &arg_sigmaSpace_value, 0, 512, "%.1f");

				//borderType
				static int arg_borderType_index = 7;
				AddLeftLabel("borderType", u8"边框模式用于外推图像外的像素");
				ImGui::Combo("##borderType", &arg_borderType_index, bt.ParseMap2Items(), bt.GetMapCount());

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("bilateralFilter()"));

					try
					{
						bilateralFilter(input_viewer->GetMat(), output_viewer->GetMat(),
							arg_d_value, arg_sigmaColor_value, arg_sigmaSpace_value, bt.ParseIndex2Enum(arg_borderType_index));

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						Logger.AddLog(LogType::Info, "bilateralFilter() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "bilateralFilter() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
					output_viewer = NULL;
					if (arg_output_index == items_count) arg_output_index = -1;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"边缘保留滤波算法–高斯双边模糊");

			//pyrMeanShiftFiltering()
			if (ImGui::TreeNode(u8"pyrMeanShiftFiltering(ioff_dt)"))
			{
				//input
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index, true);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//sp
				static float arg_sp_value = 15.0;
				AddLeftLabel("sp", u8"空间窗口半径");
				ImGui::SliderFloat("##sp", &arg_sp_value, 0, 512, "%.1f");

				//sr
				static float arg_sr_value = 50.0;
				AddLeftLabel("sr", u8"颜色窗口半径");
				ImGui::SliderFloat("##sr", &arg_sr_value, 0, 512, "%.1f");

				//maxLevel
				static int arg_maxLevel_value = 1;
				AddLeftLabel("maxLevel", u8"分割的金字塔的最大级别");
				ImGui::SliderInt("##maxLevel", &arg_maxLevel_value, 0, 8, "%d");
				
				//termcrit
				static TermCriteria arg_termcrit_value = TermCriteria(TermCriteria::MAX_ITER + TermCriteria::EPS, 5, 1);
				AddLeftLabel("termcrit", u8"终止条件：何时停止 meanshift 迭代");
				ImGui::Button("null", EXCE_BUTTON_SIZE);

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("pyrMeanShiftFiltering()"));

					try
					{
						pyrMeanShiftFiltering(input_viewer->GetMat(), output_viewer->GetMat(),
							arg_sp_value, arg_sr_value, arg_maxLevel_value, arg_termcrit_value);

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						Logger.AddLog(LogType::Info, "pyrMeanShiftFiltering() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "pyrMeanShiftFiltering() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
					output_viewer = NULL;
					if (arg_output_index == items_count) arg_output_index = -1;
				}
				ImGui::TreePop();
			}
			AddMarker(u8"边缘保留滤波算法–均值迁移模糊(mean-shift blur)");

			//图像积分图算法 integral

			//edgePreservingFilter()
			if (ImGui::TreeNode(u8"edgePreservingFilter(io_dff)"))
			{
				static int arg_flags[2] = { RECURS_FILTER , NORMCONV_FILTER };

				//input
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index, true, "输入8位3通道图像");

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false, "输出8位3通道图像");

				//flags
				static int arg_flags_value = 1;
				AddLeftLabel("flags", u8"1:RECURS_FILTER\n2:NORMCONV_FILTER");
				ImGui::SliderInt("##flags", &arg_flags_value, 1, 2, "%d");

				//sigma_s 
				static float arg_sigmas_value = 60;
				AddLeftLabel("sigma_s", u8"当sigma_s取值不变时候，sigma_r越大图像滤波效果越明显");
				ImGui::SliderFloat("##sigma_s", &arg_sigmas_value, 0, 200, "%.1f");

				//sigma_r
				static float arg_sigmar_value = 0.4;
				AddLeftLabel("sigma_r", u8"当sigma_r取值不变时候，窗口sigma_s越大图像模糊效果越明显\n当sgma_r取值很小的时候，窗口sigma_s取值无论如何变化，图像双边滤波效果都不好");
				ImGui::SliderFloat("##sigma_r", &arg_sigmar_value, 0, 1, "%.1f");

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("edgePreservingFilter()"));

					try
					{
						edgePreservingFilter(input_viewer->GetMat(), output_viewer->GetMat(),
							arg_flags_value, arg_sigmas_value, arg_sigmar_value);

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						Logger.AddLog(LogType::Info, "edgePreservingFilter() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "edgePreservingFilter() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
					output_viewer = NULL;
					if (arg_output_index == items_count) arg_output_index = -1;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"快速的图像边缘保留滤波算法");

			//OpenCV自定义的滤波器 filter2D

			//Sobel()
			if (ImGui::TreeNode(u8"Sobel(ioddd_dffd)"))
			{
				//input
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index, true);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//ddepth
				static int arg_ddepth_index = 6;
				AddLeftLabel("ddepth", u8"");
				ImGui::Combo("##ddepth", &arg_ddepth_index, cv_dtype_char, cv_dtype_map.size());

				//dx dy
				static int arg_dxy_value[2] = {1, 0};
				AddLeftLabel("dx/dy", u8"dx/dy");
				ImGui::SliderInt2("##dxy", arg_dxy_value, 0, 1, "%d");

				//ksize
				static int arg_ksize_value = 3;
				AddLeftLabel("ksize", u8"它必须是1, 3, 5或7");
				ImGui::DragInt("##ksize", &arg_ksize_value, 2.0f, 1, 31, "%d");

				//scale
				static float arg_scale_value = 1.1;
				AddLeftLabel("scale", u8"放缩比率，1 表示不变");
				ImGui::DragFloat("##scale", &arg_scale_value, 1.0f, 0.0f, 3.0f, "%.1f");

				//delta
				static float arg_delta_value = 0.0;
				AddLeftLabel("delta", u8"可选delta值，将值存储到 output 之前添加到结果中\n对输出结果图像加上常量值");
				ImGui::DragFloat("##delta", &arg_delta_value, 1.0f, 0.0f, 3.0f, "%.1f");

				//borderType
				static int arg_borderType_index = 7;
				AddLeftLabel("borderType", u8"");
				ImGui::Combo("##borderType", &arg_borderType_index, bt.ParseMap2Items(), bt.GetMapCount());

				//执行
				if (ImGui::Button("exce", EXCE_BUTTON_SIZE) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("Sobel()"));
					
					Logger.AddLog(LogType::Code, "Sobel(input, output, %s, %d, %d, %d, %.1f, %.1f, %s)\n", cv_dtype_char[arg_ddepth_index],
						arg_dxy_value[0], arg_dxy_value[1], arg_ksize_value, arg_scale_value, arg_delta_value, bt.ParseIndex2Char(arg_borderType_index));

					try
					{
						Sobel(input_viewer->GetMat(), output_viewer->GetMat(), get_cv_dtype(arg_ddepth_index) ,
							arg_dxy_value[0], arg_dxy_value[1], arg_ksize_value, 
							arg_scale_value, arg_delta_value, bt.ParseIndex2Enum(arg_borderType_index));

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						Logger.AddLog(LogType::Info, "Sobel() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						Logger.AddLog(e, "Sobel() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
					output_viewer = NULL;
					if (arg_output_index == items_count) arg_output_index = -1;
				}
				ImGui::TreePop();
			}
			AddMarker(u8"图像梯度–Sobel算子");
		}

		//二值图像分析
		if (ImGui::CollapsingHeader(u8"二值图像分析"))
		{
			ImGui::Text(u8"待添加 ...");

		}

		//图像形态学
		if (ImGui::CollapsingHeader(u8"图像形态学分析"))
		{
			ImGui::Text(u8"待添加 ...");
		}
		
	}

	//end
	ImGui::End();
}
