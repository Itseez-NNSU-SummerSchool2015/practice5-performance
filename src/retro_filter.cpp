#include <time.h>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

#include "retro_filter.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <time.h>

using namespace std;
using namespace cv;

// Macros for time measurements
#if 1
  #define TS(name) int64 t_##name = getTickCount()
  #define TE(name) printf("TIMER_" #name ": %.2fms\n", \
    1000.f * ((getTickCount() - t_##name) / getTickFrequency()))
#else
  #define TS(name)
  #define TE(name)
#endif


inline void alphaBlend(const Mat& src, Mat& dst, const Mat& alpha)
{
    Mat w, d, s, dw, sw;
    alpha.convertTo(w, CV_32S);
    src.convertTo(s, CV_32S);
    dst.convertTo(d, CV_32S);

    multiply(s, w, sw);
    multiply(d, -w, dw);
    
    d += sw / 255.0 + dw / 255.0;
    d.convertTo(dst, CV_8U);
}

RetroFilter::RetroFilter(const Parameters& params) : rng_(time(0))
{
    params_ = params;

    resize(params_.fuzzyBorder, params_.fuzzyBorder, params_.frameSize);

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
    int col, row;
    Mat luminance;
    cvtColor(frame, luminance, CV_BGR2GRAY);

    // Add scratches
    Scalar meanColor = mean(luminance.row(luminance.rows / 2));
    Mat scratchColor(params_.frameSize, CV_8UC1, meanColor * 2.0);
    int x = rng_.uniform(0, params_.scratches.cols - luminance.cols);
    int y = rng_.uniform(0, params_.scratches.rows - luminance.rows);

    for (row = 0; row < luminance.size().height; row++)
    {
        for (col = 0; col < luminance.size().width; col++)
        {
            if (params_.scratches.at<uchar>(row + y, col + x))
            {
                luminance.at<uchar>(row, col) = scratchColor.at<uchar>(row, col);
            }
        }
    }

    // Add fuzzy border
    Mat borderColor(params_.frameSize, CV_32FC1, Scalar::all(meanColor[0] * 1.5));
    alphaBlend(borderColor, luminance, params_.fuzzyBorder);

    // Apply sepia-effect
    retroFrame.create(luminance.size(), CV_8UC3);
    vector<Mat> channels;
    Mat hsv(luminance.size(), CV_8UC3);
    split(hsv, channels);

    for (col = 0; col < luminance.size().width; col++)
    {
        for (row = 0; row < luminance.size().height; row++)
        {
            channels[2].at<uchar>(row, col) = cv::saturate_cast<uchar>(luminance.at<uchar>(row, col) * hsvScale_ + hsvOffset_);
            channels[0].at<uchar>(row, col) = 19;
            channels[1].at<uchar>(row, col) = 78;
        }
    }
    merge(channels, retroFrame);
    cvtColor(retroFrame, retroFrame, CV_HSV2BGR);
}
