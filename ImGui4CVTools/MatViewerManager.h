#pragma once

#include "MatViewer.h"

class MatViewerManager
{
public:
	static MatViewerManager &Instance();
	~MatViewerManager();

	void AddViewer(MatViewer *viewer);
	void RemoveViewer(MatViewer *viewer);

	//渲染视图
	void RenderViewer();

	//获取所以视图的标题
	//每个视图的标题是唯一的，不可重复
	const char** GetAllTitle();
	const char** GetAllTitle(const char* n_title);


	//获取指定的视图对象
	//MatViewer* GetViewer(int index);
	//MatViewer* GetViewer(const char *title);

	//有问题？？？？
	//跟据索引或标题，获取指定的视图对象，优先跟据索引查找，其次标题查找
	//如果指定的视图对象不存在，则以 title 命名一个新的视图对象，并返回
	MatViewer* GetViewer(int index, const char *title = NULL);

	bool HasViewer(const char *title);

	int GetCount();

private:
	MatViewerManager() = default;
	vector<MatViewer*> viewers;

};

