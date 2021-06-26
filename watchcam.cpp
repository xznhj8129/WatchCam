#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <cstdlib>
#include <fmt/core.h>
#include <fmt/format.h>
#include "camconfig.h"
#include "tracking.h"
#include "functions.h"

using namespace std;
using namespace CamConfigClass;
using namespace TrackObjectClass;
using namespace CamFunctions;
    
int main(int argc, char* argv[]) {
    bool cmdvid = false;
    if (argc > 1) {cmdvid = true;}

    srand(time(0));
    char v[] = "1.0";
    const char* configfile = "cam.cfg";
    const int confalert = 3;
    const int record_tconf = 4;
    bool recording = false;
    //int recordstart = 0;
    bool manual_record = false;
    bool vid_motion = false;
    bool calibrated = false;
    bool motion_notif = false;
    bool playalert = false;
    bool exclude_mode = false;
    int motiontime;
    int timeplay;
    string alerttrack;
    string vidtime;
    int tracked;
    int tracks_n;
    int led;
    int maxconf;
    cv::Mat frame1;
    cv::Mat frame2;
    cv::Mat feed;
    TrackMap tracking;
    cv::VideoCapture cap;
    CamConfig camvars;
    
    // 2560 * 1920
    
    cout << "WatchCam " << v << " started " << timestamp() << endl;
    
    try {
        camvars.Load(configfile);

        int cam_id;
        if (cmdvid) { camvars.internet_cam_url = argv[1]; }
        else { camvars.internet_cam_url = camvars.cam_urls[0]; }
        
        cam_id = 0;
        for (int n = 0; n < 4; ++n) {
            if (camvars.internet_cam_url == camvars.cam_urls[n]) { cam_id = n+1; }
        }
        if (cam_id!=0 and cam_id<=4) {camvars.zone = camvars.cam_zones[cam_id-1];}
        
        //notif(fmt::format("Motion zone: {}",camvars.zone), cam_id);

        cap.open(camvars.internet_cam_url);
        if(!cap.isOpened()){
            throw runtime_error("Error opening video stream or file");
        }
    
        unsigned first = camvars.internet_cam_url.find("@");
        unsigned last = camvars.internet_cam_url.find("/h264");
        string url = camvars.internet_cam_url.substr (first+1,camvars.internet_cam_url.length());
        string ip = camvars.internet_cam_url.substr (first+1,last-first-1);
        string windowname = fmt::format("WatchCam Feed Cam {} - {}",cam_id,ip);
        cv::namedWindow(windowname, cv::WINDOW_GUI_NORMAL);
        
        // no cv::RectSelector???
        
        const int frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
        const int frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        notif(fmt::format("Frame width: {} Frame height: {}",frame_width,frame_height),cam_id);
        if (camvars.resize_display) {cv::resizeWindow(windowname, camvars.windowres.width, camvars.windowres.height);}
        else {cv::resizeWindow(windowname, frame_width, frame_height);}
        int md_scale_width = frame_width / camvars.motionimgres.width;
        int md_scale_height = frame_height / camvars.motionimgres.height;
        if (md_scale_width == md_scale_height) {camvars.md_scale = md_scale_width;}
        
        bool big_image = frame_width > 640;
        //int new_width = frame_width / camvars.md_scale;
        //int new_height = frame_height / camvars.md_scale;
        cv::Size new_size;
        new_size.width = camvars.motionimgres.width;
        new_size.height= camvars.motionimgres.height;
        
        TrackObject tracks[32];
        
        int t;
        int rt = time(0);
        cap >> frame1;
        cap >> frame2;
        feed = frame1.clone();
        
        notif(fmt::format("Source: {}",url), cam_id);
        
        while(1){
            cv::Mat diff;
            cv::Mat gray;
            cv::Mat blur;
            cv::Mat thresh;
            cv::Mat dilated;
            Contours contours;

            cv::Mat p1;
            cv::Mat p2;
            cv::resize(frame1, p1, new_size, 0, 0, cv::INTER_AREA);
            cv::resize(frame2, p2, new_size, 0, 0, cv::INTER_AREA);
            
            if (camvars.zone.height!=0 and camvars.zone.width!=0) { // crop shots to motion zone
                p1 = p1(camvars.zone);
                p2 = p2(camvars.zone);
            }
            
            absdiff(p1,p2, diff);
            cvtColor(diff, gray, cv::COLOR_BGR2GRAY);
            GaussianBlur( gray, blur, cv::Size( camvars.blur_aperture, camvars.blur_aperture ), 0, 0 );
            threshold(blur, thresh, camvars.binarize_threshold, 255, cv::THRESH_BINARY);
            dilate(thresh, dilated, cv::Mat(), cv::Point(-1,-1));
            findContours( dilated, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);//, cv::Point(0, 0) );
            
            if (calibrated) {
                for (const Contour &contour : contours) {
                    int csize = contourArea(contour);
                    if (csize >= camvars.min_contour_size) {
                        cv::Rect rect = cv::boundingRect(contour);
                        //int x = rect.x;
                        //int y = rect.y;
                        //int w = rect.width;
                        //int h = rect.height;
                        cv::Point center = find_center(rect, camvars.md_scale, camvars.zone);
                        cv::Rect scaled_rect = scale_rect(rect, camvars.md_scale, camvars.zone);
                        
                        //rectangle(feed, cv::Point(x*md_scale,y*md_scale), cv::Point((x+w)*md_scale,(y+h)*md_scale), cv::Scalar(0, 255, 0), 1);
                        //circle(feed, center, 1, cv::Scalar(0, 255, 0), 2);   
                        
                        tracked = 0;
                        for (const auto &idpair : tracking) {
                            string id = idpair.first;
                            if (point_in_bounds(center, tracking[id].arect)) {
                                
                                tracking[id].update(contour, camvars);
                                tracked = 1;
                                if (tracking[id].confidence >= confalert) {
                                    playalert = true;
                                    vid_motion = true;
                                    motiontime = time(0);
                                    if (tracking[id].confidence > maxconf) {
                                        maxconf = tracking[id].confidence;
                                        alerttrack = id;
                                    }
                                }
                            }
                        };
                        
                        if (tracked == 0 and not is_tracked(scaled_rect, tracking)) {
                            TrackObject to;
                            to.track_init(contour, camvars);
                            string tid = to.id;
                            tracking[tid] = to;
                            //cout << "new object: " << tid << " " << scaled_rect << endl; /////////////////////////////
                        }
                    }
                };
                
                tracks_n = 0;
                int tconf;
                vector<string> erasethese;
                for (const auto &idpair : tracking) {
                    string id = idpair.first;
                    if (not tracking[id].initialized) {continue;}
                    const int x = tracking[id].arect.x;
                    const int y = tracking[id].arect.y;
                    const int w = tracking[id].arect.width;
                    const int h = tracking[id].arect.height;
                    tconf = tracking[id].confidence;
                    if (tconf >= 1) {
                        
                        if (tconf >=3 ) {tracks_n+=1;} //3
                    
                        int rsize = 1;
                        cv::Scalar color = cv::Scalar(0,255,0);
                        cv::Scalar ccolor = cv::Scalar(0,255,0);
                        
                        if (tconf == 1) {
                            color = cv::Scalar(255,0,0);
                            ccolor = color;
                            rsize = 1;
                        }
                        else if (tconf == 2) {
                            color = cv::Scalar(0,255,0);
                            ccolor = color;
                            rsize = 1;
                        }
                        else if (tconf == 3) {
                            color = cv::Scalar(0,255,0);
                            ccolor = cv::Scalar(0,0,255);
                            rsize = 2;
                        }
                        else if (tconf == 4) {
                            color = cv::Scalar(0,255,0);
                            ccolor = color;
                            rsize = 2;
                        }
                        
                        rectangle(feed, cv::Point(x,y), cv::Point(x+w, y+h), color, rsize);
                        cv::putText(feed, id, cv::Point(x, y-1), cv::FONT_HERSHEY_PLAIN, 1, color, 1);
                        cv::putText(feed, tracking[id].classification, cv::Point(x, y+h+11), cv::FONT_HERSHEY_PLAIN, 1, color, 1);
                        circle(feed, tracking[id].center, 1, ccolor, 2);   
                    }
                    
                    if (time(0) > (tracking[id].time + camvars.tracktimes[tconf])) {
                        if (tconf >=3) { //>=3
                            notif(fmt::format("Track deleted: {} UPD: {} CONF: {} CLASS: {} VEL: {} SIZE: {} R: {} MAXCONF: {}",
                                id,
                                tracking[id].updates, 
                                tracking[id].confidence, 
                                tracking[id].classification,
                                tracking[id].vel,
                                tracking[id].size,
                                tracking[id].ratio,
                                maxconf), cam_id);
                        }
                        erasethese.push_back(id);
                    }
                    
                    int cx1 = tracking[id].center.x;
                    int cy1 = tracking[id].center.y;
                    for (const auto &idpair2 : tracking) {
                        string id2 = idpair2.first;
                        int cx2 = tracking[id2].center.x;
                        int cy2 = tracking[id2].center.y;
                        if (id != id2 and cx1==cx2 and cy1 == cy2 and tracking[id2].first >= tracking[id].first) {
                            if (tracking[id2].confidence >= 3 ) {
                                notif(fmt::format("Overwrite track {}",id2),cam_id);
                            } 
                            erasethese.push_back(id2);
                        }
                    };
                    
                };
                for (const string id : erasethese) {
                    tracking.erase(id);
                }
                erasethese.clear();
                
            }
            else {
                if ((time(0) - rt) > camvars.calibration_t) {
                    calibrated = true;
                }
            }
            
            if (camvars.serial_alert) {
                cv::putText(feed, "A", cv::Point(int(frame_width*0.95), int(frame_height*0.975)), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0,255,0), 2);
                if (maxconf<=1 and led!=1) {
                    led = 1;
                    WriteTextFile(".led",fmt::format("1:{}",cam_id));
                }
                else if (maxconf < 4 and maxconf >1 and led!=2) {
                    led = 2;
                    WriteTextFile(".led",fmt::format("2:{}",cam_id));
                }
                else if (maxconf >= 4 and led!=3) {
                    led = 3;
                    WriteTextFile(".led",fmt::format("3:{}",cam_id));
                }
            }
            
            if ((manual_record or (camvars.record and (maxconf >= record_tconf or (maxconf>=3 and tracks_n> 4)))) and not recording) {
                recording = true;
                //recordstart = time(0);
                notif("Recording started",cam_id);
            }
            else if (recording and not manual_record and maxconf < record_tconf and t-motiontime > 5){
                recording = false;
                notif("Recording stopped",cam_id);
            }
            
            if (vid_motion and not motion_notif) {
                notif("Motion alert",cam_id);
                motion_notif = true;
                notif(fmt::format("Tracking: {} UPD: {} CONF: {} CLASS: {} VEL: {} SIZE: {} R: {} MAXCONF: {}",
                    alerttrack,
                    tracking[alerttrack].updates, 
                    tracking[alerttrack].confidence, 
                    tracking[alerttrack].classification,
                    tracking[alerttrack].vel,
                    tracking[alerttrack].size,
                    tracking[alerttrack].ratio,
                    maxconf), cam_id);
            }
            
            if (vid_motion and (time(0)-timeplay > camvars.alert_delay)) {
                timeplay = time(0);
                if (camvars.alert_sound and playalert) {
                    notif("Play sound",cam_id);
                }
            }
            
            vidtime = timestamp();
            
            if (camvars.zone.width != 0) {
                rectangle(feed, cv::Point(camvars.zone.x * camvars.md_scale, camvars.zone.y * camvars.md_scale), 
                                cv::Point((camvars.zone.x + camvars.zone.width) * camvars.md_scale, (camvars.zone.y + camvars.zone.height) * camvars.md_scale), 
                                cv::Scalar(0,0,255), 2);
            }
            
            if (camvars.record) {
                cv::Scalar reccolor;
                if (recording) {reccolor = cv::Scalar(0,0,255);}
                else {reccolor = cv::Scalar(0,255,0);}
                cv::putText(feed, "RECORD", cv::Point(int(frame_width*0.01), int(frame_height*0.89)), cv::FONT_HERSHEY_PLAIN, 2, reccolor, 2);
            }
            
            if (vid_motion) {
                cv::putText(feed, "MOTION", cv::Point(int(frame_width*0.01), int(frame_height*0.1)), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0,0,255), 2);
            }
            
            if (camvars.alert_sound) {
                cv::putText(feed, "SOUND", cv::Point(int(frame_width*0.01), int(frame_height*0.95)), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0,255,0), 2);
            }
            
            if (camvars.resize_display) {
                cv::Size newwinsize;
                newwinsize.width = camvars.windowres.width;
                newwinsize.height = camvars.windowres.height;
                cv::resize(feed, feed, newwinsize, 0, 0, cv::INTER_AREA);
            }
            
            imshow( windowname, feed );

            int c = (char)cv::waitKey(1);
            if (c == 27) {
                notif("WatchCam stopped",cam_id);
                break;
            }
                
            if (c == 115) { //s
                camvars.alert_sound = not camvars.alert_sound;
            } 
            if (c == 97) { //a
                camvars.serial_alert = not camvars.serial_alert;
                if (not camvars.serial_alert) {
                    led = 1;
                    WriteTextFile(".led",fmt::format("1:{}",cam_id));
                }
            } 
            if (c == 99) {  //c
                camvars.zone.x = 0;
                camvars.zone.y = 0;
                camvars.zone.height = 0;
                camvars.zone.width = 0;
            }
            if (c==101) { //e
                if (not exclude_mode) {exclude_mode = true;}
                exclude_mode = not exclude_mode;
            }
            if (c==109) { //m
                if (not camvars.record) {camvars.record = true;}
                manual_record = not manual_record;
            }
            if (c==114) { //r
                camvars.record = not camvars.record;
            } 
            if (c==120) {
                notif("Tracking cleared",cam_id);
                tracking.clear();
                TrackMap tracking;
            }
            
            frame2 = frame1.clone();
            cap >> frame1;
            feed = frame1.clone();
            
            t = time(0);
            if (vid_motion and t-motiontime > 5) {
                rt = time(0);
                vid_motion = false;
                motion_notif = false;
                maxconf = 0;
            }

            if (frame1.empty())
                break;
        }
        

    }
    catch(const std::runtime_error& re)
    {
        // speciffic handling for runtime_error
        std::cerr << "Runtime error: " << re.what() << std::endl;
    }
    catch(const std::exception& ex)
    {
        // speciffic handling for all exceptions extending std::exception, except
        // std::runtime_error which is handled explicitly
        std::cerr << "Error occurred: " << ex.what() << std::endl;
    }
    catch(...)
    {
        // catch any other errors (that we have no information about)
        std::cerr << "Unknown failure occurred. Possible memory corruption" << std::endl;
    }
    
    cap.release();
    cv::destroyAllWindows();
    
    return 0;

}


