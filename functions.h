#include "opencv2/opencv.hpp"
#include <cstdlib>
#include <fstream>
#include <fmt/core.h>
#include <fmt/format.h>
#include "camconfig.h"

using namespace std;
using namespace cv;
using namespace CamConfigClass;

#ifndef FUNCTIONS_H
#define FUNCTIONS_H
namespace CamFunctions {
    
    template<typename T> 
    void pop_front(std::vector<T> &v)
    {
        if (v.size() > 0) {
            v.erase(v.begin());
        }
    }

    string RunCommand(char* command) {
        FILE *cmd = popen(command, "r");
        char line[100];
        string result;
        while (fgets(line, 100, cmd))
            result += line;
        return result;
    }

    string randomid() {
        string id;
        string validl = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        string validn = "1234567890";
        const int ll = 26;
        const int nl = 10;
        char letter;  
        char num;
        letter = validl[rand() % ll];
        id.push_back(letter);
        for (int i = 0; i < 3; ++i )
        {
            num = validn[rand() % nl];
            id.push_back(num);
        }
        return id;
    }

    string timestamp() {
        time_t rawtime;
        struct tm * timeinfo;
        char buffer [80];
        time (&rawtime);
        timeinfo = localtime (&rawtime);
        strftime (buffer,80,"%D %R:%S",timeinfo);
        return buffer;
    }

    void notif(string text, int cam_id) {
        cout << timestamp() << " Cam " << cam_id << " : " << text << endl;
    }

    void on_rect(cv::Rect rectangle, double md_scale, cv::Rect zone) {
        zone.x = rectangle.x / md_scale;
        zone.y = rectangle.y / md_scale;
        zone.width = rectangle.width / md_scale;
        zone.height = rectangle.height / md_scale;
        cout << "Set zone: " << zone.x << ", " << zone.y << ", " << zone.height << ", " << zone.width << endl;
    }
    
    cv::Point reframe_pos(int x, int y, double md_scale, cv::Rect zone) {
        int X;
        int Y;
        if (zone.width > 0) {
            X = (( x + zone.x) * md_scale);
            Y = (( y + zone.y) * md_scale);
        } 
        else {
            X = (x * md_scale);
            Y = (y * md_scale);
        }
        cv::Point point(X,Y);
        return point;
    }
    
    cv::Rect scale_rect( cv::Rect rectangle, double md_scale, cv::Rect zone) {
        const int x = rectangle.x;
        const int y = rectangle.y;
        const int w = rectangle.width;
        const int h = rectangle.height;
        int X;
        int Y;
        if (zone.height > 0 and zone.width > 0) {
            X = (( x + zone.x) * md_scale);
            Y = (( y + zone.y) * md_scale);
        } 
        else {
            X = (x * md_scale);
            Y = (y * md_scale);
        }
        int W = w * md_scale;
        int H = h * md_scale;
        cv::Rect ret(X,Y,W,H);
        return ret;
    }

    cv::Point find_center(cv::Rect rectangle, double md_scale, cv::Rect zone) {
        const int x = rectangle.x;
        const int y = rectangle.y;
        const int w = rectangle.width;
        const int h = rectangle.height;
        double X;
        double Y;
        if (zone.height > 0 and zone.width > 0) {
            X = (( x + zone.x) * md_scale);
            Y = (( y + zone.y) * md_scale);
        } 
        else {
            X = (x * md_scale);
            Y = (y * md_scale);
        }
        int W = w * md_scale;
        int H = h * md_scale;
        int x1 = X+(W/2);
        int y1 = Y+(H/2);
        cv::Point point(x1,y1);
        return point;
    }

    bool point_in_bounds(cv::Point point, cv::Rect bounds) {
        const int px = point.x;
        const int py = point.y;
        const int bx = bounds.x;
        const int by = bounds.y;
        const int bw = bounds.width;
        const int bh = bounds.height;
        const int bxmax = bx + bw;
        const int bymax = by + bh;
        if (px >= bx and px <= bxmax and py >= by and py <= bymax) {return true;} 
        else {return false;}
    }

    bool size_bounds(double size1, double size2, double fraction) {
        if (size1==size2) {return true;}
        if (size2 > size2-(size2*fraction) and size2 < (size2*fraction)) {return true;}
        else { return false;}
    }

    double vectorAverage(std::vector<double>& v) {
        double avg, sum = 0;
        int i;
        int size = v.size();
        for (i = 0; i < size; ++i) {
            sum += v[i];
        }
        avg = sum / size;
        return avg;
        
    }

    string ReadTextFile(string filename) {
        string line;
        ifstream myfile(filename.c_str());
        string out;
        if (myfile.is_open()) {
            while (getline(myfile, line)) {out = out + line;}
        }
        else { out = "func ReadTextFile: FILE READ ERROR"; }
        myfile.close();
        return out;
    }

    int WriteTextFile(string filename, string data) {
        ofstream myfile;
        int returnme;
        myfile.open(filename.c_str());
        if (myfile.is_open()) {
            myfile << data;
            returnme = 0;
        }
        else { returnme = 1; }
        myfile.close();
        return returnme;
    }
    
}
#endif
