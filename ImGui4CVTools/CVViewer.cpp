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

//�������
const char window_title[] = "ImGui for OpenCV Tools Viewer";
//���屳����ɫ
const ImVec4 ClearColor(0.01f, 0.01f, 0.01f, 1.0f);

void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);

	std::ofstream errorLogger;
	errorLogger.open("k4aviewer.err", std::ofstream::out | std::ofstream::app);
	errorLogger << "Glfw error [" << error << "]: " << description << std::endl;
	errorLogger.close();
}


//	OpenCV Main Viewer ��ͼ
CVViewer::CVViewer()
{
	m_showDemoWindow = false;
	glfwSetErrorCallback(glfw_error_callback);
	
	cv_rtype_it = cv_rtype_map.begin();
	for (int i = 0; cv_rtype_it != cv_rtype_map.end(); cv_rtype_it++, i++)
		cv_rtype_char[i] = cv_rtype_it->first;

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

	//���ñ��洰�ڲ���
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

		//��ʼ ImGui ֡
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ShowMainMenuBar(m_showDemoWindow, m_showCVAPIWindow, m_showLogWindow, m_showCVAPIHelp);

		ShowCVAPIWindow(window);
		if (m_showLogWindow)	ShowLoggerWindow(&m_showLogWindow);
		if (m_showDemoWindow)	ImGui::ShowDemoWindow(&m_showDemoWindow);
		if (m_showCVAPIHelp)	ShowCVAPIHelpWindow(&m_showCVAPIHelp);

		//Render CV Mat Viewers
		MatViewerManager::Instance().RenderViewer();

		// ���/��Ⱦ֡
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

//�����־��Ϣ
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
//����쳣��Ϣ
static void AddLogger(Exception ex, const char *msg, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, IM_ARRAYSIZE(buf), msg, args);
	buf[IM_ARRAYSIZE(buf) - 1] = 0;
	va_end(args);

	AddLogger(LogType::Error, "%s: \n\t\t\t\t%s\n", buf, ex.what());
	//AddLogger(LogType::Error, "%s: code:%d func:%s msg:%s, line:%d\n \t\t\t\n%s\n", msg, e.code, e.func, e.msg, e.line, e.what());
}

//��� Marker ���
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

//��� help ���
static void AddHelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	AddMarker(desc);
}

//�������ǩ
static void AddLeftLabel(const char *label, const char *helpDes)
{
	ImGui::Text(label);
	ImGui::SameLine(COL_LEFT_WIDTH);

	if (helpDes != NULL)
		AddHelpMarker(helpDes);

	ImGui::SameLine();
}

//�豸��ʾ DPI ����
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

//��ʾ���˵���
static void ShowMainMenuBar(bool &m_showDemoWindow, bool &m_showCVAPIWindow, bool &m_showLogger, bool &m_showCVAPIHelp)
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu(u8"Settings ����"))
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
			if (ImGui::MenuItem("OpenCV API Help"))
				m_showCVAPIHelp = !m_showCVAPIHelp;
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

//��ʾ��־����
static void ShowLoggerWindow(bool* p_open)
{
	ImGui::SetNextWindowPos(ImVec2(1000, 25), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

	ImGui::Begin("Logger", p_open);	
	ImGui::End();

	logger.Draw("Logger", p_open);
}

static void AddTableRow(const char *label1, const char *label2, const char *label3)
{
	ImGui::Text(label1);
	ImGui::SameLine(50);
	ImGui::Text(label2);
	ImGui::SameLine(200);
	ImGui::Text(label3);
}

static void ShowCVAPIHelpWindow(bool *p_open)
{
	ImGui::SetNextWindowPos(ImVec2(500, 300), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
	
	ImGui::Begin("OpenCV(v410) API Help", p_open);

	ImGui::Text(u8"API����˵����('_'֮���ʾ��ʹ��Ĭ��ֵ)");

	AddTableRow("i", "InputArray", u8"��ʾΪ������������");
	AddTableRow("o", "OutputArray", u8"��ʾΪ�����������");
	AddTableRow("m", "InputOutputArray", u8"��ʾΪ����/����������");
	AddTableRow("d", "int,uint ...", u8"��������");
	AddTableRow("f", "float,double ...", u8"���������");
	AddTableRow("p", "Point", u8"");
	AddTableRow("r", "Rect", u8"");
	AddTableRow("s", "Scalar", u8"Scalar,Size");
	AddTableRow("v", "ImVec(n)", u8"");
	AddTableRow("t", "Text", u8"");
	AddTableRow("b", "Boolean", u8"");

	ImGui::Text(u8"ע�⣺OpenCV API ��������ʹ�õĶ��� double ���ͣ�\n\tImGui ��������ʹ�õĶ��� float ���ͣ�����һ��ľ�����ʧ��");

	ImGui::End();
}

//��� Viewer List Guide Combo 
static void AddViewerCombo(const char *label, int *out_index, bool input = true, const char *des = NULL)
{
	const int items_count = MatViewerManager::Instance().GetCount();
	const char **items = MatViewerManager::Instance().GetAllTitle();
	const char **n_itmes = MatViewerManager::Instance().GetAllTitle("Create New Viewer");

	ImGui::Text(label);
	ImGui::SameLine(COL_LEFT_WIDTH);

	static char c_label[255] = {};
	sprintf_s(c_label, "%s%s", "##", label);
	
	char c_des[255] = {};

	if (input)
	{
		if (des != NULL)
			sprintf_s(c_des, u8"ѡ��������ͼ���ڵ� InputArray ����\n%s", des);
		else
			sprintf_s(c_des, u8"ѡ��������ͼ���ڵ� InputArray ����");

		AddHelpMarker(u8"ѡ��������ͼ���ڵ� InputArray ����");
		ImGui::SameLine();
		ImGui::Combo(c_label, out_index, items, items_count);
	}
	else
	{
		if (des != NULL)
			sprintf_s(c_des, u8"ѡ�����ͼ���ڵ� InputArray ����\nѡ�� [Create New Viewer] ��ѡ�񣬱�ʾ�����µ� InputArray ���� ����Ĭ�ϲ��� ���ǲ���Ϊ��\n%s", des);
		else
			sprintf_s(c_des, u8"ѡ�����ͼ���ڵ� InputArray ����\nѡ�� [Create New Viewer] ��ѡ�񣬱�ʾ�����µ� InputArray ���� ����Ĭ�ϲ��� ���ǲ���Ϊ��");

		AddHelpMarker(c_des);
		ImGui::SameLine();
		ImGui::Combo(c_label, out_index, n_itmes, items_count + 1);
	}
}

//��ʾ cv api ����
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
		if (ImGui::Button("imread()", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)))
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
				AddLeftLabel(u8"code", u8"��ɫ�ռ�ת������(����ֵ�� #ColorConversionCodes)");
				ImGui::Combo("##code", &arg_code_index, cv_cccodes.ParseMap2Items(), cv_cccodes.GetMapCount());

				//dstCn
				static int arg_dstCn_index = 0;
				AddLeftLabel(u8"dstCn", u8"Ŀ��ͼ���е�ͨ�������������Ϊ0����ͨ�������Զ���Դͼ�ʹ�����������");
				ImGui::SliderInt("##dstCn", &arg_dstCn_index, 0, 4);

				//ִ�� cvtColor()
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("cvtColor()"));

					try
					{
						cvtColor(input_viewer->GetMat(), output_viewer->GetMat(), cv_cccodes.ParseIndex2Enum(arg_code_index));
						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						AddLogger(LogType::Info, "cvtColor() succeeded: %s\n", output_viewer->GetTitle());
					}
					catch (Exception e)
					{
						AddLogger(LogType::Info, "cvtColor() error: %s\n", output_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(output_viewer);
					}

					input_viewer = NULL;
					output_viewer = NULL;
				}
				ImGui::TreePop();
			}
			AddMarker(u8"ɫ�ʿռ�ת��");

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
				static int arg_rtype_value = 0;
				static int arg_rtype_index = 0;
				AddLeftLabel("rtype", "rtype");
				ImGui::Combo("##rtype", &arg_rtype_index, cv_rtype_char, cv_rtype_map.size());
				
				//alpha
				static float arg_alpha_value = 1.0;
				AddLeftLabel("alpha", "alpha default 1.0");
				ImGui::SliderFloat("##alpha", &arg_alpha_value, 0.0, 1.0, "%.1f");

				//beta
				static float arg_beta_value = 0.0;
				AddLeftLabel("beta", "beta default 0.0");
				ImGui::SliderFloat("##beta", &arg_beta_value, 0.0, 1.0, "%.1f");

				//ִ��
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) &&
					arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("convertTo()"));
					arg_output_index = arg_output_index == items_count ? -1 : arg_output_index;
					
					arg_rtype_value = GetCVRtype(arg_rtype_index);

					try
					{
						input_viewer->GetMat().convertTo(output_viewer->GetMat(), arg_rtype_value, arg_alpha_value, arg_beta_value);
						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						AddLogger(LogType::Info, "%s succeeded: %s\n", "convertTo()", output_viewer->GetTitle());
					}
					catch (Exception e)
					{
						arg_input_index = -1;
						AddLogger(e, "%s error, remove viewer:%s", "convertTo()", output_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(output_viewer);
					}

					input_viewer = NULL;
					output_viewer = NULL;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"Mat.convertTo(od_ff)");

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
				AddLeftLabel(u8"flipCode", u8"0��ʾ��x�ᷭת��1��ʾ��y�ᷭת��-1��ʾ����������Χ��ת");
				ImGui::SliderInt("##dstCn", &arg_flipCode_index, -1, 1);

				//ִ�� flip()
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
						AddLogger(e, "flip() error: %s\n", output_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(output_viewer);
					}
					input_viewer = NULL;
					output_viewer = NULL;
				}
				ImGui::TreePop();
			}
			AddMarker(u8"ͼ��ת");

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
				AddLeftLabel("dsize", "Size(width,height) ���ͼ���С");
				ImGui::DragInt2("##dsize", arg_dsize_value, 1.0f, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE * 4, "%d");

				//fx fy
				static float arg_fxy_value[2] = {0.0, 0.0};
				AddLeftLabel("fx.fy", "fx/fy ��ˮƽ/��ֱ��ı�������");
				ImGui::SliderFloat2("##fxy", arg_fxy_value, 0.0, 1.0, "%.1f");

				//interpolation
				static int arg_interpolation_index = 1;
				AddLeftLabel("interpolation", u8"��ֵ����/����");
				ImGui::Combo("##interpolation", &arg_interpolation_index, ifs.ParseMap2Items(), ifs.GetMapCount());

				//ִ��
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) && arg_input_index != -1)
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

						AddLogger(LogType::Info, "resize() succeeded: %s\n", output_viewer->GetTitle());
					}
					catch (Exception e)
					{
						AddLogger(e, "resize() error: %s", output_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(output_viewer);
					}
					input_viewer = NULL;
					output_viewer = NULL;
					if(arg_output_index == items_count) arg_output_index = -1;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"ͼ������/��ֵ");

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
				AddLeftLabel(u8"lowerb", u8"�����±߽���������");
				ImGui::ColorEdit3("##lowerb", (float*)&lowerb, color_flags);

				//upperb
				static ImVec4 upperb;
				AddLeftLabel(u8"upperb", u8"�����ϱ߽���������");
				ImGui::ColorEdit3("##upperb", (float*)&upperb, color_flags);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//ִ�� inRange()
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) && arg_input_index != -1)
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

						AddLogger(LogType::Info, "inRange() succeeded: %s\n", output_viewer->GetTitle());
					}
					catch (Exception e)
					{
						AddLogger(e, "inRange() error: %s", output_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(output_viewer);
					}
					input_viewer = NULL;
					output_viewer = NULL;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"�������Ԫ���Ƿ�λ���������������Ԫ��֮��");

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
				AddLeftLabel("alpha", u8"NORM_MINMAXʱ���ֵ");
				ImGui::SliderFloat("##alpha", &arg_alpha_value, 0.0, 1.0, "%.1f");

				//beta
				static float arg_beta_value = 0.0;
				AddLeftLabel("beta", u8"NORM_MINMAXʱ���ֵ");
				ImGui::SliderFloat("##beta", &arg_beta_value, 0.0, 1.0, "%.1f");

				//norm_type
				static EnumParser<NormTypes> nt;
				static int arg_type_index = 2;
				AddLeftLabel("norm_type", u8"��һ������");
				ImGui::Combo("##norm_type", &arg_type_index, nt.ParseMap2Items(), nt.GetMapCount());

				//dtype
				static int arg_dtype_value = -1;
				AddLeftLabel("dtype", u8"���Ϊ��������������� src(input) ������ͬ�����ͣ�������������� src(input) ������ͬ��ͨ����");
				ImGui::SliderInt("##dtype", &arg_dtype_value, -1, 1, "%d");

				//mask
				static int arg_mask_index = -1;
				AddViewerCombo("mask", &arg_mask_index, false);

				//ִ��
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) && 
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

						AddLogger(LogType::Info, "%s succeeded: %s\n", "normalize()", output_viewer->GetTitle());
					}
					catch (Exception e)
					{
						AddLogger(e, "%s error, remove viewer:%s", "normalize()", output_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(output_viewer);
					}

					mask.~_InputArray();
					input_viewer = NULL;
					output_viewer = NULL;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"���ݹ�һ��");

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
				AddLeftLabel("colormap", u8"Ӧ�� OpenCV �Դ�����ɫ��");
				ImGui::Combo("##colormap", &arg_colormap_index, ct.ParseMap2Items(), ct.GetMapCount());

				//ִ��
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("applyColorMap()"));

					try
					{
						applyColorMap(input_viewer->GetMat(), output_viewer->GetMat(), ct.ParseIndex2Enum(arg_colormap_index));

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						AddLogger(LogType::Info, "applyColorMap() succeeded: %s\n", output_viewer->GetTitle());
					}
					catch (Exception e)
					{
						MatViewerManager::Instance().RemoveViewer(output_viewer);
						AddLogger(e, "applyColorMap() error: %s\n", output_viewer->GetTitle());
					}
					input_viewer = NULL;
					output_viewer = NULL;
					if (arg_output_index == items_count) arg_output_index = -1;
				}
				ImGui::TreePop();
			}
			AddMarker(u8"Look Up Table(LUT)���ұ�");

			//applyColorMap(ioi)
			if (ImGui::TreeNode(u8"applyColorMap(ioi)"))
			{
				/*
				//input
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index, true);

				//output
				static int arg_output_index = -1;
				AddViewerCombo("output", &arg_output_index, false);

				//userColor
				static int arg_userColor_index = -1;
				AddViewerCombo("userColor", &arg_userColor_index, true);

				//ִ��
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) &&
					(arg_input_index != -1 && arg_userColor_index != -1))
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("applyColorMap(ioi)"));

					InputArray userColor = arg_userColor_index == -1 || arg_userColor_index == items_count ? noArray() : MatViewerManager::Instance().GetViewer(arg_userColor_index)->GetMat();
					arg_output_index = arg_output_index == items_count ? -1 : arg_output_index;

					try
					{
						applyColorMap(input_viewer->GetMat(), output_viewer->GetMat(), userColor);

						output_viewer->UpdateMat();
						output_viewer->is_open = true;
						output_viewer->SetViewerPos(input_viewer->GetNextViewerPos());

						AddLogger(LogType::Info, "applyColorMap(ioi) succeeded: %s\n",  output_viewer->GetTitle());
					}
					catch (Exception e)
					{
						AddLogger(e, "applyColorMap(ioi) error. remove viewer:", output_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(output_viewer);
					}

					input_viewer = NULL;
					output_viewer = NULL;
				}
				*/
				ImGui::TreePop();
			}
			AddMarker(u8"Look Up Table(LUT)���ұ�");

			//rotate()
		}

		//ͼ������������� API
		if (ImGui::CollapsingHeader(u8"ͼ�������������"))
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
				static int arg_dtype_value[count] = { -1,-1 };

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
						AddLeftLabel(u8"dtype", u8"�����ѡ������ȣ�Ĭ��Ϊ -1");
						ImGui::SliderInt("##dtype", &arg_dtype_value[i], -1, 4);

						//ִ��
						if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) &&
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
								func_addr[i](input1_viewer->GetMat(), input2_viewer->GetMat(), output_viewer->GetMat(), mask, arg_dtype_value[i]);

								output_viewer->UpdateMat();
								output_viewer->is_open = true;
								output_viewer->SetViewerPos(input2_viewer->GetNextViewerPos());

								AddLogger(LogType::Info, "%s succeeded: %s\n", func_name[i], output_viewer->GetTitle());
							}
							catch (Exception e)
							{
								AddLogger(e, "%s error, remove viewer:%s", func_name[i], output_viewer->GetTitle());
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
				static int arg_dtype_value[count] = { -1,-1 };

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
						AddLeftLabel(u8"dtype", u8"�����ѡ������ȣ�Ĭ��Ϊ -1");
						ImGui::SliderInt("##dtype", &arg_dtype_value[i], -1, 4);

						//ִ��
						if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) &&
							(arg_input1_index[i] != -1 && arg_input2_index[i] != -1))
						{
							MatViewer *input1_viewer = MatViewerManager::Instance().GetViewer(arg_input1_index[i]);
							MatViewer *input2_viewer = MatViewerManager::Instance().GetViewer(arg_input2_index[i]);
							MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index[i], input2_viewer->GetNextTitle(func_name[i]));
							arg_output_index[i] = arg_output_index[i] == items_count ? -1 : arg_output_index[i];

							try
							{
								func_addr[i](input1_viewer->GetMat(), input2_viewer->GetMat(), output_viewer->GetMat(), arg_scale_value[i], arg_dtype_value[i]);

								output_viewer->UpdateMat();
								output_viewer->is_open = true;
								output_viewer->SetViewerPos(input2_viewer->GetNextViewerPos());

								AddLogger(LogType::Info, "%s succeeded: %s\n", func_name[i], output_viewer->GetTitle());
							}
							catch (Exception e)
							{
								AddLogger(e, "%s error, remove viewer:%s", func_name[i], output_viewer->GetTitle());
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

						//ִ��
						if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) &&
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

								AddLogger(LogType::Info, "%s succeeded: %s\n", func_name[i], output_viewer->GetTitle());
							}
							catch (Exception e)
							{
								AddLogger(e, "%s error. remove viewer:", func_name[i], output_viewer->GetTitle());
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

						//ִ��
						if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) &&
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

								AddLogger(LogType::Info, "%s succeeded: %s\n", func_name[i], output_viewer->GetTitle());
							}
							catch (Exception e)
							{
								AddLogger(e, "%s error. remove viewer:", func_name[i], output_viewer->GetTitle());
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

		//������״���� API
		if (ImGui::CollapsingHeader(u8"���Ƽ�����״"))
		{			
			static EnumParser<LineTypes> lt;	//LineTypes Enum			
			static EnumParser<MarkerTypes> mt;	//MarkerTypes Enum

			//line()
			if (ImGui::TreeNode(u8"line(mpps_ddd)"))
			{
				//input/output
				static int arg_input_index = -1;
				AddViewerCombo("input/output", &arg_input_index, true);

				//pt2
				static int arg_pt1_value[2] = {};
				AddLeftLabel("point1", u8"��һ����");
				ImGui::DragInt2("##pt1", arg_pt1_value, 1.0f, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE, "%d");

				//pt2
				static int arg_pt2_value[2] = {};
				AddLeftLabel("point2", u8"�ڶ�����");
				ImGui::DragInt2("##pt2", arg_pt2_value, 1.0f, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE, "%d");

				//color
				static ImVec4 color;
				AddLeftLabel("color", u8"������ɫ");
				ImGui::ColorEdit4("##color", (float*)&color);

				//thickness
				static int arg_thickness_value = 1;
				AddLeftLabel("thickness", u8"������ϸ��FILLED���ͱ�ʾ�ú����������һ��������");
				ImGui::SliderInt("##thickness", &arg_thickness_value, ARG_THICK_MIN_VALUE, ARG_THICK_MAX_VALUE, "%d");

				//lineType
				static int arg_lineType_index = 2;
				AddLeftLabel("lineType", u8"��������");
				ImGui::Combo("##lineType", &arg_lineType_index, lt.ParseMap2Items(), lt.GetMapCount());

				//shift
				static int arg_shift_value = 0;
				AddLeftLabel("shift", u8"�������е�С��λ��");
				ImGui::SliderInt("##shift", &arg_shift_value, ARG_SHIFT_MIN_VALUE, ARG_SHIFT_MAX_VALUE, "%d");

				//ִ��
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);

					try
					{

						line(input_viewer->GetMat(), Point(arg_pt1_value[0], arg_pt1_value[1]), Point(arg_pt2_value[0], arg_pt2_value[1]),
							Scalar(color.z * 255, color.y * 255, color.x * 255, color.w * 255), arg_thickness_value,
							lt.ParseIndex2Enum(arg_lineType_index), arg_shift_value);

						input_viewer->UpdateMat();
						input_viewer->is_open = true;

						AddLogger(LogType::Info, "line() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						AddLogger(e, "line() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"�����߶η���");

			//arrowedLine()
			if (ImGui::TreeNode(u8"arrowedLine(mpps_dddf)"))
			{
				//input/output
				static int arg_input_index = -1;
				AddViewerCombo("input/output", &arg_input_index, true);

				//pt2
				static int arg_pt1_value[2] = {};
				AddLeftLabel("point1", u8"��һ����");
				ImGui::DragInt2("##pt1", arg_pt1_value, 1.0f, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE, "%d");

				//pt2
				static int arg_pt2_value[2] = {};
				AddLeftLabel("point2", u8"�ڶ�����");
				ImGui::DragInt2("##pt2", arg_pt2_value, 1.0f, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE, "%d");

				//color
				static ImVec4 color;
				AddLeftLabel("color", u8"������ɫ");
				ImGui::ColorEdit4("##color", (float*)&color);

				//thickness
				static int arg_thickness_value = 1;
				AddLeftLabel("thickness", u8"������ϸ��FILLED���ͱ�ʾ�ú����������һ��������");
				ImGui::SliderInt("##thickness", &arg_thickness_value, ARG_THICK_MIN_VALUE, ARG_THICK_MAX_VALUE, "%d");

				//lineType
				static int arg_lineType_index = 2;
				AddLeftLabel("lineType", u8"��������");
				ImGui::Combo("##lineType", &arg_lineType_index, lt.ParseMap2Items(), lt.GetMapCount());

				//shift
				static int arg_shift_value = 0;
				AddLeftLabel("shift", u8"�������е�С��λ��");
				ImGui::SliderInt("##shift", &arg_shift_value, ARG_SHIFT_MIN_VALUE, ARG_SHIFT_MAX_VALUE, "%d");

				//tipLength
				static float arg_tipLength_value = 0.1;
				AddLeftLabel("tipLength", u8"��ͷ�������ڼ�ͷ���ȵĳ���");
				ImGui::SliderFloat("##tipLength", &arg_tipLength_value, ARG_TIPLEN_MIN_VALUE, ARG_TIPLEN_MAX_VALUE, "%.1f");

				//ִ��
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);

					try
					{

						arrowedLine(input_viewer->GetMat(), Point(arg_pt1_value[0], arg_pt1_value[1]), Point(arg_pt2_value[0], arg_pt2_value[1]),
							Scalar(color.z * 255, color.y * 255, color.x * 255, color.w * 255), arg_thickness_value,
							lt.ParseIndex2Enum(arg_lineType_index), arg_shift_value, arg_tipLength_value);

						input_viewer->UpdateMat();
						input_viewer->is_open = true;

						AddLogger(LogType::Info, "arrowedLine() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						AddLogger(e, "arrowedLine() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
				}
				ImGui::TreePop();
			}
			AddMarker(u8"���Ʒ����ͷ����");

			//rectangle()
			if (ImGui::TreeNode(u8"rectangle(mrs_ddd)"))
			{
				//input
				static int arg_input_index = -1;
				AddViewerCombo("input/output", &arg_input_index, true);

				//rect
				static int arg_rect_value[4] = {};
				AddLeftLabel("rect", u8"��������");
				ImGui::DragInt4("##rect", arg_rect_value, 1.0f, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE, "%d");

				//color
				static ImVec4 color;
				AddLeftLabel("color", u8"������ɫ");
				ImGui::ColorEdit4("##color", (float*)&color);

				//thickness
				static int arg_thickness_value = 1;
				AddLeftLabel("thickness", u8"������ϸ��FILLED���ͱ�ʾ�ú����������һ��������");
				ImGui::SliderInt("##thickness", &arg_thickness_value, ARG_THICK_MIN_VALUE, ARG_THICK_MAX_VALUE, "%d");

				//lineType
				static int arg_lineType_index = 2;
				AddLeftLabel("lineType", u8"��������");
				ImGui::Combo("##lineType", &arg_lineType_index, lt.ParseMap2Items(), lt.GetMapCount());

				//shift
				static int arg_shift_value = 0;
				AddLeftLabel("shift", u8"�������е�С��λ��");
				ImGui::SliderInt("##shift", &arg_shift_value, ARG_SHIFT_MIN_VALUE, ARG_SHIFT_MAX_VALUE, "%d");

				//ִ�� rectangle()
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) && arg_input_index != -1)
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

						AddLogger(LogType::Info, "rectangle() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						AddLogger(e, "rectangle() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"���ƾ��η���");

			//circle()
			if (ImGui::TreeNode(u8"circle(mpds_ddd)"))
			{
				//input/output
				static int arg_input_index = -1;
				AddViewerCombo("input/output", &arg_input_index, true);

				//center
				static int arg_center_value[2] = {};
				AddLeftLabel("center", u8"���ĵ�����");
				ImGui::DragInt2("##center", arg_center_value, 1.0f, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE, "%d");

				//radius
				static int arg_radius_value = 5;
				AddLeftLabel("radius", u8"Բ�뾶");
				ImGui::SliderInt("##radius", &arg_radius_value, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE * 0.5, "%d");

				//color
				static ImVec4 color;
				AddLeftLabel("color", u8"������ɫ");
				ImGui::ColorEdit4("##color", (float*)&color);

				//thickness
				static int arg_thickness_value = 1;
				AddLeftLabel("thickness", u8"������ϸ��FILLED���ͱ�ʾ�ú����������һ��������");
				ImGui::SliderInt("##thickness", &arg_thickness_value, ARG_THICK_MIN_VALUE, ARG_THICK_MAX_VALUE, "%d");

				//lineType
				static int arg_lineType_index = 2;
				AddLeftLabel("lineType", u8"��������");
				ImGui::Combo("##lineType", &arg_lineType_index, lt.ParseMap2Items(), lt.GetMapCount());

				//shift
				static int arg_shift_value = 0;
				AddLeftLabel("shift", u8"��������Ͱ뾶ֵ�е�С��λ��");
				ImGui::SliderInt("##shift", &arg_shift_value, ARG_SHIFT_MIN_VALUE, ARG_SHIFT_MAX_VALUE, "%d");

				//ִ��
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);

					try
					{
						circle(input_viewer->GetMat(), Point(arg_center_value[0], arg_center_value[1]), arg_radius_value,
							Scalar(color.z * 255, color.y * 255, color.x * 255, color.w * 255), arg_thickness_value,
							lt.ParseIndex2Enum(arg_lineType_index), arg_shift_value);

						input_viewer->UpdateMat();
						input_viewer->is_open = true;

						AddLogger(LogType::Info, "circle() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						AddLogger(e, "circle() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
				}
				ImGui::TreePop();
			}
			AddMarker(u8"����Բ�η���");

			//ellipse()
			if (ImGui::TreeNode(u8"ellipse(mpsfffs_ddd)"))
			{
				//input/output
				static int arg_input_index = -1;
				AddViewerCombo("input/output", &arg_input_index, true);

				//center
				static int arg_center_value[2] = {};
				AddLeftLabel("center", u8"���ĵ�����");
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
				AddLeftLabel("color", u8"������ɫ");
				ImGui::ColorEdit4("##color", (float*)&color);

				//thickness
				static int arg_thickness_value = 1;
				AddLeftLabel("thickness", u8"������ϸ��FILLED���ͱ�ʾ�ú����������һ��������");
				ImGui::SliderInt("##thickness", &arg_thickness_value, ARG_THICK_MIN_VALUE, ARG_THICK_MAX_VALUE, "%d");

				//lineType
				static int arg_lineType_index = 2;
				AddLeftLabel("lineType", u8"��������");
				ImGui::Combo("##lineType", &arg_lineType_index, lt.ParseMap2Items(), lt.GetMapCount());

				//shift
				static int arg_shift_value = 0;
				AddLeftLabel("shift", u8"�����������ֵ��С��λ��");
				ImGui::SliderInt("##shift", &arg_shift_value, ARG_SHIFT_MIN_VALUE, ARG_SHIFT_MAX_VALUE, "%d");

				//ִ��
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) && arg_input_index != -1)
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

						AddLogger(LogType::Info, "ellipse() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						AddLogger(e, "ellipse() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
				}
				ImGui::TreePop();
			}
			AddMarker(u8"���ƻ��λ���Բ����");

			//drawMarker()
			if (ImGui::TreeNode(u8"drawMarker(mps_dddd)"))
			{
				//input/output
				static int arg_input_index = -1;
				AddViewerCombo("input/output", &arg_input_index, true);

				//position
				static int arg_position_value[2] = {};
				AddLeftLabel("position", u8"λ������");
				ImGui::DragInt2("##position", arg_position_value, 1.0f, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE, "%d");

				//color
				static ImVec4 color;
				AddLeftLabel("color", u8"������ɫ");
				ImGui::ColorEdit4("##color", (float*)&color);

				//markerType
				static int arg_markerType_index = 0;
				AddLeftLabel("markerType", u8"�������");
				ImGui::Combo("##markerType", &arg_markerType_index, mt.ParseMap2Items(), mt.GetMapCount());

				//markerSize
				static int arg_markerSize_value = 20;
				AddLeftLabel("markerSize", u8"��Ǵ�С");
				ImGui::SliderInt("##markerSize", &arg_markerSize_value, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE * 0.5, "%d");

				//thickness
				static int arg_thickness_value = 1;
				AddLeftLabel("thickness", u8"������ϸ��FILLED���ͱ�ʾ�ú����������һ��������");
				ImGui::SliderInt("##thickness", &arg_thickness_value, ARG_THICK_MIN_VALUE, ARG_THICK_MAX_VALUE, "%d");

				//lineType
				static int arg_lineType_index = 2;
				AddLeftLabel("lineType", u8"��������");
				ImGui::Combo("##lineType", &arg_lineType_index, lt.ParseMap2Items(), lt.GetMapCount());

				//ִ��
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);

					try
					{
						drawMarker(input_viewer->GetMat(), Point(arg_position_value[0], arg_position_value[1]),
							Scalar(color.z * 255, color.y * 255, color.x * 255, color.w * 255), mt.ParseIndex2Enum(arg_markerType_index),
							arg_markerSize_value, arg_thickness_value, lt.ParseIndex2Enum(arg_lineType_index));

						input_viewer->UpdateMat();
						input_viewer->is_open = true;

						AddLogger(LogType::Info, "drawMarker() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						AddLogger(e, "drawMarker() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
				}
				ImGui::TreePop();
			}
			AddMarker(u8"���Ʊ�Ƿ���");

			//putText()
			if (ImGui::TreeNode(u8"putText(mtpdfs_ddb)"))
			{
				static EnumParser<HersheyFonts> hf;		//HersheyFonts Enum

				//input/output
				static int arg_input_index = -1;
				AddViewerCombo("input/output", &arg_input_index, true);

				//text
				static char arg_text_value[64] = "";
				AddLeftLabel("text", u8"Ҫ���Ƶ��ı��ַ���");
				ImGui::InputText("##text", arg_text_value, IM_ARRAYSIZE(arg_text_value));

				//org
				static int arg_org_value[2] = {};
				AddLeftLabel("org", u8"ͼ�����ı��ַ��������½�");
				ImGui::DragInt2("##org", arg_org_value, 1.0f, ARG_PT_MIN_VALUE, ARG_PT_MAX_VALUE, "%d");

				//fontFace
				static int arg_fontFace_index = 0;
				AddLeftLabel("fontFace", u8"��������");
				ImGui::Combo("##fontFace", &arg_fontFace_index, hf.ParseMap2Items(), hf.GetMapCount());

				//fontScale
				static float arg_fontScale_value = 1.0;
				AddLeftLabel("fontScale", u8"����������ӳ����ض�������Ļ�����С");
				ImGui::SliderFloat("##fontScale", &arg_fontScale_value, 0.5, 10.0, "%.1f");

				//color
				static ImVec4 color;
				AddLeftLabel("color", u8"������ɫ");
				ImGui::ColorEdit4("##color", (float*)&color);

				//thickness
				static int arg_thickness_value = 1;
				AddLeftLabel("thickness", u8"������ϸ");
				ImGui::SliderInt("##thickness", &arg_thickness_value, 1, ARG_THICK_MAX_VALUE, "%d");

				//lineType
				static int arg_lineType_index = 2;
				AddLeftLabel("lineType", u8"��������");
				ImGui::Combo("##lineType", &arg_lineType_index, lt.ParseMap2Items(), lt.GetMapCount());

				//bottomLeftOrigin
				static bool arg_bottomLeftOrigin_value = false;
				AddLeftLabel("BLOrigin", u8"���Ϊtrue����ͼ������ԭ��λ�����½ǡ�������λ�����Ͻǡ�");
				ImGui::Checkbox("bottomLeftOrigin", &arg_bottomLeftOrigin_value);

				//ִ��
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) && arg_input_index != -1)
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

						AddLogger(LogType::Info, "putText() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						AddLogger(e, "putText() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					input_viewer = NULL;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"�����ı�����");
		}

		//ͼ��ֱ��ͼ����
		if (ImGui::CollapsingHeader(u8"ͼ��ֱ��ͼ����"))
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

				//ִ��
				if (ImGui::Button("exce", ImVec2(ImGui::GetContentRegionAvail().x, BTN_HEIGHT)) && arg_input_index != -1)
				{
					MatViewer *input_viewer = MatViewerManager::Instance().GetViewer(arg_input_index);
					MatViewer *output_viewer = MatViewerManager::Instance().GetViewer(arg_output_index, input_viewer->GetNextTitle("drawHist()"));
					Mat result;
					try
					{
						result = drawCalcHist(input_viewer->GetMat());
						output_viewer->LoadMat(result);
						input_viewer->is_open = true;

						AddLogger(LogType::Info, "calcHist() succeeded: %s\n", input_viewer->GetTitle());
					}
					catch (Exception e)
					{
						AddLogger(e, "calcHist() error: %s", input_viewer->GetTitle());
						MatViewerManager::Instance().RemoveViewer(input_viewer);
					}
					result.release();
					input_viewer = NULL;
					output_viewer = NULL;
					if (arg_output_index == items_count) arg_output_index = -1;
				}

				ImGui::TreePop();
			}
			AddMarker(u8"����ֱ��ͼ����");

			//equalizeHist()
			if (ImGui::TreeNode(u8"equalizeHist()"))
			{
				static int arg_input_index = -1;
				AddViewerCombo("input", &arg_input_index, true, "8λ��ͨ������������");

				ImGui::TreePop();
			}
			//equalizeHist
		}

		//��ֵͼ�����
		if (ImGui::CollapsingHeader(u8"��ֵͼ�����"))
		{
			ImGui::Text(u8"����� ...");

		}

		//ͼ����̬ѧ
		if (ImGui::CollapsingHeader(u8"ͼ����̬ѧ����"))
		{
			ImGui::Text(u8"����� ...");
		}
		
	}

	//end
	ImGui::End();
}
