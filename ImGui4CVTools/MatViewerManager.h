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


	//��ȡָ������ͼ����
	//MatViewer* GetViewer(int index);
	//MatViewer* GetViewer(const char *title);

	//�����⣿������
	//������������⣬��ȡָ������ͼ�������ȸ����������ң���α������
	//���ָ������ͼ���󲻴��ڣ����� title ����һ���µ���ͼ���󣬲�����
	MatViewer* GetViewer(int index, const char *title = NULL);

	bool HasViewer(const char *title);

	int GetCount();

private:
	MatViewerManager() = default;
	vector<MatViewer*> viewers;

};

