#ifndef GABOR_H
#define GABOR_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

using namespace std;

class Gabor
{
public:
    Gabor();
    Gabor(string name);
    Gabor(string name, double rotation, double frequency);
    Gabor(string name, double rotation, double amplitude, double frequency, double offset);

    void setName        (string);
    void setRotation    (double);
    void setAmplitude   (double);
    void setFrequency   (double);
    void setOffset      (double);

    void filter(cv::Mat &inputImage);
    void filter(cv::Mat &outputImage, cv::Mat inputImage);
    string Name();

private:
    cv::Mat kernel;

    string name;
    double rotation;
    double amplitude;
    double frequency;
    double offset;

    void calculate_kernel(int ks, double sig, double th, double lm, double ps);

};

#endif // GABOR_H
