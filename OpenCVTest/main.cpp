//AL.
//TODO
//Limit image input types (You don't even know how the flags operate - yet.) CV_8UC3 ?!?!?!
//Alpha processing 
//Blur?
//Scale?
//Clean up the filthy RNG
//Revise all the funky casts
//Write meta to files
//refactor LoadImages()
//Save image via hotkey
//Improve config format (key/val pairs!)
//Import images in a more robust manner
//Make a fookin GUI, man... 

#include "Definitions.h"
#include <random>


int gl::ShowImageInWindow(const Mat& image, const char* windowName/*=""*/, const int showTime/*= 0*/, const int windowFlags /*=WINDOW_AUTOSIZE*/)
{
  namedWindow(windowName, windowFlags); // Create a window for display.
  imshow(windowName, image);						// Show our image inside it.
  return waitKeyEx(showTime);           // Wait until key input or time runs out. Return key pressed.
}


void gl::ApplyColourShiftsToPixel(const TransformationValues& transvals, Pixel& p)
{
  p.r = (p.r + transvals.r_shift > 0) && (p.r + transvals.r_shift <= MAX_RGBA_VAL) ? p.r + transvals.r_shift : 0;
  p.g = (p.g + transvals.g_shift > 0) && (p.g + transvals.g_shift <= MAX_RGBA_VAL) ? p.g + transvals.g_shift : 0;
  p.b = (p.b + transvals.b_shift > 0) && (p.b + transvals.b_shift <= MAX_RGBA_VAL) ? p.b + transvals.b_shift : 0;
}


bool gl::IsPointWithinRect(Point& point, Rect& rect)
{
  if (point.x < rect.x)
  {
    return false;
  }

  if (point.y < rect.y)
  {
    return false;
  }

  if (point.x > rect.width - 1)
  {
    return false;
  }

  if (point.y > rect.height - 1)
  {
    return false;
  }

  return true;
}


void gl::ApplyTransformationToCanvas(Mat& image, TransformationValues& transVals, Mat& canvas)
{
  Rect canvasRect = { 0, 0, canvas.cols, canvas.rows };

  for (int x = transVals.x_pos_start; x < transVals.x_pos_end; ++x)
  {
    for (int y = transVals.y_pos_start; y < transVals.y_pos_end; ++y)
    {
      Point shiftedPoint = { x + transVals.x_pos_start_offset, y + transVals.y_pos_start_offset };
      if (IsPointWithinRect(shiftedPoint, canvasRect))
      {
        Pixel p = image.ptr<Pixel>(y)[x];

        ApplyColourShiftsToPixel(transVals, p);

        canvas.ptr<Pixel>(shiftedPoint.y)[shiftedPoint.x] = p;
      }
    }
  }
}


void gl::TrimBounds(Size& imageSize, TransformationValues& transVals)
{
  //Commented out the < 0 checks cos at time of writing, we got the vals from RNG with lowerbound = 0
  //transVals.x_pos_start = transVals.x_pos_start < 0 ? 0 : transVals.x_pos_start;
  //transVals.y_pos_start = transVals.y_pos_start < 0 ? 0 : transVals.y_pos_start;

  transVals.x_pos_end = transVals.x_pos_end > imageSize.width ? imageSize.width : transVals.x_pos_end;
  transVals.y_pos_end = transVals.y_pos_end >  imageSize.height ? imageSize.height : transVals.y_pos_end;
}


//Are we leaving these global? Perhaps an RNG class? 
random_device seed;
mt19937 gen(seed());
template <typename T>
T gl::GetRandom(const T lowerBound, const T upperBound)
{
  const uniform_real_distribution<float> real_dist(static_cast<float>(lowerBound), static_cast<float>(upperBound));
  return static_cast<T>(real_dist(gen));
}


void gl::GetTransformationValues(Size& imageSize, const GlitchParameters& params, TransformationValues& transVals)
{

  //Get starting x pos 
  transVals.x_pos_start = GetRandom(0, imageSize.width - 1);
                          
  //Get starting y pos    
  transVals.y_pos_start = GetRandom(0, imageSize.height - 1);
                          
  //Get boxed length      
  const int x_range_pix = static_cast<int>(imageSize.width * GetRandom(params.x_size_min, params.x_size_max));
                          
  //Get boxed height      
  const int y_range_pix = static_cast<int>(imageSize.height * GetRandom(params.y_size_min, params.y_size_max));

  //Get x offsets for paint position on actual canvas.
  transVals.x_pos_start_offset = static_cast<int>(imageSize.width * GetRandom(params.x_shift_min, params.x_shift_max));

  //Get y offsets for paint position on actual canvas.
  transVals.y_pos_start_offset = static_cast<int>(imageSize.height *  GetRandom(params.y_shift_min, params.y_shift_max));

  //Calculate right edge of box
  transVals.x_pos_end = transVals.x_pos_start + x_range_pix;

  //Calculate bottom edge of box
  transVals.y_pos_end = transVals.y_pos_start + y_range_pix;

  TrimBounds(imageSize, transVals);

  //Calculate r shift
  transVals.r_shift = static_cast<int>(MAX_RGBA_VAL * GetRandom(params.r_shift_min, params.r_shift_max));

  //Calculate g shift
  transVals.g_shift = static_cast<int>(MAX_RGBA_VAL * GetRandom(params.g_shift_min, params.g_shift_max));

  //Calculate b shift
  transVals.b_shift = static_cast<int>(MAX_RGBA_VAL * GetRandom(params.b_shift_min, params.b_shift_max));
}


void gl::PerformSingleGlitchOnImage(Mat& image, const GlitchParameters& params, Mat& canvas)
{
  Size imageSize = Size(image.cols, image.rows);

  TransformationValues transVals;
  GetTransformationValues(imageSize, params, transVals);

  ApplyTransformationToCanvas(image, transVals, canvas);
}


void gl::GenerateRandomImageSequence(const int sequenceLength, const int upperBound, vector<int>& randomImageSequence)
{
  //AL.
  //egh, sort this out. 

  //Single image optimisation
  if (upperBound == 1)
  {
    for (int i = 0; i < sequenceLength; i++)
    {
      randomImageSequence.push_back(0);
    }
  }
  //

  const uniform_int_distribution<int> int_dist(0, upperBound-1);

  for (int i = 0; i < sequenceLength; i++)
  {
    randomImageSequence.push_back(int_dist(gen));
  }
}


void gl::GlitchMe(vector<Mat>& images, const GlitchParameters& params, Mat& canvas, const bool showProcess, const int processShowTime)
{
  vector<int> randomImageSequence;
  GenerateRandomImageSequence(params.iterations, static_cast<int>(images.size()), randomImageSequence);

  for (auto index : randomImageSequence)
  {
    PerformSingleGlitchOnImage(images[index], params, canvas);

    if (showProcess == true)
    {
      ShowImageInWindow(canvas, "", processShowTime);
    }
  }
}


void gl::CopyImage(Mat& imageSource, Mat& imageDestination)
{
  for (int x = 0; x < imageSource.cols; ++x)
  {
    for (int y = 0; y < imageSource.rows; ++y)
    {
      imageDestination.ptr<Pixel>(y)[x] = imageSource.ptr<Pixel>(y)[x];
    }
  }
}


void gl::CopyAllImagesToCanvas(vector<Mat>& images, Mat& canvas)
{
  for (auto image : images)
  {
    CopyImage(image, canvas);
  }
}


void gl::DetermineCanvasSize(Size& canvasSize, vector<Mat>& images)
{
	for (const auto image : images)
	{
		if (canvasSize.width < image.cols)
		{
			canvasSize.width = image.cols;
		}
		if (canvasSize.height < image.rows)
		{
			canvasSize.height = image.rows;
		}
	}
}


/*
char* gl::GetFileNameFromPath(char* fullPath)
{
	char* c = &fullPath[strlen(fullPath)]-1;
	while (*c != '/')
	{
		-- c;
	}
	return ++c;
}
*/


void gl::GetGlitchParameters_Config(vector<string>& configBlob, GlitchParameters& params)
{
  int i = 0;

  params.iterations = atoi(configBlob.at(++i).c_str());

  params. x_size_min = static_cast<float>(atof(configBlob.at(++i).c_str()));
  params. x_size_max = static_cast<float>(atof(configBlob.at(++i).c_str()));
  params. y_size_min = static_cast<float>(atof(configBlob.at(++i).c_str()));
  params. y_size_max = static_cast<float>(atof(configBlob.at(++i).c_str()));

  params. x_shift_min =  static_cast<float>(atof(configBlob.at(++i).c_str()));
  params. x_shift_max =  static_cast<float>(atof(configBlob.at(++i).c_str()));
  params. y_shift_min =  static_cast<float>(atof(configBlob.at(++i).c_str()));
  params. y_shift_max =  static_cast<float>(atof(configBlob.at(++i).c_str()));

  params. r_shift_min =  static_cast<float>(atof(configBlob.at(++i).c_str()));
  params. r_shift_max =  static_cast<float>(atof(configBlob.at(++i).c_str()));
  params. g_shift_min =  static_cast<float>(atof(configBlob.at(++i).c_str()));
  params. g_shift_max =  static_cast<float>(atof(configBlob.at(++i).c_str()));
  params. b_shift_min =  static_cast<float>(atof(configBlob.at(++i).c_str()));
  params. b_shift_max =  static_cast<float>(atof(configBlob.at(++i).c_str()));
}


void gl::GetGlitchParameters_Random(GlitchParameters& params)
{
  params.iterations = static_cast<int>(GetRandom(1.0f, static_cast<float>(MAX_ITERATIONS)));

  params.x_size_min = 0;
  params.x_size_max = GetRandom(params.x_size_min, 1.0f);
  params.y_size_min = 0;
  params.y_size_max = GetRandom(params.y_size_min, 1.0f);

  params.x_shift_min = -0.01f;//GetRandom(-1.0f, 1.0f);
  params.x_shift_max = 0.01f;//GetRandom(params.x_shift_min, 1.0f);
  params.y_shift_min = -0.01f;//GetRandom(-1.0f, 1.0f);
  params.y_shift_max = 0.01f;//GetRandom(params.y_shift_min, 1.0f);

  params.r_shift_min = GetRandom(-1.0f, 1.0f);
  params.r_shift_max = GetRandom(params.r_shift_min, 1.0f);
  params.g_shift_min = GetRandom(-1.0f, 1.0f);
  params.g_shift_max = GetRandom(params.g_shift_min, 1.0f);
  params.b_shift_min = GetRandom(-1.0f, 1.0f);
  params.b_shift_max = GetRandom(params.b_shift_min, 1.0f);
}


bool gl::LoadImages(const int imageCount, vector<Mat>& images)
{
  if (imageCount <= 0)
  {
    return false;
  }

  int imagesLoaded = 0;

  for (int i = 1; i <= imageCount; ++i)
  {
    char buff[MAX_FILENAME_LENGTH] = { 0 };
    _itoa_s(i, buff, 10);
    char imageName[MAX_FILENAME_LENGTH] = { 0 };
    strcat_s(imageName, kImagePathPrefix);
    strcat_s(imageName, buff);
    strcat_s(imageName, ".jpg");
     
    const Mat image = imread(imageName, CV_LOAD_IMAGE_UNCHANGED);

    if (image.data == nullptr)
    {
      continue;
    }

    images.push_back(image);

    ++imagesLoaded;
  }

  return (imagesLoaded == imageCount);
}


#include<fstream>
void gl::GetConfigBlob(vector<string>& configBlob)
{ 
  ifstream myFile;
  myFile.open(kConfigFileName);
  if (myFile.is_open()) 
  {
    string str;
    while (myFile.eof() == false)
    {
      getline(myFile, str);
      configBlob.push_back(str);
    }
  }
  myFile.close();
}


using namespace gl;

struct ParamPack
{
  bool randomizeParams;
  int glitchShowMin;
  int glitchShowMax;
  bool showOriginalImage;
  int origShowMin;
  int origShowMax;
  bool showProcess;
  int processShowTime;

  ParamPack(
    const bool randomizeParams = true,
    const int glitchShowMin = 2000,
    const int glitchShowMax = 2000,
    const bool showOriginalImage = true,
    const int origShowMin = 500,
    const int origShowMax = 500,
    const bool showProcess = true,
    const int processShowTime = 1
  )
    :
    randomizeParams(randomizeParams),
    glitchShowMin(glitchShowMin),
    glitchShowMax(glitchShowMax),
    showOriginalImage(showOriginalImage),
    origShowMin(origShowMin),
    origShowMax(origShowMax),
    showProcess(showProcess),
    processShowTime(processShowTime)
  {
  }

  ParamPack* operator = (ParamPack* p)
  {
    this->randomizeParams= p->randomizeParams;
    this->glitchShowMin = p->glitchShowMin;
    this->glitchShowMax = p->glitchShowMax;
    this->showOriginalImage = p->showOriginalImage;
    this->origShowMin = p->origShowMin;
    this->origShowMax = p->origShowMax;
    this->showProcess = p->showProcess;
    this->processShowTime = p->processShowTime;
    return this;
  }
};


void RunAsArt(ParamPack* pack = nullptr)
{
  if (pack == nullptr)
  {
    pack = new ParamPack();
  }

  vector<string> configBlob;
  GetConfigBlob(configBlob);

	vector<Mat> images;
  if (LoadImages(atoi(configBlob.at(0).c_str()) , images) == false)
	{
    return;
  }

	Size canvasSize(0,0);
	DetermineCanvasSize(canvasSize, images);
  Mat baseCanvas(canvasSize.height, canvasSize.width, CANVAS_TYPE);
  CopyAllImagesToCanvas(images, baseCanvas);
  
  GlitchParameters params;
  GetGlitchParameters_Config(configBlob, params);

  while(true)
  {
    Mat canvas(canvasSize.height, canvasSize.width, CANVAS_TYPE);
    CopyImage(baseCanvas, canvas);


    if (pack->randomizeParams == true)
    {
      GetGlitchParameters_Random(params);
    }
    

    if (pack->showOriginalImage == true)
    {
      const int key = ShowImageInWindow(images.front(), "", GetRandom(pack->origShowMin, pack->origShowMax));
      if (key == KEY_ESCAPE)
      {
        return;
      }
    }

    GlitchMe(images, params, canvas, pack->showProcess, pack->processShowTime);
   
    const int key = ShowImageInWindow(canvas, "", GetRandom(pack->glitchShowMin, pack->glitchShowMax));
    if (key == KEY_ESCAPE)
    {
      return;
    }
   
  }

}

bool main()
{
  ParamPack* p = nullptr;
  p = new ParamPack(true, 50, 50, false, 0, 0, true, 15);
  //p = new ParamPack(true, 1000, 1200, false, 0, 0, false);
  //p = new ParamPack(false, 1, 200, true, 800, 4000, false);

  RunAsArt(p);
  delete p;
}