#include "gabor.h"

Gabor::Gabor()
{
    
}

Gabor::Gabor(string name)
{
    this->name = name;

    rotation = 1;
    amplitude = 5;
    frequency = 50;
    offset = 90;
    
    int kernelSize = 21;
    kernel = cv::Mat (kernelSize, kernelSize, CV_32F);
    calculate_kernel(kernelSize, amplitude, rotation, 0.5 + frequency / 100.0, offset);
}

Gabor::Gabor(string name, double rotation, double frequency)
{
    this->name = name;
    this->rotation = rotation;
    this->frequency = frequency;

    amplitude = 5;
    offset = 90;
    
    int kernelSize = 21;
    kernel = cv::Mat (kernelSize, kernelSize, CV_32F);
    calculate_kernel(kernelSize, amplitude, rotation, 0.5 + frequency / 100.0, offset);
}

Gabor::Gabor(string name, double rotation, double amplitude, double frequency, double offset)
{
    this->name = name;
    this->rotation = rotation;
    this->amplitude = amplitude;
    this->frequency = frequency;
    this->offset = offset;
    
    int kernelSize = 21;
    kernel = cv::Mat (kernelSize, kernelSize, CV_32F);
    calculate_kernel(kernelSize, amplitude, rotation, 0.5 + frequency / 100.0, offset);
}

void Gabor::filter(cv::Mat &inputImage)
{
    inputImage.convertTo(inputImage, CV_32F, 1.0/255.0, 0.0);

    cv::filter2D(inputImage, inputImage, CV_32F, kernel);
    cv::pow(inputImage, 2.0, inputImage);

    inputImage.convertTo(inputImage, CV_8UC1, 255.0, 0.0);
}

void Gabor::filter(cv::Mat &outputImage, cv::Mat inputImage)
{
    inputImage.convertTo(inputImage, CV_32F, 1.0/255.0, 0.0);
    
    cv::filter2D(inputImage, outputImage, CV_32F, kernel);
    cv::pow(outputImage, 2.0, outputImage);
    
    outputImage.convertTo(outputImage, CV_8UC1, 255.0, 0.0);
    inputImage.release();
}

void Gabor::calculate_kernel(int ks, double sig, double th, double lm, double ps)
{
    int     hks     = (ks-1)/2;
    double  theta   = th*CV_PI/180;
    double  psi     = ps*CV_PI/180;
    double  del     = 2.0/(ks-1);
    double  lmbd    = lm;
    double  sigma   = sig/ks;

    double  x_theta;
    double  y_theta;

    for (int y = -hks; y <= hks; y++)
    {
        for (int x = -hks; x <= hks; x++)
        {
            x_theta =  x * del * cos(theta) + y * del * sin(theta);
            y_theta = -x * del * sin(theta) + y * del * cos(theta);
            kernel.at<float>(hks+y,hks+x) = (float)exp(-0.5 * (pow(x_theta, 2) + pow(y_theta, 2)) / pow(sigma, 2)) * cos(2 * CV_PI * x_theta / lmbd + psi);
        }
    }
}

string Gabor::Name()
{
    return name;
}

void Gabor::setName(string value)
{
    name = value;
}

void Gabor::setRotation(double value)
{
    rotation = value;
}

void Gabor::setAmplitude(double value)
{
    amplitude = value;
}

void Gabor::setFrequency(double value)
{
    frequency = value;
}

void Gabor::setOffset(double value)
{
    offset = value;
}
