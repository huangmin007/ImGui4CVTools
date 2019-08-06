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


/*
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
*/

/*
MatViewer* MatViewerManager::GetViewer(int index)
{
	return viewers[index];
}*/

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

	printf("create new viewer ...\n");
	MatViewer *viewer = new MatViewer(title);
	viewers.push_back(viewer);

	return viewer;
}


MatViewer* MatViewerManager::GetViewer(int index, const char *title)
{
	if (index < 0 || index >= viewers.size())
	{
		printf("create new viewer ...\n");

		MatViewer *viewer = title == NULL ? new MatViewer() : new MatViewer(title);
		viewers.push_back(viewer);

		return viewer;
	}

	return viewers[index];
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

	titles[i] = n_title;

	return titles;
}

int MatViewerManager::GetCount()
{
	return viewers.size();
}