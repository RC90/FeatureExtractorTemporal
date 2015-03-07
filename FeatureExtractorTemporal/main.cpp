
#include <iostream>
#include <string>
#include <fstream>
#include <boost/filesystem.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "gabor.h"

#define TEMPORALWINDOW 5
#define WSEGMENTS 4
#define HSEGMENTS 4

struct Image
{
    cv::Mat img;
    bool success;
};

using namespace std;
namespace fs = boost::filesystem;

ofstream errorLog;
vector<Image> imgs;
vector<Gabor> filters;

void InitFilters()
{
    filters.push_back(Gabor("Filter_01", 0,   5, 90,   90));
    filters.push_back(Gabor("Filter_02", 30,  5, 90,   90));
    filters.push_back(Gabor("Filter_03", 60,  5, 90,   90));
    filters.push_back(Gabor("Filter_04", 90,  5, 90,   90));
    filters.push_back(Gabor("Filter_05", 120, 5, 90,   90));
    filters.push_back(Gabor("Filter_06", 150, 5, 90,   90));
    filters.push_back(Gabor("Filter_07", 0,   5, 45,   90));
    filters.push_back(Gabor("Filter_08", 30,  5, 45,   90));
    filters.push_back(Gabor("Filter_09", 60,  5, 45,   90));
    filters.push_back(Gabor("Filter_10", 90,  5, 45,   90));
    filters.push_back(Gabor("Filter_11", 120, 5, 45,   90));
    filters.push_back(Gabor("Filter_12", 150, 5, 45,   90));
    filters.push_back(Gabor("Filter_13", 0,   5, 22.5, 90));
    filters.push_back(Gabor("Filter_14", 30,  5, 22.5, 90));
    filters.push_back(Gabor("Filter_15", 60,  5, 22.5, 90));
    filters.push_back(Gabor("Filter_16", 90,  5, 22.5, 90));
    filters.push_back(Gabor("Filter_17", 120, 5, 22.5, 90));
    filters.push_back(Gabor("Filter_18", 150, 5, 22.5, 90));
}

void FilterImages (vector<cv::Mat> block) {
    for (int i = 0; i < filters.size(); i++) {
        vector<cv::Mat> blockFiltered;
        for (int k = 0; k < block.size(); k++) {
            cv::Mat imgFiltered;
            filters[i].filter(imgFiltered, block[k].clone());
            blockFiltered.push_back(imgFiltered);
        }
        
        int stepWidth = block.front().cols / WSEGMENTS;
        int stepHeight = block.front().rows / HSEGMENTS;
        int nsegments = 0;
        for (int k = 0; k < HSEGMENTS; k++) {
            for (int n = 0; n < WSEGMENTS; n++) {
                vector<cv::Mat> segment;
                nsegments++;
                for (int m = 0; m < blockFiltered.size(); m++) {
                    
                }
            }
        }
        
    }
}

void ComposeBlocks () {
    cout << " *** Processing blocks with temporal window " << TEMPORALWINDOW << " ... " << endl;
    int nblocks = 0;
    for (int i = 0; i < imgs.size(); i++) {
        int imagesLeft = (int)imgs.size() - i;
        if (imagesLeft < TEMPORALWINDOW) {
            // save empty histogram
            continue;
        }
        
        vector<cv::Mat> block;
        for (int k = i; k < i + TEMPORALWINDOW; k++) {
            if (imgs[k].success) {
                block.push_back(imgs[k].img);
            } else {
                // save empty histogram
                break;
            }
        }
        
        if (block.size() == TEMPORALWINDOW) {
            nblocks++;
            FilterImages(block);
            // compute and save histogram
        }
    }
    cout << " *** " << nblocks << " blocks processed out of " << imgs.size() << " images " << endl;
}

void CheckForBadFaceDetections () {
    cout << " *** Checking images ... " << endl;
    int failures = 0;
    for (int i = 0; i < imgs.size(); i++) {
        if (cv::countNonZero(imgs[i].img) < 1) {
            imgs[i].success = false;
            failures++;
        }
    }
    cout << " *** Checked " << imgs.size() << " lines, " << failures << " empty images found" << endl;
}

void LoadImages (fs::path filename) {
    cout << " *** Loading images ... " << endl;
    ifstream file(filename.c_str());
    string line;
    imgs.clear();
    int counter = 0;
    int failures = 0;
    while (getline(file, line))
    {
        Image image;
        image.img = cv::imread(line, CV_LOAD_IMAGE_GRAYSCALE);
        if(!image.img.data || image.img.empty()) {
            errorLog << "WARNING - Could not open image: \"" << line << "\"" << endl;
            image.success = false;
            failures++;
        } else {
            image.success = true;
            
//            cv::imshow("Images", image.img);
//            cv::waitKey(1);
        }
        imgs.push_back(image);
        counter++;
    }
    cout << " *** Processed " << counter << " lines, " << failures << " images failed to read" << endl;
}

int main(int argc, char **argv) {
    
    if (argc < 3) {
        cerr << endl << " *** ERROR - Please specify input and output paths. " << endl << endl;
        return -1;
    }
    
    fs::path input(argv[1]);
    fs::path output(argv[2]);
    
    if (!fs::exists(input)) {
        cerr << endl << " *** ERROR - Input path " << input << " does not exist. " << endl << endl;
        return -1;
    }
    
    if (!fs::exists(output)) {
        cerr << endl << " *** ERROR - Output path " << output << " does not exist. " << endl << endl;
        return -1;
    }
    
    cout << endl << " *** Input path: " << input << endl;
    cout << " *** Output path: " << output << endl;
    
    fs::path logPath = output;
    logPath /= "errors.log";
    remove(logPath.c_str());
    errorLog.open(logPath.c_str());
    cout << " *** Error log file path: " << logPath << endl;
    
    InitFilters();
    fs::directory_iterator end_iter;
    for (fs::directory_iterator dir_iter(input) ; dir_iter != end_iter ; ++dir_iter)
    {
        fs::path filepath = dir_iter->path();
        string extension = fs::extension(filepath.string());
        if (extension.compare(".txt") == 0) {
            cout << endl << " *** Processing " << filepath << ":" << endl;
            LoadImages(filepath);
            CheckForBadFaceDetections();
            ComposeBlocks();
        }
    }
    
    errorLog.close();
    return 0;
}
