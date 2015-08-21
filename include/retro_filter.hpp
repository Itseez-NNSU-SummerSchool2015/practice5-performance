#ifndef __RETRO_FILTER_H__
#define __RETRO_FILTER_H__

#include "opencv2/core/core.hpp"

// Macros for time measurements
#if 1
  #define TS(name) int64 t_##name = getTickCount()
  #define TE(name) printf("TIMER_" #name ": %.2fms\n", \
    1000.f * ((getTickCount() - t_##name) / getTickFrequency()))
#else
  #define TS(name)
  #define TE(name)
#endif

class RetroFilter
{
 public:
    struct Parameters
    {
        cv::Size frameSize;
        cv::Mat fuzzyBorder;
        cv::Mat scratches;
    };

    RetroFilter(const Parameters& params);
    virtual ~RetroFilter() {};

    void applyToVideo(const cv::Mat& frame, cv::Mat& retroFrame);

 private:
    Parameters params_;
    cv::RNG rng_;

    float hsvScale_;
    float hsvOffset_;
};

#endif //__RETRO_FILTER_H__
