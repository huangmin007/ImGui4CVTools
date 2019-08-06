#include "stdafx.h"
#include "cvImGuiConfig.h"
#include "MatViewerManager.h"

MatViewerManager &MatViewerManager::Instance()
{
	static MatViewerManager instance;
	return instance;
}

MatViewerManager::~MatViewerManager()
{
}

void MatViewerManager::AddViewer(MatViewer *viewer)
{
	viewers.push_back(viewer);
}

void MatViewerManager::RemoveViewer(MatViewer *viewer)
{

}

bool MatViewerManager::HasViewer(const char *title)
{
	for (int i = 0; i < viewers.size(); i++)
	{
		if (strlen(viewers[i]->GetTitle()) == strlen(title) && strcmp(viewers[i]->GetTitle(), title) == 0)return true;
	}

	return false;
}

void MatViewerManager::RenderViewer()
{
	for (int i = 0; i < viewers.size(); i++)
	{
		if (viewers[i] == nullptr)continue;

		if(viewers[i]->is_open)	viewers[i]->Render();
	}
}



Mat MatViewerManager::GetMat(int index)
{
	return viewers[index]->GetMat();
}

Mat MatViewerManager::GetMat(const char* title)
{
	for (int i = 0; i < viewers.size(); i++)
	{
		if (viewers[i] == nullptr)continue;

		if (strlen(viewers[i]->GetTitle()) == strlen(title) && strcmp(viewers[i]->GetTitle(), title) == 0)
		{
			return viewers[i]->GetMat();
		}
	}

	Mat temp;
	return temp;
}



MatViewer* MatViewerManager::GetViewer(int index)
{
	return viewers[index];
}

MatViewer* MatViewerManager::GetViewer(const char *title)
{
	for (int i = 0; i < viewers.size(); i++)
	{
		if (viewers[i] == nullptr)continue;

		if (strlen(viewers[i]->GetTitle()) == strlen(title) && strcmp(viewers[i]->GetTitle(), title) == 0)
		{
			return viewers[i];
		}
	}

	printf("create...\n");
	MatViewer *viewer = new MatViewer(title);
	viewers.push_back(viewer);

	return viewer;
}


const char** MatViewerManager::GetAllTitle()
{
	static const char* titles[MAX_CHAR] = {};
	for (int i = 0; i < viewers.size(); i++)
	{
		if (viewers[i] == nullptr)continue;

		titles[i] = viewers[i]->GetTitle();
	}

	return titles;
}

const char** MatViewerManager::GetAllTitle(const char* n_title)
{
	int i = 0;
	static const char* titles[MAX_CHAR] = {};
	for (; i < viewers.size(); i++)
	{
		if (viewers[i] == nullptr)continue;

		titles[i] = viewers[i]->GetTitle();
	}

	//i++;
	titles[i] = n_title;

	return titles;
}

int MatViewerManager::GetCount()
{
	return viewers.size();
}