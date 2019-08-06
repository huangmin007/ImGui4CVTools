#include "stdafx.h"
#include "MatViewer.h"

#include "cvImGuiConfig.h"
#include "MatViewerManager.h"
#include "CVAPIUtils.h"

MatViewer::MatViewer()
{
	char c_time[32];
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
	texture_id = -1;
	if (!n_image.empty())
	{
		n_image.release();
		n_image = NULL;
	}
}

void MatViewer::Render()
{
	if (n_image.empty() || texture_id == -1) return;

	//ImVec2 vec2 = ImGui::GetWindowPos();
	//ImGui::SetNextWindowPos(ImVec2(vec2.x + 40, vec2.y + 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(n_image.cols, n_image.rows), ImGuiCond_FirstUseEver);

	//static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize;

	//Buttons x1.5 x1.0 x0.5
	if (ImGui::Begin(n_title, &is_open))
	{
		//ImGui::SetWindowPos(pos);

		//rate x2.0 x1.0 x0.5
		for (int i = 0; i < 6; i++)
		{
			if (i > 0)ImGui::SameLine();

			char label[8];
			sprintf_s(label, "x%.1f", rate[i]);

			if (ImGui::Button(label, btn_size))
				ImGui::SetWindowSize(ImVec2(n_image.cols * rate[i], n_image.rows * rate[i]));
		}
		ImGui::SameLine();

		//imshow
		if (ImGui::Button("imshow", ImVec2(80, BTN_HEIGHT)))
		{
			if (!n_image.empty())
			{
				imshow(n_title, n_image);
				waitKey(30);
			}
		}

		//Image
		ImGui::Image((GLuint*)texture_id, ImGui::GetContentRegionAvail());
	}

	ImGui::End();

	pos = ImGui::GetWindowPos();
	size = ImGui::GetWindowSize();

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
	return ImVec2(pos.x + 40, pos.y + 40);
}

ImVec2 MatViewer::GetViewerSize()
{
	return size;
}

void MatViewer::LoadMat(Mat mat)
{
	if (mat.empty())	return;
	if (mat.ptr() != n_image.ptr())	mat.copyTo(n_image);

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
	if (n_image.empty())return;

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