#pragma once

#include "MatViewer.h"

class MatViewerManager
{
public:
	static MatViewerManager &Instance();
	~MatViewerManager();

	void AddViewer(MatViewer *viewer);
	void RemoveViewer(MatViewer *viewer);

	//��Ⱦ��ͼ
	void RenderViewer();
	//��ȡ������ͼ�ı���
	//ÿ����ͼ�ı�����Ψһ�ģ������ظ�
	const char** GetAllTitle();
	const char** GetAllTitle(const char* n_title);

	//��ȡָ����ͼ�� Mat����
	Mat GetMat(int index);
	Mat GetMat(const char *title);

	//��ȡָ������ͼ����
	MatViewer* GetViewer(int index);
	MatViewer* GetViewer(const char *title);

	bool HasViewer(const char *title);

	int GetCount();

private:
	MatViewerManager() = default;
	vector<MatViewer*> viewers;

};

