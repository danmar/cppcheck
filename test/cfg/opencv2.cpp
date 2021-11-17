
// Test library configuration for opencv2.cfg
//
// Usage:
// $ cppcheck --check-library --library=cairo --enable=information --error-exitcode=1 --inline-suppr --suppress=missingIncludeSystem test/cfg/opencv2.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <iostream>
#include <opencv2/opencv.hpp>


void validCode(char* argStr)
{
    cv::Mat image;
    image = cv::imread(argStr, cv::IMREAD_COLOR);
    if (!image.data) {
        printf("No image data \n");
        return;
    }
    cv::namedWindow("Display Image", cv::WINDOW_AUTOSIZE);
    cv::imshow("Display Image", image);
    cv::waitKey(0);

    cv::String cvStr("Hello");
    cvStr += " World";
    std::cout << cvStr;

    char * pBuf = (char *)cv::fastMalloc(20);
    cv::fastFree(pBuf);
}

void ignoredReturnValue()
{
    // cppcheck-suppress ignoredReturnValue
    cv::imread("42.png");
}

void memleak()
{
    char * pBuf = (char *)cv::fastMalloc(1000);
    std::cout << pBuf;
    // cppcheck-suppress memleak
}
