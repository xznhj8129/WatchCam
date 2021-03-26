#include <opencv2/opencv.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/core/mat.hpp>
#include "camconfig.h"
#include "functions.h"

using namespace std;
using namespace cv;
using namespace CamConfigClass;
using namespace CamFunctions;

#ifndef TRACKOBJ_H
#define TRACKOBJ_H
namespace TrackObjectClass {
    typedef vector<cv::Point> Contour;
    typedef vector<vector<cv::Point>> Contours;
    
    class TrackObject {
        public:
            string id;
            int first;
            int time;
            cv::Rect rect;
            cv::Rect arect;
            cv::Point center;
            string classification;
            int updates;
            int confidence;
            double wavg;
            double havg;
            double size;
            double ratio;
            double vel;
            vector<double> wavgs;
            vector<double> havgs;
            vector<double> sizes;
            vector<double> vels;
            vector<double> ratios;
            bool initialized = false;
            
            void track_init(Contour contour, CamConfig& camvars) {
                initialized = true;
                id = randomid();
                first = std::time(0);
                time = std::time(0);
                cv::Rect rect = cv::boundingRect(contour);
                rect = scale_rect(rect, camvars.md_scale, camvars.zone);
                center = find_center(rect, camvars.md_scale, camvars.zone);
                wavg = rect.width;
                havg = rect.height;
                wavgs.push_back(wavg);
                havgs.push_back(havg);
                arect = rect;
                size = contourArea(contour);
                classification = "UNKNOWN";
                vels.push_back(0);
                updates = 1;
                confidence = 0;
                sizes.push_back(contourArea(contour));
                ratio = 0;
                ratios.push_back((double)rect.width / (double)rect.height);
            };
            
            void update(Contour contour, CamConfig& camvars) {
                time = std::time(0);
                cv::Rect crect = cv::boundingRect(contour);
                cv::Rect rect = scale_rect(crect, camvars.md_scale, camvars.zone);
                cv::Point newcenter = find_center(crect, camvars.md_scale, camvars.zone); //rect?? putting rect causes doubling of center, why?!
                vels.push_back(sqrt( pow((double)center.x-(double)newcenter.x,2) + pow((double)center.y - (double)newcenter.y,2)));
                center = newcenter;
                wavgs.push_back(rect.width);
                havgs.push_back(rect.height);
                wavg = round(vectorAverage(wavgs));
                havg = round(vectorAverage(havgs));
                arect.x = center.x - (wavg/2);
                arect.y = center.y - (havg/2);
                arect.width = wavg;
                arect.height = havg;
                sizes.push_back(contourArea(contour));
                ratios.push_back((double)rect.width / (double)rect.height);
                vel = round(vectorAverage(vels));
                size = round(vectorAverage(sizes));
                ratio = round(vectorAverage(ratios));
                updates = updates + 1;
                
                if (updates>=25 and confidence<4 and classification!="UNKNOWN") {confidence = 4;}
                else if (updates >= 15 and confidence<3) {confidence = 3;}
                else if (updates > 8 and confidence < 2) {confidence = 2;}
                else if (updates > 4 and confidence==0) {confidence = 1;}
                
                if (confidence >= 3) {
                    if (ratio > 1 and size > 800) {classification = "VEHICLE";}
                    else if (ratio < 1 and size < 800 and size > 100 /*and vel < 25*/) { classification = "PERSON";}
                    else {classification = "UNKNOWN";}
                    
                    if (classification == "UNKNOWN" and confidence == 4) {confidence = 3;}
                }
                
                if (wavgs.size() > 30) {pop_front(wavgs);}
                if (havgs.size() > 30) {pop_front(havgs);}
                if (sizes.size() > 30) {pop_front(sizes);}
                if (ratios.size() > 30) {pop_front(ratios);}
                
            };
    };
    
    typedef map<string, TrackObject> TrackMap;
    
    bool is_tracked(cv::Rect rectangle, TrackMap& tracking) {
        const int x = rectangle.x;
        const int y = rectangle.y;
        for (const auto &idpair : tracking) {
            string id = idpair.first;
            int x1 = tracking[id].rect.x;
            int y1 = tracking[id].rect.y;
            if (x==x1 and y==y1) {return true;}
        };
        return false;
    }
    
}
#endif
