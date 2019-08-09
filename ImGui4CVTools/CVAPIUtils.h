#pragma once

#include "CommDlg.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "cvImGuiConfig.h"

// Enum to Char
// Example：char *str = Enum2CharPtr(ABC);
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
};

EnumParser<NormTypes>::EnumParser()
{
	enum_map["NORM_INF"] = NormTypes::NORM_INF;
	enum_map["NORM_L1"] = NormTypes::NORM_L1;
	enum_map["NORM_L2"] = NormTypes::NORM_L2;
	enum_map["NORM_L2SQR"] = NormTypes::NORM_L2SQR;
	enum_map["NORM_HAMMING"] = NormTypes::NORM_HAMMING;
	enum_map["NORM_HAMMING2"] = NormTypes::NORM_HAMMING2;
	enum_map["NORM_TYPE_MASK"] = NormTypes::NORM_TYPE_MASK;
	enum_map["NORM_RELATIVE"] = NormTypes::NORM_RELATIVE;
	enum_map["NORM_MINMAX"] = NormTypes::NORM_MINMAX;
};

EnumParser<LineTypes>::EnumParser()
{
	enum_map["FILLED"] = LineTypes::FILLED;
	enum_map["LINE_4"] = LineTypes::LINE_4;
	enum_map["LINE_8"] = LineTypes::LINE_8;
	enum_map["LINE_AA"] = LineTypes::LINE_AA;
}

EnumParser<MarkerTypes>::EnumParser()
{
	enum_map["MARKER_CROSS"] = MarkerTypes::MARKER_CROSS;
	enum_map["MARKER_TILTED_CROSS"] = MarkerTypes::MARKER_TILTED_CROSS;
	enum_map["MARKER_STAR"] = MarkerTypes::MARKER_STAR;
	enum_map["MARKER_DIAMOND"] = MarkerTypes::MARKER_DIAMOND;
	enum_map["MARKER_SQUARE"] = MarkerTypes::MARKER_SQUARE;
	enum_map["MARKER_TRIANGLE_UP"] = MarkerTypes::MARKER_TRIANGLE_UP;
	enum_map["MARKER_TRIANGLE_DOWN"] = MarkerTypes::MARKER_TRIANGLE_DOWN;
}

EnumParser<InterpolationFlags>::EnumParser()
{
	enum_map["INTER_NEAREST"] = InterpolationFlags::INTER_NEAREST;
	enum_map["INTER_LINEAR"] = InterpolationFlags::INTER_LINEAR;
	enum_map["INTER_CUBIC"] = InterpolationFlags::INTER_CUBIC;
	enum_map["INTER_AREA"] = InterpolationFlags::INTER_AREA;
	enum_map["INTER_LANCZOS4"] = InterpolationFlags::INTER_LANCZOS4;
	enum_map["INTER_LINEAR_EXACT"] = InterpolationFlags::INTER_LINEAR_EXACT;
	enum_map["INTER_MAX"] = InterpolationFlags::INTER_MAX;
	enum_map["WARP_FILL_OUTLIERS"] = InterpolationFlags::WARP_FILL_OUTLIERS;
	enum_map["WARP_INVERSE_MAP"] = InterpolationFlags::WARP_INVERSE_MAP;
}

EnumParser<HersheyFonts>::EnumParser()
{
	enum_map["FONT_HERSHEY_SIMPLEX"] = HersheyFonts::FONT_HERSHEY_SIMPLEX;
	enum_map["FONT_HERSHEY_PLAIN"] = HersheyFonts::FONT_HERSHEY_PLAIN;
	enum_map["FONT_HERSHEY_DUPLEX"] = HersheyFonts::FONT_HERSHEY_DUPLEX;
	enum_map["FONT_HERSHEY_COMPLEX"] = HersheyFonts::FONT_HERSHEY_COMPLEX;
	enum_map["FONT_HERSHEY_TRIPLEX"] = HersheyFonts::FONT_HERSHEY_TRIPLEX;
	enum_map["FONT_HERSHEY_COMPLEX_SMALL"] = HersheyFonts::FONT_HERSHEY_COMPLEX_SMALL;
	enum_map["FONT_HERSHEY_SCRIPT_SIMPLEX"] = HersheyFonts::FONT_HERSHEY_SCRIPT_SIMPLEX;
	enum_map["FONT_HERSHEY_SCRIPT_COMPLEX"] = HersheyFonts::FONT_HERSHEY_SCRIPT_COMPLEX;
	enum_map["FONT_ITALIC"] = HersheyFonts::FONT_ITALIC;

}

EnumParser<ColormapTypes>::EnumParser()
{
	enum_map["COLORMAP_AUTUMN"] = ColormapTypes::COLORMAP_AUTUMN;
	enum_map["COLORMAP_BONE"] = ColormapTypes::COLORMAP_BONE;
	enum_map["COLORMAP_JET"] = ColormapTypes::COLORMAP_JET;
	enum_map["COLORMAP_WINTER"] = ColormapTypes::COLORMAP_WINTER;
	enum_map["COLORMAP_RAINBOW"] = ColormapTypes::COLORMAP_RAINBOW;
	enum_map["COLORMAP_OCEAN"] = ColormapTypes::COLORMAP_OCEAN;
	enum_map["COLORMAP_SUMMER"] = ColormapTypes::COLORMAP_SUMMER;
	enum_map["COLORMAP_SPRING"] = ColormapTypes::COLORMAP_SPRING;
	enum_map["COLORMAP_COOL"] = ColormapTypes::COLORMAP_COOL;
	enum_map["COLORMAP_HSV"] = ColormapTypes::COLORMAP_HSV;
	enum_map["COLORMAP_PINK"] = ColormapTypes::COLORMAP_PINK;
	enum_map["COLORMAP_HOT"] = ColormapTypes::COLORMAP_HOT;
	enum_map["COLORMAP_PARULA"] = ColormapTypes::COLORMAP_PARULA;
	enum_map["COLORMAP_MAGMA"] = ColormapTypes::COLORMAP_MAGMA;
	enum_map["COLORMAP_INFERNO"] = ColormapTypes::COLORMAP_INFERNO;
	enum_map["COLORMAP_PLASMA"] = ColormapTypes::COLORMAP_PLASMA;
	enum_map["COLORMAP_VIRIDIS"] = ColormapTypes::COLORMAP_VIRIDIS;
	enum_map["COLORMAP_CIVIDIS"] = ColormapTypes::COLORMAP_CIVIDIS;
	enum_map["COLORMAP_TWILIGHT"] = ColormapTypes::COLORMAP_TWILIGHT;
	enum_map["COLORMAP_TWILIGHT_SHIFTED"] = ColormapTypes::COLORMAP_TWILIGHT_SHIFTED;
}

EnumParser<BorderTypes>::EnumParser()
{
	enum_map["BORDER_CONSTANT"] = BorderTypes::BORDER_CONSTANT;
	enum_map["BORDER_REPLICATE"] = BorderTypes::BORDER_REPLICATE;
	enum_map["BORDER_REFLECT"] = BorderTypes::BORDER_REFLECT;
	enum_map["BORDER_WRAP"] = BorderTypes::BORDER_WRAP;
	enum_map["BORDER_REFLECT_101"] = BorderTypes::BORDER_REFLECT_101;
	enum_map["BORDER_TRANSPARENT"] = BorderTypes::BORDER_TRANSPARENT;
	enum_map["BORDER_REFLECT101"] = BorderTypes::BORDER_REFLECT101;
	enum_map["BORDER_DEFAULT"] = BorderTypes::BORDER_DEFAULT;
	enum_map["BORDER_ISOLATED"] = BorderTypes::BORDER_ISOLATED;
}

//选择图片文件
//@param window parent window
//@param filename output image file path
static bool SelectImageFile(GLFWwindow *window, char *filename)
{
	TCHAR szBuffer[MAX_PATH] = { 0 };

	//open file name
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = glfwGetWin32Window(window);
	ofn.lpstrFilter = _T("Image文件(*.jpg)\0*.jpg\0Image文件(*.png)\0*.png\0所有文件(*.*)\0*.*\0");	//要选择的文件后缀   
	ofn.lpstrInitialDir = _T("D:\\Program Files");							//默认的文件路径   
	ofn.lpstrFile = szBuffer;												//存放文件的缓冲区   
	ofn.nMaxFile = sizeof(szBuffer) / sizeof(*szBuffer);
	ofn.nFilterIndex = 0;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;		//标志如果是多选要加上OFN_ALLOWMULTISELECT  
	BOOL bSel = GetOpenFileName(&ofn);

	if (bSel)
	{
		//TCHAR to char*
		int iLength;
		char path[MAX_PATH] = "";

		iLength = WideCharToMultiByte(CP_ACP, 0, szBuffer, -1, NULL, 0, NULL, NULL);	//获取字节长度 
		WideCharToMultiByte(CP_ACP, 0, szBuffer, -1, filename, iLength, NULL, NULL);	//将tchar值赋给char 
	}

	return bSel == 1;
}

//获取系统当前时间
//@param buffer 返回 time 字符
//@param bsize buffer大小
//@param format 返回 time 字符格式
static void GetCurrentForamtTime(char *buffer, size_t bsize, const char* format)
{
	struct tm n_tm;
	time_t n_now = time(0);
	localtime_s(&n_tm, &n_now);

	strftime(buffer, bsize, format, &n_tm);
}


static Mat drawCalcHist(Mat input)
{
	const int channels[1] = { 0 };
	const int bins[1] = { 256 };
	float hranges[2] = { 0, 255 };
	const float* ranges[1] = { hranges };
	int dims = input.channels();

	// 显示直方图
	int hist_w = 512;
	int hist_h = 256;
	int bin_w = cvRound((double)hist_w / bins[0]);
	Mat histImage = Mat::zeros(hist_h, hist_w, CV_8UC3);

	if (dims == 3)
	{
		vector<Mat> bgr_plane;
		split(input, bgr_plane);

		Mat b_hist, g_hist, r_hist;

		// 计算Blue, Green, Red通道的直方图
		calcHist(&bgr_plane[0], 1, 0, Mat(), b_hist, 1, bins, ranges);
		calcHist(&bgr_plane[1], 1, 0, Mat(), g_hist, 1, bins, ranges);
		calcHist(&bgr_plane[2], 1, 0, Mat(), r_hist, 1, bins, ranges);

		// 归一化直方图数据
		normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
		normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
		normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

		// 绘制直方图曲线
		for (int i = 1; i < bins[0]; i++)
		{
			line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(b_hist.at<float>(i - 1))),
				Point(bin_w*(i), hist_h - cvRound(b_hist.at<float>(i))), Scalar(255, 0, 0), 1, 8, 0);

			line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(g_hist.at<float>(i - 1))),
				Point(bin_w*(i), hist_h - cvRound(g_hist.at<float>(i))), Scalar(0, 255, 0), 1, 8, 0);

			line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(r_hist.at<float>(i - 1))),
				Point(bin_w*(i), hist_h - cvRound(r_hist.at<float>(i))), Scalar(0, 0, 255), 1, 8, 0);
		}

		return histImage;
	}
	else
	{
		Mat hist;
		// 计算单通道的直方图
		calcHist(&input, 1, 0, Mat(), hist, 1, bins, ranges);

		// 归一化直方图数据
		normalize(hist, hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

		// 绘制直方图曲线
		for (int i = 1; i < bins[0]; i++) 
		{
			line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(hist.at<float>(i - 1))),
				Point(bin_w*(i), hist_h - cvRound(hist.at<float>(i))), Scalar(255, 255, 255), 1, 8, 0);
		}

		return histImage;
	}
}