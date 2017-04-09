
#include <iostream>
#include <string>
#include <fstream>

#include <boost/filesystem.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "tinyxml2.h"
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
using namespace tinyxml2;
namespace fs = boost::filesystem;

ofstream errorLog;
vector<Image> imgs;
vector<Gabor> filters;
float* features = NULL;
int findex = 0;
string ftype;
string extension;

void LoadImages ( fs::path sessionPath, XMLElement *session, string extension );
int ParseMetadata ( fs::path xmlPath, fs::path dataPath, fs::path outputPath );
int ParseSubjects ( XMLElement *subject, fs::path xmlPath, fs::path dataPath, fs::path outputPath, const string &DBName );
int ParseSessions ( XMLElement *session, fs::path xmlPath, fs::path subjectPath, fs::path outputPath, const string &DBName, const string &SubjectName );
int ProcessSession ( XMLElement *session, fs::path sessionPath, fs::path outputPath, const string &DBName, const string &SubjectName, const string &SessionName );

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
    bool blockCompromised = false;
    vector<cv::Mat> block;
    for (int i = 0; i < imgs.size(); i++) {
        if (!imgs[i].success)
            blockCompromised = true;
        
        block.push_back(imgs[i].img);
        
        if (block.size() == TEMPORALWINDOW) {
            nblocks++;
            
            findex = 0;
            features = new float[nfeatures];
            fill(features, features + nfeatures, 0);
            
            if (!blockCompromised)
                FilterImages(block);
            
            SaveHistogram(nfeatures, outputPath);
            blockCompromised = false;
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

int main(int argc, char **argv) {
    
    // Arguments:
    // 1. Path to face images
    // 2. Path to database metadata
    // 3. Output features path
    // 4. Features type: LBP, LBP-TOP, LGBP or LGBP-TOP
    // 5. Images extension

    if (argc < 5) {
        cerr << endl << " *** ERROR - Please specify all required runtime arguments. " << endl << endl;
        return -1;
    }
    
    fs::path dataPath	(argv[1]);
    fs::path xmlPath	(argv[2]);
    fs::path outputPath	(argv[3]);
    
    ftype = string(argv[4]);
	extension = string(argv[5]);
    
    if (!fs::exists(dataPath)) {
        cerr << endl << " *** ERROR - Data path " << dataPath << " does not exist. " << endl << endl;
        return -1;
    }
    
    if (!fs::exists(xmlPath)) {
        cerr << endl << " *** ERROR - XML path " << xmlPath << " does not exist. " << endl << endl;
        return -1;
    }
    
    if (!fs::exists(outputPath)) {
        cerr << endl << " *** ERROR - Output path " << outputPath << " does not exist. " << endl << endl;
        return -1;
    }
    
    cout << endl << " *** Data path: " << dataPath << endl;
    cout << " *** XML path: " << xmlPath << endl;
    cout << " *** Output path: " << outputPath << endl;
    cout << " *** Feature type: \"" << ftype << "\"" << endl;
    cout << " *** Extension: \"" << extension << "\"" << endl;
    
    fs::path logPath = outputPath;
    logPath /= "errors.log";
    remove(logPath.string().c_str());
    errorLog.open(logPath.c_str());
    cout << " *** Error log file path: " << logPath << endl;
    
    InitFilters();
    if (ParseMetadata(xmlPath, dataPath, outputPath) != 0) {
        return -1;        
    }

    errorLog.close();
    return 0;
}

int ProcessSession ( XMLElement *session, fs::path sessionPath, fs::path outputPath, const string &DBName, const string &SubjectName, const string &SessionName ) {

    string ofilename = DBName;
    ofilename.append(".");
    ofilename.append(SubjectName);
    ofilename.append(".");
    ofilename.append(SessionName);
            
    cout << endl << " *** Processing " << ofilename << ":" << endl;
    ofilename.append(".dat");
            
    fs::path opath = outputPath;
    opath /= ofilename;
    remove(opath.string().c_str());
    cout << " *** Output path is: " << opath << endl;
        
    LoadImages(sessionPath, session, extension);
    CheckForBadFaceDetections();

    if (ftype == "LGBP-TOP") {
        ComposeBlocks(opath);   
        return 0;
    }

    cerr << endl << " *** ERROR - Unknown feature type specified: " << ftype << endl << endl;
    errorLog << "ERROR - Unknown feature type specified: " << ftype << endl;
    return -1;
}

int ParseSessions ( XMLElement *session, fs::path xmlPath, fs::path subjectPath, fs::path outputPath, const string &DBName, const string &SubjectName ) {

    while (session) 
    {
        string SessionName;
        const char *ssn = session->Attribute("Name");
        if (ssn != 0) {
            SessionName = string(ssn);
        } else {
            errorLog << "WARNING - Unable to read session name at " << xmlPath << endl;
            session = session->NextSiblingElement("Session");
            continue;
        }
            
        string SessionRelativePath;
        const char *ssp = session->Attribute("RelativePath");
        if (ssp != 0) {
            SessionRelativePath = string(ssp);
        } else {
            errorLog << "WARNING - Unable to read session relative path at " << xmlPath << endl;
            session = session->NextSiblingElement("Session");
            continue;
        }
            
        fs::path sessionPath = subjectPath;
        sessionPath /= SessionRelativePath;
        
        if (ProcessSession( session, sessionPath, outputPath, DBName, SubjectName, SessionName ) != 0) {
            return -1;
        }
        
        session = session->NextSiblingElement("Session");
    }

    return 0;
}

int ParseSubjects ( XMLElement *subject, fs::path xmlPath, fs::path dataPath, fs::path outputPath, const string &DBName ) {

    while (subject)
    {
        string SubjectName;
        const char *st = subject->Attribute("Name");
        if (st != 0) {
            SubjectName = string(st);
        } else {
            errorLog << "WARNING - Unable to read subject name at " << xmlPath << endl;
            subject = subject->NextSiblingElement("Subject");
            continue;
        }
        
        string SubjectRelativePath;
        const char *sp = subject->Attribute("RelativePath");
        if (sp != 0) {
            SubjectRelativePath = string(sp);
        } else {
            errorLog << "WARNING - Unable to read subject relative path at " << xmlPath << endl;
            subject = subject->NextSiblingElement("Subject");
            continue;
        }
        fs::path subjectPath = dataPath;
        subjectPath /= SubjectRelativePath;
        
        XMLElement *session = NULL;
        session = subject->FirstChildElement("Session");
        if (!session) {
            errorLog << "WARNING - No session nodes found at " << xmlPath << endl;
            return -1;
        }

        if (ParseSessions( session, xmlPath, subjectPath, outputPath, DBName, SubjectName) != 0) {
            return -1;        
        }
        
        subject = subject->NextSiblingElement("Subject");
    }

    return 0;
}

int ParseMetadata ( fs::path xmlPath, fs::path dataPath, fs::path outputPath ) {

    XMLDocument doc;
    doc.LoadFile(xmlPath.c_str());
    if (doc.ErrorID() != 0) {
        cerr << " *** ERROR - Unable to open XML at " << xmlPath << " *** " << endl;
        return -1;
    }
    
    XMLElement *root = NULL;
    root = doc.FirstChildElement("Database");
    if (!root) {
        cerr << " *** ERROR - Null pointer to the root element at " << xmlPath << " *** " << endl;
        return -1;
    }
    
    string DBName;
    const char *dn = root->Attribute("Name");
    if (dn != 0) {
        DBName = string(dn);
    } else {
        errorLog << "WARNING - Unable to read database name at " << xmlPath << endl;
        return -1;
    }
       
    XMLElement *subject = NULL;
    subject = root->FirstChildElement("Subject");
    if (!subject) {
        errorLog << "WARNING - No subject nodes found at " << xmlPath << endl;
        return -1;
    }

    cout << " *** Database: \"" << DBName << "\"" << endl;
    if (ParseSubjects( subject, xmlPath, dataPath, outputPath, DBName ) != 0) {
        return -1;        
    }

    return 0;
}

void LoadImages (fs::path sessionPath, XMLElement *session, string extension) {
	cout << " *** Loading images ... " << endl;
	imgs.clear();
	int counter = 0;
    int failures = 0;
    
	XMLElement *imageXML = NULL;
	imageXML = session->FirstChildElement("Image");
    if (!imageXML) {
       	errorLog << "WARNING - No image nodes found at " << sessionPath << endl;
       	return;
    }
    
    while (imageXML)
    {
    	string ImageFileName;
    	const char *ifn = imageXML->Attribute("Name");
        if (ifn != 0) {
         	ImageFileName = string(ifn);
        } else {
           	errorLog << "WARNING - Unable to read image filename at " << sessionPath << endl;
           	imageXML = imageXML->NextSiblingElement("Image");
           	continue;
        }
        ImageFileName.append(extension);

		fs::path ImagePath = sessionPath;
		ImagePath /= ImageFileName;
		
		Image image;
        image.img = cv::imread(ImagePath.string(), CV_LOAD_IMAGE_GRAYSCALE);
        if(!image.img.data || image.img.empty()) {
            errorLog << "WARNING - Could not open image: " << ImagePath << endl;
            image.success = false;
            failures++;
        } else {
            image.success = true;
            
           	// cv::imshow("Images", image.img);
           	// cv::waitKey(1);
        }
        imgs.push_back(image);
        counter++;
    
    	imageXML = imageXML->NextSiblingElement("Image");
    }
	cout << " *** Processed " << counter << " lines, " << failures << " images failed to read" << endl;
}