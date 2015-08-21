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

void RetroFilter::applyToVideo(const Mat& frame, Mat& retroFrame)
{
    Mat gray;
    frame.copyTo(retroFrame);
    double beta = 0.5;

    TS(scratching);
    cvtColor(frame,gray,COLOR_BGR2GRAY);
    resize(params_.scratches, retroFrame, frame.size());
    addWeighted(gray, beta, retroFrame, beta, 0.0, retroFrame);

    TE(scratching);

    // Add fuzzy border
    int col, row;
    Mat luminance;
    cvtColor(frame, luminance, CV_BGR2GRAY);

    Scalar meanColor = mean(luminance.row(luminance.rows / 2));
    TS(bording);

    Mat borderColor(params_.frameSize, CV_32FC3, Scalar::all(meanColor[0] * 1.5));
    Mat tmp;
    cvtColor(borderColor,tmp,COLOR_BGR2GRAY);
    addWeighted(gray, 0.5, retroFrame, 0.5, 0.0, retroFrame);
    resize(params_.fuzzyBorder, tmp, frame.size());
    addWeighted(tmp, 0.5, retroFrame, 0.5, 0.0, retroFrame);

    TE(bording);

    /*// Apply sepia-effect
    retroFrame.create(luminance.size(), CV_8UC3);
    Mat hsv_pixel(1, 1, CV_8UC3);
    Mat rgb_pixel(1, 1, CV_8UC3);

    TS(sepia);

    for (col = 0; col < luminance.size().width; col += 1)
    {
        for (row = 0; row < luminance.size().height; row += 1)
        {
            hsv_pixel.ptr()[2] = cv::saturate_cast<uchar>(luminance.at<uchar>(row, col) * hsvScale_ + hsvOffset_);
            hsv_pixel.ptr()[0] = 19;
            hsv_pixel.ptr()[1] = 78;

            cvtColor(hsv_pixel, rgb_pixel, CV_HSV2RGB);

            retroFrame.at<Vec3b>(row, col)[0] = rgb_pixel.ptr()[2];
            retroFrame.at<Vec3b>(row, col)[1] = rgb_pixel.ptr()[1];
            retroFrame.at<Vec3b>(row, col)[2] = rgb_pixel.ptr()[0];
        }
    }

    TE(sepia);*/
}
