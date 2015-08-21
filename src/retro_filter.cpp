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

    // Apply sepia-effect

    TS(sepia);

    Mat h(retroFrame.size(),CV_8UC1);
    Mat s(retroFrame.size(),CV_8UC1);
    Mat v(retroFrame.size(),CV_8UC1);
    
    h = Scalar(19);
    s = Scalar(78);
    v = retroFrame * hsvScale_ + hsvOffset_;

    std::vector<cv::Mat> array_to_merge;
    array_to_merge.push_back(h);
    array_to_merge.push_back(s);
    array_to_merge.push_back(v);

    merge(array_to_merge,retroFrame);
    cvtColor(retroFrame,retroFrame,COLOR_HSV2BGR);

    TE(sepia);

}
