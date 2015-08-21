#include "retro_filter.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <time.h>

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

void RetroFilter::applyToVideo(const Mat& frame, Mat& retroFrame, int flag)
{
    int col, row;
    Mat luminance;
    cvtColor(frame, luminance, CV_BGR2GRAY);

    // Add scratches
    Scalar meanColor = mean(luminance.row(luminance.rows / 2));
    Mat scratchColor(params_.frameSize, CV_8UC1, meanColor * 2.0);
    int x = rng_.uniform(0, params_.scratches.cols - luminance.cols);
    int y = rng_.uniform(0, params_.scratches.rows - luminance.rows);

    Rect rect(x, y, luminance.cols, luminance.rows);
    Mat qqq = params_.scratches(rect);
    luminance.setTo(meanColor * 2.0, qqq);

    // Add fuzzy border
    Mat borderColor(params_.frameSize, CV_32FC1, Scalar::all(meanColor[0] * 1.5));
    alphaBlend(borderColor, luminance, params_.fuzzyBorder);


    // Apply sepia-effect
    retroFrame.create(luminance.size(), CV_8UC3);

    switch (flag)
    {
    case 49:
        {
            Mat a,b,c;
            a.create( luminance.size(),CV_8UC1);
            a.setTo(19);
            b.create( luminance.size(),CV_8UC1);
            b.setTo(78);
            c.create( luminance.size(),CV_8UC1);
            c = luminance * hsvScale_ + hsvOffset_;

            std::vector<Mat> mat;
            mat.push_back(a);
            mat.push_back(b);
            mat.push_back(c);
            cv::merge(mat,retroFrame);
            cv::cvtColor(retroFrame,retroFrame,COLOR_HSV2BGR);
            break;
        }
    case 50:
        {
            std::vector<Mat> mat;
            mat.push_back(luminance);
            mat.push_back(luminance);
            mat.push_back(luminance);
            cv::merge(mat,retroFrame);
            break;
        }
    case 51:
        {
            std::vector<Mat> mat;
            mat.push_back(luminance);
            mat.push_back(luminance);
            mat.push_back(luminance);
            cv::merge(mat,retroFrame);
            cv::cvtColor(retroFrame,retroFrame,COLOR_HSV2BGR);
            break;
        }
    }
}
