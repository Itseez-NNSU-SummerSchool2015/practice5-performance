#include "retro_filter.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <time.h>


using namespace std;
using namespace cv;

RetroFilter::RetroFilter(const Parameters& params) :
    rng_(time(0)),
    params_(params)
{
    resize(params_.fuzzyBorder, params_.fuzzyBorder, params_.frameSize);
    params_.fuzzyBorder.convertTo(border_, CV_32F);
   
    if (params_.scratches.rows < params_.frameSize.height ||
        params_.scratches.cols < params_.frameSize.width)
    {
        resize(params_.scratches, params_.scratches, params_.frameSize);
    }

    hsvScale_ = 1;
    hsvOffset_ = 20;
}

void RetroFilter::applyToVideo(const Mat& frame, Mat& retroFrame)
{
    Mat luminance;
    cvtColor(frame, luminance, CV_BGR2GRAY);

    Scalar meanColor = mean(luminance.row(luminance.rows / 2));

    // Add scratches
    int x = rng_.uniform(0, params_.scratches.cols - luminance.cols);
    int y = rng_.uniform(0, params_.scratches.rows - luminance.rows);
    Mat mask = params_.scratches(Range(y, y + luminance.rows), Range(x, x + luminance.cols));
    luminance.setTo(meanColor * 2.0, mask);

    // Add fuzzy border
    Mat borderColor(params_.frameSize, CV_32F, Scalar::all(meanColor[0] * 1.5));
    Mat t;
    luminance.convertTo(t, CV_32F);
    multiply(borderColor - t, border_, t, 1.0 / 255.0, CV_8U);
    add(t, luminance, luminance);

    // Apply sepia-effect
    std::vector<Mat> layers;
    layers.push_back(Mat(frame.rows, frame.cols, CV_8UC1, Scalar(19)));
    layers.push_back(Mat(frame.rows, frame.cols, CV_8UC1, Scalar(78)));
    layers.push_back(luminance * hsvScale_ + hsvOffset_);

    merge(layers, retroFrame);
    cvtColor(retroFrame, retroFrame, CV_HSV2BGR);
}
