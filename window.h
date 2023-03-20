#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;

#ifndef WINDOWOBJ_H
#define WINDOWOBJ_H
namespace WindowClass {
    
    Mat zoomedImg;
    double zoomFactor = 1.0;
    Point2i mousePos;
    bool sel_drawing = false;
    Point sel_start_point, sel_end_point;
    Rect sel_rect;
    int mousemoves;
    bool zone_updated;

    void onMouse(int event, int x, int y, int flags, void* userdata) {
        
        if (event == EVENT_MOUSEMOVE) {
            mousePos.x = x;
            mousePos.y = y;
            if (sel_drawing) {
                sel_end_point = Point(x, y);
                //cout << sel_start_point << " " << sel_end_point << endl;
                mousemoves+=1;
            }
            
        } else if (event == 10 && flags>0) {
            zoomFactor += 0.1;
            
        } else if (event == 10 && flags<0 && zoomFactor > 1.0) {
            zoomFactor -= 0.1;
            if (zoomFactor<1) {zoomFactor = 1;}
            
        } else if (event == EVENT_LBUTTONDOWN) {
            sel_drawing = true;
            sel_start_point = Point(x, y);
            mousemoves = 0;
            
        } else if (event == EVENT_LBUTTONUP) {
            if (sel_drawing) {
                sel_drawing = false;
                if (mousemoves>=2) {
                    sel_end_point = Point(x, y);
                    
                    if (sel_end_point.x < sel_start_point.x) {
                        sel_rect.x = sel_end_point.x;
                        sel_rect.width = sel_start_point.x - sel_end_point.x;
                    } else {
                        sel_rect.x = sel_start_point.x;
                        sel_rect.width = sel_end_point.x - sel_start_point.x;
                    }
                    
                    if (sel_end_point.y < sel_start_point.y) {
                        sel_rect.y = sel_end_point.y;
                        sel_rect.height = sel_start_point.y - sel_end_point.y;
                    } else {
                        sel_rect.y = sel_start_point.y;
                        sel_rect.height = sel_end_point.y - sel_start_point.y;
                    }
                    
                    //cout << "sel_rect " << sel_rect.x << " " << sel_rect.y << " " << sel_rect.width << " " << sel_rect.height << endl;
                    zone_updated = true;
                }
            }
        }
    }

    void updateZoomedImage(Mat img) {
        int zoomedWidth = static_cast<int>(img.cols / zoomFactor);
        int zoomedHeight = static_cast<int>(img.rows / zoomFactor);
        Rect roi(mousePos.x - zoomedWidth / 2, mousePos.y - zoomedHeight / 2, zoomedWidth, zoomedHeight);
        
        if (roi.x < 0) roi.x = 0;
        if (roi.y < 0) roi.y = 0;
        if (roi.x + roi.width > img.cols) roi.x = img.cols - roi.width;
        if (roi.y + roi.height > img.rows) roi.y = img.rows - roi.height;

        Mat croppedImg = img(roi);
        resize(croppedImg, zoomedImg, img.size(), 0, 0, INTER_LINEAR);
    }
    
}
#endif
