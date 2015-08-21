#include "retro_filter.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <ctime>

using namespace std;
using namespace cv;

inline void alphaBlend(const Mat& src, Mat& dst, const Mat& alpha)
{
    Mat w, d, s, dw, sw;
    alpha.convertTo(w, CV_32S);
    src.convertTo(s, CV_32S);
    dst.convertTo(d, CV_32S);

    multiply(s, w, sw);
    multiply(d, -w, dw);
    d = (d*255 + sw + dw)/255.0;
    d.convertTo(dst, CV_8U);
}

RetroFilter::RetroFilter(const Parameters& params) : rng_(5)
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
    int lRows = luminance.rows;
    int lCols = luminance.cols;
    // Add scratches
    Scalar meanColor = mean(luminance.row(luminance.rows / 2));
    int x = rng_.uniform(0, params_.scratches.cols - luminance.cols);
    int y = rng_.uniform(0, params_.scratches.rows - luminance.rows);
    Mat scratchRoi = params_.scratches(Rect(x,y,luminance.cols,luminance.rows));

    luminance.setTo(meanColor * 2.0, scratchRoi);
    
    // Add fuzzy border

    Mat borderColor(params_.frameSize, CV_32FC1, Scalar::all(meanColor[0] * 1.5));
    alphaBlend(borderColor, luminance, params_.fuzzyBorder);


    // Apply sepia-effect
    retroFrame.create(luminance.size(), CV_8UC3);
    Mat hsv_pixel(1, 1, CV_8UC3);
    Mat rgb_pixel(1, 1, CV_8UC3);
    
    //vector<Mat> hsvSplit;
    Mat hsvSplit[3];
    Mat hsvC2 = luminance * hsvScale_  + hsvOffset_;
    Mat hsvC0 = Mat(lRows, lCols,CV_8UC1, Scalar(19));
    Mat hsvC1 = Mat(lRows, lCols,CV_8UC1, Scalar(78));
    hsvSplit[0] = hsvC0;
    hsvSplit[1] = hsvC1;
    hsvSplit[2] = hsvC2;
    Mat hsvMat;
    merge(hsvSplit,3, hsvMat);
    cvtColor(hsvMat,retroFrame,CV_HSV2BGR);
}
