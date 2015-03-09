
#include <iostream>
#include <string>
#include <fstream>
#include <boost/filesystem.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "gabor.h"
#include "LBP.h"

#define TEMPORALWINDOW 5
#define WSEGMENTS 4
#define HSEGMENTS 4
#define UNIFORM 59

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
float* features = NULL;
int findex = 0;

void InitFilters() {
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

void ExtractFeatures (vector<cv::Mat> segment) {
    uchar*** pppArray = new uchar**[TEMPORALWINDOW];
    int height = segment.front().rows;
    int width = segment.front().cols;
 
    for (int i = 0; i < TEMPORALWINDOW; i++) {
        pppArray[i] = new uchar*[height];
        for (int j = 0; j < height; j++) {
            pppArray[i][j] = new uchar[width];
            for (int k = 0; k < width; k++) {
                pppArray[i][j][k] = segment[i].at<uchar>(j,k);
            }
        }
    }
    
    LBP lbp;
    lbp.width = width;     lbp.height = height;    	lbp.tlength = TEMPORALWINDOW;
    lbp.R.xR = 1;          lbp.R.yR = 1;            lbp.R.tR = 1;
    lbp.SN.xy = 8;         lbp.SN.xt = 8;           lbp.SN.yt = 8;
    lbp.uni = 1;           lbp.interp = 1;          lbp.norm = 1;
    lbp.CreateHistogram(pppArray, 1);
    
    copy(lbp.uni_hist.pHist_xy, lbp.uni_hist.pHist_xy + UNIFORM, features + findex);
    findex += UNIFORM;
    
    copy(lbp.uni_hist.pHist_xt, lbp.uni_hist.pHist_xt + UNIFORM, features + findex);
    findex += UNIFORM;
    
    copy(lbp.uni_hist.pHist_yt, lbp.uni_hist.pHist_yt + UNIFORM, features + findex);
    findex += UNIFORM;
    
    if (pppArray) {
        for (int i = 0; i < TEMPORALWINDOW; i++) {
            if (pppArray[i]) {
                for (int j = 0; j < height; j++) {
                    if (pppArray[i][j]) {
                        delete []pppArray[i][j];
                    }
                }
                delete []pppArray[i];
            }
        }
        delete []pppArray;					
    }
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
        for (int k = 0; k < HSEGMENTS; k++) {
            for (int n = 0; n < WSEGMENTS; n++) {
                vector<cv::Mat> segment;
                int startWidth = n * stepWidth;
                int startHeight = k * stepHeight;
                for (int m = 0; m < blockFiltered.size(); m++) {
                    cv::Mat cropppedImg = cv::Mat(blockFiltered[m], cv::Rect(startWidth, startHeight, stepWidth, stepHeight));
                    segment.push_back(cropppedImg);
                }
                
                // extract features from the 3D segment
                ExtractFeatures(segment);
            }
        }
        
    }
}

void SaveHistogram(int nfeatures, fs::path outputPath) {
    ofstream file(outputPath.c_str(), ios::app);
    if (!file.is_open()) {
        errorLog << "WARNING - Unable to save histogram at " << outputPath << endl;
        return;
    }

    for (int i = 0; i < nfeatures; i++)
        file << features[i] << " ";

    file << "\n";
    file.close();
}

void ComposeBlocksNonOverlapping (fs::path outputPath) {
    cout << " *** Processing blocks with temporal window " << TEMPORALWINDOW << " ... " << endl;
    int nblocks = 0;
    int nfeatures = (int)filters.size() * HSEGMENTS * WSEGMENTS * UNIFORM * 3;
    vector<cv::Mat> block;
    for (int i = 0; i < imgs.size(); i++) {
        if (!imgs[i].success) {
            block.clear();
            continue;
        }
        
        if (block.size() < TEMPORALWINDOW) {
            block.push_back(imgs[i].img);
        }
        
        if (block.size() == TEMPORALWINDOW) {
            findex = 0;
            features = new float[nfeatures];
            fill(features, features + nfeatures, 0);
            
            nblocks++;
            FilterImages(block);
            SaveHistogram(nfeatures, outputPath);
            block.clear();
            delete []features;
        }
    }
    cout << " *** " << nblocks << " blocks processed out of " << imgs.size() << " images " << endl;
}

void ComposeBlocks (fs::path outputPath) {
    cout << " *** Processing blocks with temporal window " << TEMPORALWINDOW << " ... " << endl;
    int nblocks = 0;
    int nfeatures = (int)filters.size() * HSEGMENTS * WSEGMENTS * UNIFORM * 3;
    for (int i = 0; i < imgs.size(); i++) {
        findex = 0;
        features = new float[nfeatures];
        fill(features, features + nfeatures, 0);
        
        int imagesLeft = (int)imgs.size() - i;
        if (imagesLeft < TEMPORALWINDOW) {
            SaveHistogram(nfeatures, outputPath);
            delete []features;
            continue;
        }
        
        vector<cv::Mat> block;
        for (int k = i; k < i + TEMPORALWINDOW; k++) {
            if (imgs[k].success) {
                block.push_back(imgs[k].img);
            } else {
                SaveHistogram(nfeatures, outputPath);
                break;
            }
        }
        
        if (block.size() == TEMPORALWINDOW) {
            nblocks++;
            FilterImages(block);
            SaveHistogram(nfeatures, outputPath);
        }
        delete []features;
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
            
            string sname = filepath.stem().string();
            sname.append(".dat");
            fs::path outputPath = output;
            outputPath /= sname;
            remove(outputPath.c_str());
            cout << " *** Output path is: " << outputPath << endl;
            
            LoadImages(filepath);
            CheckForBadFaceDetections();
            ComposeBlocksNonOverlapping(outputPath);
        }
    }
    
    errorLog.close();
    return 0;
}
