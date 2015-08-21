#include "retro_filter.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2\highgui\highgui.hpp"
#include <time.h>
#include <vector>

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
    Mat luminance;
    cvtColor(frame, luminance, CV_BGR2GRAY);

    int x = rng_.uniform(0, params_.scratches.cols - luminance.cols);
    int y = rng_.uniform(0, params_.scratches.rows - luminance.rows);

    Scalar meanColor = mean(luminance.row(luminance.rows / 2));

    Rect pos(x, y, luminance.cols, luminance.rows);

    luminance.setTo(meanColor * 2.0, params_.scratches(pos));


    // Add fuzzy border    
    Mat borderColor(params_.frameSize, CV_32FC1, Scalar::all(meanColor[0] * 1.5));
    alphaBlend(borderColor, luminance, params_.fuzzyBorder);


    // Apply sepia-effect
    Mat v = luminance * hsvScale_ + hsvOffset_;
    Mat s(luminance.size(), CV_8UC1, Scalar(78, 0, 0));
    Mat h(luminance.size(), CV_8UC1, Scalar(19, 0, 0));

    std::vector<Mat> hsv;
    hsv.push_back(h);
    hsv.push_back(s);
    hsv.push_back(v);
    cv::merge(hsv, luminance);
    
    cvtColor(luminance, retroFrame, CV_HSV2BGR);

    


}
/*https://docs.google.com/document/d/1R8-PY-nklomHmxXBJx_6Uitn0fjtcj1n_oxPIK7Gs3M/edit?pli=1#*/