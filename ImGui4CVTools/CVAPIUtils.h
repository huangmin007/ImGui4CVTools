#pragma once

#include "CommDlg.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "cvImGuiConfig.h"

// Enum to Char
// Example��char *str = Enum2CharPtr(ABC);
#define Enum2CharPtr(value) #value

template <typename T>
class EnumParser
{
public:
	EnumParser() {};

	//parse enum string to enum
	T ParseChar2Enum(const char* value)
	{
		map <const char*, T>::const_iterator iValue = enum_map.find(value);
		if (iValue == enum_map.end())
			throw runtime_error("not find ...");
		return iValue->second;
	}

	//parse enum to enum string
	const char* ParseEnum2Char(const T value)
	{
		map <const char*, T>::const_iterator iValue = enum_map.begin();
		for (; iValue != enum_map.end(); iValue++)
		{
			if (iValue->second == value)return iValue -> first;
		}
		
		return nullptr;
	}
	//ImGui::Combo index
	//parse combo select index to enum
	T ParseIndex2Enum(int index)
	{
		map <const char*, T>::const_iterator iValue = enum_map.begin();

		int i = 0;
		for (; iValue != enum_map.end(); iValue++, i++)
		{
			if (i == index)return iValue->second;
		}

		return (T)0;
	}

	//ImGui::Combo index
	//parse combo select index to enum string
	const char* ParseIndex2Char(int index)
	{
		map <const char*, T>::const_iterator iValue = enum_map.begin();

		int i = 0;
		for (; iValue != enum_map.end(); iValue++, i++)
		{
			if (i == index)return iValue->first;
		}

		return nullptr;
	}

	//parse map to Combo items
	const char** ParseMap2Items()
	{
		static const char* enum_chars[64] = {};
		map <const char*, T>::const_iterator iValue = enum_map.begin();

		int i = 0;
		for (; iValue != enum_map.end(); iValue++,i++)
		{
			enum_chars[i] = iValue->first;
		}

		return enum_chars;
	}

	//get map count
	size_t GetMapCount()
	{
		return enum_map.size();
	}

private:
	map <const char*, T> enum_map;
};



EnumParser<ColorConversionCodes>::EnumParser()
{
	enum_map["COLOR_BGR2GRAY"] = ColorConversionCodes::COLOR_BGR2GRAY;
	enum_map["COLOR_GRAY2BGR"] = ColorConversionCodes::COLOR_GRAY2BGR;

	enum_map["COLOR_BGR2HSV"] = ColorConversionCodes::COLOR_BGR2HSV;
	enum_map["COLOR_RGB2HSV"] = ColorConversionCodes::COLOR_RGB2HSV;

	enum_map["COLOR_HSV2BGR"] = ColorConversionCodes::COLOR_HSV2BGR;
	enum_map["COLOR_HSV2RGB"] = ColorConversionCodes::COLOR_HSV2RGB;
	
	enum_map["COLOR_BGR2YUV"] = ColorConversionCodes::COLOR_BGR2YUV;
	enum_map["COLOR_RGB2YUV"] = ColorConversionCodes::COLOR_RGB2YUV;
	enum_map["COLOR_YUV2BGR"] = ColorConversionCodes::COLOR_YUV2BGR;
	enum_map["COLOR_YUV2RGB"] = ColorConversionCodes::COLOR_YUV2RGB;

	enum_map["COLOR_BGR2YCrCb"] = ColorConversionCodes::COLOR_BGR2YCrCb;
}


//ѡ��ͼƬ�ļ�
//@param window parent window
//@param filename output image file path
static bool SelectImageFile(GLFWwindow *window, char *filename)
{
	TCHAR szBuffer[MAX_PATH] = { 0 };

	//open file name
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = glfwGetWin32Window(window);
	ofn.lpstrFilter = _T("Image�ļ�(*.jpg)\0*.jpg\0Image�ļ�(*.png)\0*.png\0�����ļ�(*.*)\0*.*\0");	//Ҫѡ����ļ���׺   
	ofn.lpstrInitialDir = _T("D:\\Program Files");							//Ĭ�ϵ��ļ�·��   
	ofn.lpstrFile = szBuffer;												//����ļ��Ļ�����   
	ofn.nMaxFile = sizeof(szBuffer) / sizeof(*szBuffer);
	ofn.nFilterIndex = 0;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;		//��־����Ƕ�ѡҪ����OFN_ALLOWMULTISELECT  
	BOOL bSel = GetOpenFileName(&ofn);

	if (bSel)
	{
		//TCHAR to char*
		int iLength;
		char path[MAX_PATH] = "";

		iLength = WideCharToMultiByte(CP_ACP, 0, szBuffer, -1, NULL, 0, NULL, NULL);	//��ȡ�ֽڳ��� 
		WideCharToMultiByte(CP_ACP, 0, szBuffer, -1, filename, iLength, NULL, NULL);	//��tcharֵ����char 
	}

	return bSel == 1;
}

//��ȡϵͳ��ǰʱ��
//@param buffer ���� time �ַ�
//@param bsize buffer��С
//@param format ���� time �ַ���ʽ
static void GetCurrentForamtTime(char *buffer, size_t bsize, const char* format)
{
	struct tm n_tm;
	time_t n_now = time(0);
	localtime_s(&n_tm, &n_now);

	strftime(buffer, bsize, format, &n_tm);
}