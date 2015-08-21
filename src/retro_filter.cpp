#include "retro_filter.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "time.h"

using namespace std;
using namespace cv;

inline void alphaBlend(const Mat& src, Mat& dst, const Mat& alpha)
{
    Mat w, d, s, subtr, mult;
    alpha.convertTo(w, CV_32S);
    src.convertTo(s, CV_32S);
    dst.convertTo(d, CV_32S);

	subtract(s,d,subtr);
	multiply(subtr,w,mult);
	d = (d*255 + mult)/255.0;
    d.convertTo(dst, CV_8U);
}

RetroFilter::RetroFilter(const Parameters& params) : rng_(time(0))
{
    params_ = params;

    resize(params_.fuzzyBorder, params_.fuzzyBorder, params_.frameSize);
    resize(params_.scratches, params_.scratches, params_.frameSize);

    hsvScale_ = 1;
    hsvOffset_ = 20;
}

void RetroFilter::applyToVideo(const Mat& frame, Mat& retroFrame)
{
    int col, row;
    Mat luminance;
    cvtColor(frame, luminance, CV_BGR2GRAY);

	// Add scratches
	TS(scratches);
    Scalar meanColor = mean(luminance.row(luminance.rows / 2));
    Mat scratchColor(params_.frameSize, CV_8UC1, meanColor * 2.0);
    int x = rng_.uniform(0, params_.scratches.cols - luminance.cols);
    int y = rng_.uniform(0, params_.scratches.rows - luminance.rows);

	for (row = 0; row < luminance.rows; row++)
    {
        for (col = 0; col < luminance.cols; col++)
        {
            uchar pix_color = params_.scratches.at<uchar>(row + y, col + x) ? (int)scratchColor.at<uchar>(row, col) : luminance.at<uchar>(row, col);
			luminance.at<uchar>(row, col) = pix_color;
        }
    }
	TE(scratches);

    // Add fuzzy border
	
    Mat borderColor(params_.frameSize, CV_32FC1, Scalar::all(meanColor[0] * 1.5));
	TS(alphablend);
    alphaBlend(borderColor, luminance, params_.fuzzyBorder);
	TE(alphablend);


    // Apply sepia-effect
	TS(sepia);
    retroFrame.create(luminance.size(), CV_8UC3);
    for (col = 0; col < luminance.cols; col++)
    {
        for (row = 0; row < luminance.rows; row++)
        {
			retroFrame.at<Vec3b>(row, col)[0] = 19;
            retroFrame.at<Vec3b>(row, col)[1] = 78;
            retroFrame.at<Vec3b>(row, col)[2] = cv::saturate_cast<uchar>(luminance.at<uchar>(row, col) * hsvScale_ + hsvOffset_);
        }
    }

	cvtColor(retroFrame, retroFrame, CV_HSV2BGR);
	TE(sepia);
}
