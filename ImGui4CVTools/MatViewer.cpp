#include "stdafx.h"
#include "MatViewer.h"

#include "cvImGuiConfig.h"
#include "MatViewerManager.h"
#include "CVAPIUtils.h"

MatViewer::MatViewer()
{
	GetCurrentForamtTime(n_title, 32, "[%X]");
}

MatViewer::MatViewer(const char *title)
{
	memset(n_title, 0x00, MAX_CHAR);
	memcpy(n_title, title, strlen(title));
}

MatViewer::MatViewer(const char *title, Mat image):MatViewer(title)
{
	LoadMat(image);
}


MatViewer::~MatViewer()
{
	printf("clear...\n");
	is_open = false;

	texture_id = -1;
	memset(n_title, 0x00, MAX_CHAR);

	if (!n_image.empty()) n_image.release();
	//if (!texture.empty()) texture.release();

	n_image = NULL;
	//texture = NULL;
}

void MatViewer::Render()
{
	if (n_image.empty() || texture_id == -1) return;

	ImGui::SetNextWindowPos(pos, ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(n_image.cols + 18, n_image.rows + 65), ImGuiCond_Once);

	//static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize;
	
	//Buttons x1.5 x1.0 x0.5
	if (ImGui::Begin(n_title, &is_open))
	{
		//默认按扭大小
		const ImVec2 btn_size = ImVec2(40, BTN_HEIGHT);
		//窗体允许缩放比例
		const double rate[6] = { 2.0, 1.5, 1.0, 0.8, 0.5, 0.2 };

		//render
		if (ImGui::Button(is_render ? "-" : "+", ImVec2(BTN_HEIGHT, BTN_HEIGHT)))
		{
			is_render = !is_render;
			ImGui::SetWindowSize(ImVec2(n_image.cols, is_render ? n_image.rows : 60));
		}
		ImGui::SameLine();

		//rate x2.0 x1.0 x0.5
		for (int i = 0; i < 6; i++)
		{
			if (i > 0)ImGui::SameLine();

			char label[8];
			sprintf_s(label, "x%.1f", rate[i]);

			if (ImGui::Button(label, btn_size) && is_render)
				ImGui::SetWindowSize(ImVec2(n_image.cols * rate[i] + 18 , n_image.rows * rate[i] + 65));
		}
		ImGui::SameLine();

		//codes
		if (ImGui::Button("codes", ImVec2(70, BTN_HEIGHT)))
		{

		}
		ImGui::SameLine();

		//imshow
		if (ImGui::Button("imshow", ImVec2(70, BTN_HEIGHT)))
		{
			if (!n_image.empty())
			{
				imshow(n_title, n_image);
				waitKey(30);
			}
		}

		//Image
		if(is_render)
			ImGui::Image((GLuint*)texture_id, ImGui::GetContentRegionAvail());
	}

	pos = ImGui::GetWindowPos();
	ImGui::End();
}

void MatViewer::SetViewerPos(ImVec2 posi)
{
	pos = posi;
}

ImVec2 MatViewer::GetViewerPos()
{
	return pos;
}
ImVec2 MatViewer::GetNextViewerPos()
{
	return ImVec2(pos.x + 30, pos.y + 30);
}

void MatViewer::LoadMat(Mat mat)
{
	if (mat.empty())	return;
	if (mat.ptr() != n_image.ptr())	mat.copyTo(n_image);

	if (is_render)
		texture_id = cvMat2glTexture(n_image);
}

void MatViewer::LoadMat(Mat mat, const char *title)
{
	memset(n_title, 0x00, MAX_CHAR);
	memcpy(n_title, title, strlen(title));

	LoadMat(mat);
}

void MatViewer::UpdateMat()
{
	if (n_image.empty() || !is_render)return;

	texture_id = cvMat2glTexture(n_image);
}

Mat& MatViewer::GetMat()
{
	return n_image;
}

const char* MatViewer::GetTitle()
{
	return n_title;
}

const char* MatViewer::GetNextTitle(const char *title)
{
	static char nx_title[MAX_CHAR];
	sprintf_s(nx_title, "%s -> %s", n_title, title);

	return nx_title;
}

GLuint MatViewer::cvMat2glTexture(const cv::Mat& img)
{
	Mat image;
	if (img.channels() == 1)
		cvtColor(img, image, COLOR_GRAY2BGR);
	else
		image = img;

	//使用快速4字节对齐（默认设置）。
	glPixelStorei(GL_UNPACK_ALIGNMENT, (image.step & 3) ? 1 : 4);

	//设置数据中一整行的长度（不需要等于image.cols）
	glPixelStorei(GL_UNPACK_ROW_LENGTH, static_cast<GLint>(image.step / image.elemSize()));

	GLenum internalformat = GL_RGB32F;
	GLenum externalformat = GL_BGR;

	if (image.channels() == 4) internalformat = GL_RGBA;
	if (image.channels() == 3) internalformat = GL_RGB;
	if (image.channels() == 2) internalformat = GL_RG;
	if (image.channels() == 1)
	{
		internalformat = GL_RED;	// GL_RED Working, GL_BLUE,GL_GREEN Not Working
		externalformat = GL_RED;	// GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT32F, GL_R32F NOT WORKING!
	}

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	//GL_LINEAR对应线性滤波, GL_NEAREST对应最近邻滤波方式.
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	try 
	{
		glTexImage2D(GL_TEXTURE_2D,
			/* level */				0,
			/* internalFormat */	internalformat,
			/* width */				image.cols,
			/* height */			image.rows,
			/* border */			0,
			/* format */			externalformat,
			/* type */				GL_UNSIGNED_BYTE, //GL_FLOAT,
			/* *data */				image.ptr());

		glGenerateMipmap(GL_TEXTURE_2D);
	}
	catch (std::exception & e)
	{
		printf("Mat 2 glTexture Error: %s.\n", e.what());
	}

	if (img.channels() == 1)
	{
		image.release();
		image = NULL;
	}

	return texture;
}
