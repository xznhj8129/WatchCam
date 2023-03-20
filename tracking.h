#include <opencv2/opencv.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/core/mat.hpp>
#include "camconfig.h"
#include "functions.h"
#include "yolo.h"

using namespace std;
using namespace cv;
using namespace CamConfigClass;
using namespace CamFunctions;
using namespace YoloDNNObjs;

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
            double vel;
            vector<double> wavgs;
            vector<double> havgs;
            vector<double> vels;
            bool initialized = false;
            std::vector<std::string> class_list;
            cv::dnn::Net net;
            std::vector<Detection> yolo_detections;
            
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
                arect = rect; // arect = rectangle scaled to original image size
                classification = "UNKNOWN";
                vels.push_back(0);
                updates = 1;
                confidence = 0;
            };
            
            void update(Contour contour, CamConfig& camvars, cv::Mat feed, DNNdata dnndata) {
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
                vel = round(vectorAverage(vels));
                updates = updates + 1;
                
                cv::Mat scan_pic;
                cv::Rect scan_rect = cv::Rect(arect.x - (arect.width/ 4), arect.y - (arect.height /4), arect.width*1.5, arect.height*1.5);
                scan_pic = feed.clone();
                scan_pic = scan_pic(scan_rect);
                imshow("scan",scan_pic);
                //getchar();
                yolo_detections.clear();
                detect(scan_pic, dnndata.net, yolo_detections, dnndata.class_list, scan_rect);
                
                int detections = yolo_detections.size();

                float maxconf = 0;
                for (int i = 0; i < detections; ++i)
                {
                    auto detection = yolo_detections[i];
                    auto box = detection.box;
                    auto classId = detection.class_id;
                    auto yolo_confidence = detection.confidence;
                    auto yolo_class = detection.classification;
                    if (yolo_confidence > maxconf) {
                        maxconf = yolo_confidence;
                        classification = yolo_class;
                    }
                }
                
                // remove the ratio/size shit totally and just rely on yolo for classification
                // problem: probably generates multiple detections
                // need to deconflict between multiple/overlapping tracks (or just print them all)
                // can lower update pings required for higher confidence
                // remember: make detection trackbox the new trackbox, not the contour boundingRect
                
                if (updates>=25 and confidence<4 and classification!="UNKNOWN") {confidence = 4;}
                else if (updates >= 15 and confidence<3) {confidence = 3;}
                else if (updates > 8 and confidence < 2) {confidence = 2;}
                else if (updates > 4 and confidence==0) {confidence = 1;}
                
                if (confidence >= 3) {
                    if (classification == "UNKNOWN" and confidence == 4) {confidence = 3;}
                }
                
                if (wavgs.size() > 30) {pop_front(wavgs);}
                if (havgs.size() > 30) {pop_front(havgs);}
                
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
