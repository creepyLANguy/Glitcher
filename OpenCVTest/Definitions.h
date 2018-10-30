#ifndef FUNCTIONDEFINITIONS_H_
#define FUNCTIONDEFINITIONS_H_


#include <opencv2/highgui/highgui.hpp>


using namespace cv;
using namespace std;


const int MAX_RGBA_VAL = 255;
const int MAX_ITERATIONS = 1000;

const char* kConfigFileName = "../img/config.txt";
const int MAX_FILENAME_LENGTH = 256;
const char* kImagePathPrefix = "../img/";

const int CANVAS_TYPE = CV_8UC3;

const int KEY_ESCAPE = 27;


struct TransformationValues
{
  int x_pos_start;
  int y_pos_start;

  int x_pos_start_offset;
  int y_pos_start_offset;

  int  x_pos_end;
  int  y_pos_end;

  int r_shift;
  int g_shift;
  int b_shift;
};


struct GlitchParameters
{
  int iterations = 0; 

  float x_size_min = 0;
  float x_size_max = 0;
  float y_size_min = 0;
  float y_size_max = 0;

  float x_shift_min = 0;
  float x_shift_max = 0;
  float y_shift_min = 0;
  float y_shift_max = 0;

  float r_shift_min = 0;
  float r_shift_max = 0;
  float g_shift_min = 0;
  float g_shift_max = 0;
  float b_shift_min = 0;
  float b_shift_max = 0;
};


struct Pixel
{
	uchar r, g, b;
};

namespace gl
{
  void GetConfigBlob(vector<string>& configBlob);

  bool LoadImages(const int imageCount, vector<Mat>& images);

  void GetGlitchParameters_Random(GlitchParameters& params);

  void GetGlitchParameters_Config(vector<string>& configBlob, GlitchParameters& params);

  //char* GetFileNameFromPath(char* fullPath);

  void DetermineCanvasSize(Size& canvasSize, vector<Mat>& images);

  void CopyAllImagesToCanvas(vector<Mat>& images, Mat& canvas);
  
  void CopyImage(Mat& imageSource, Mat& imageDestination);

  void GlitchMe(vector<Mat>& images, const GlitchParameters& params, Mat& canvas, const bool showProcess, const int processShowTime);

  void GenerateRandomImageSequence(const int sequenceLength, const int upperBound, vector<int>& randomImageSequence);

  void PerformSingleGlitchOnImage(Mat& image, const GlitchParameters& params, Mat& canvas);

  template <typename T>
  T GetRandom(const T lowerBound, const T upperBound);

  void GetTransformationValues(Size& imageSize, const GlitchParameters& params, TransformationValues& transVals);

  void TrimBounds(Size& imageSize, TransformationValues& transVals);

  void ApplyTransformationToCanvas(Mat& image, TransformationValues& transVals, Mat& canvas);

  bool IsPointWithinRect(Point& point, Rect& rect);

  void ApplyColourShiftsToPixel(const TransformationValues& transvals, Pixel& p);

  int ShowImageInWindow(const Mat& image, const char* windowName = "", const int showTime = 0, const int windowFlags = WINDOW_AUTOSIZE);
}
#endif//FUNCTIONDEFINITIONS_H_