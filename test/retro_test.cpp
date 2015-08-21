#include <gtest/gtest.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"

#include "retro_filter.hpp"

using namespace cv;
using namespace std;

#if 1
  #define TS(name) int64 t_##name = getTickCount()
  #define TE(name) printf("TIMER_" #name ": %.2fms\n", \
    1000.f * ((getTickCount() - t_##name) / getTickFrequency()))
#else
  #define TS(name)
  #define TE(name)
#endif

TEST(Matrix, matrix_can_set_zeros)
{
    
    string video_file = "testdata/05.avi";
    string border = "testdata/fuzzy_border.png";
    string scratches = "testdata/scratches.png ";

    RetroFilter::Parameters params;
    params.fuzzyBorder = imread(border, 0);
    params.scratches   = imread(scratches, 0);

    if (params.fuzzyBorder.empty())
        cout << "Error: failed to open image to use as a border: " << border << endl;
    if (params.scratches.empty())
        cout << "Error: failed to open image to use for scratches: " << scratches << endl;
    if (params.fuzzyBorder.empty() || params.scratches.empty())
        EXPECT_TRUE(false);

    VideoCapture capture;
    if (!video_file.empty())
    {
        capture.open(video_file);
        if (!capture.isOpened())
        {
            cout << "Error: failed to open video stream for: " << video_file << endl;
            EXPECT_TRUE(false);
        }
    }
    else
    {
        cout << "Error: declare a source of images" << endl;
        EXPECT_TRUE(false);
    }

    Mat frame;
    capture >> frame;

    while (frame.empty())
    {
        capture >> frame;
    }

    params.frameSize   = frame.size();
    RetroFilter filter(params);

    Mat expFrame, retroFrame;
    for(int i = 1; i < 6; i++){
        char name[100];
        sprintf(name, "%d_correct.png", i);
        TS(filter);
        filter.applyToVideo(frame, retroFrame);
        TE(filter);
        expFrame = imread(name);
        cv::Mat diff ;
        absdiff(expFrame , retroFrame,diff);
        int s = cv::sum( diff )[0];

        EXPECT_EQ(0,s);
        capture >> frame;
    }
}
