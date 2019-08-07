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
	if (!texture.empty()) texture.release();

	n_image = NULL;
	texture = NULL;
}

void MatViewer::Render()
{
	if (n_image.empty() || texture_id == -1) return;

	ImGui::SetNextWindowPos(pos, ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(n_image.cols, n_image.rows), ImGuiCond_Once);

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
				ImGui::SetWindowSize(ImVec2(n_image.cols * rate[i], n_image.rows * rate[i]));
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

	if (!is_render)	return;

	if (n_image.type() != 16)
		cvtColor(mat, texture, COLOR_GRAY2BGR);
	else
		texture = n_image;
	
	try
	{
		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.cols, texture.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, texture.ptr());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	catch (exception e)
	{
		printf("std exception: %s", e.what());
	}

	//mat.release();
	//texture.release();
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

	if (n_image.type() != 16)
		cvtColor(n_image, texture, COLOR_GRAY2BGR);
	else
		texture = n_image;

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.cols, texture.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, texture.ptr());
	glGenerateMipmap(GL_TEXTURE_2D);
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
	//memset(nx_title, 0x00, MAX_CHAR);
	sprintf_s(nx_title, "%s -> %s", n_title, title);

	return nx_title;
}