
// Test library configuration for opencv2.cfg
//
// Usage:
// $ cppcheck --check-library --library=opencv2 --enable=style,information --inconclusive --error-exitcode=1 --disable=missingInclude --inline-suppr test/cfg/opencv2.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <iostream>
#include <opencv2/opencv.hpp>


void validCode(const char* argStr)
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

    char * pBuf = (char *)cv::fastMalloc(20); // cppcheck-suppress cstyleCast
    cv::fastFree(pBuf);
}

void ignoredReturnValue()
{
    // cppcheck-suppress ignoredReturnValue
    cv::imread("42.png");
}

void memleak()
{
    char * pBuf = (char *)cv::fastMalloc(1000); // cppcheck-suppress cstyleCast
    std::cout << pBuf;
    // cppcheck-suppress memleak
}
