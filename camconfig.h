#include <cstdlib>
#include <libconfig.h++>
#include <opencv2/opencv.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/core/mat.hpp>

using namespace std;

#ifndef CAM_CONFIG_CLASS_H
#define CAM_CONFIG_CLASS_H
namespace CamConfigClass {
    
    class CamConfig {
        public:
            bool record;
            int record_time;
            string record_dir;
            int alert_delay;
            bool alert_sound;
            string alert_sound_file;
            bool internet_cam;
            string internet_cam_url;
            int vidfps;
            int min_contour_size;
            int binarize_threshold;
            int blur_aperture;
            int motion_frames_min;
            double md_scale;
            bool save_tracks;
            int buffer_len;
            double calibration_t;
            bool resize_display;
            bool time_mark;
            bool serial_alert;
            string serial_device;
            cv::Size windowres;
            cv::Size motionimgres;
            cv::Rect zone;
            string cam_urls[8];
            cv::Rect cam_zones[8];
            double tracktimes[5];
            
            void Load(const char* filename) {
                libconfig::Config config;
                try{
                    config.readFile (filename);
                } catch (libconfig::FileIOException &e){
                    cerr << "FileIOException occurred. Could not read can.cfg!\n";
                    exit (EXIT_FAILURE);
                } catch (libconfig::ParseException &e){
                    cerr << "Parse error at " << e.getFile() << ":" << e.getLine()
                              << " - " << e.getError() << endl;
                    exit(EXIT_FAILURE);
                }

                try {
                    // load config into variables
                    record = config.lookup("record");
                    record_time = config.lookup("record_time");
                    record_dir = config.lookup("record_dir").c_str();
                    alert_delay = config.lookup("alert_delay");
                    alert_sound = config.lookup("alert_sound");
                    alert_sound_file = config.lookup("alert_sound_file").c_str();
                    internet_cam = config.lookup("internet_cam");
                    internet_cam_url = config.lookup("internet_cam_url").c_str();
                    vidfps = config.lookup("vidfps");
                    min_contour_size = config.lookup("min_contour_size");
                    blur_aperture = config.lookup("blur_aperture");
                    binarize_threshold = config.lookup("binarize_threshold");
                    motion_frames_min = config.lookup("motion_frames_min");
                    md_scale = config.lookup("md_scale");
                    save_tracks = config.lookup("save_tracks");
                    buffer_len = config.lookup("buffer_len");
                    calibration_t = config.lookup("calibration_t");
                    resize_display = config.lookup("resize_display");
                    time_mark = config.lookup("time_mark");
                    serial_alert = config.lookup("serial_alert");
                    serial_device = config.lookup("serial_device").c_str();
            
                    const libconfig::Setting &tt = config.lookup("tracktimes");
                    for (int n = 0; n < tt.getLength(); ++n) {
                        tracktimes[n] = tt[n];
                    }

                    windowres.width = config.lookup("windowres")[0];
                    windowres.height = config.lookup("windowres")[1];
                    
                    motionimgres.width = config.lookup("motionimgres")[0];
                    motionimgres.height = config.lookup("motionimgres")[1];
                    
                    zone.x = config.lookup("zone")[0];
                    zone.y = config.lookup("zone")[1];
                    zone.width = config.lookup("zone")[2];
                    zone.height = config.lookup("zone")[3];
                    
                    /*const libconfig::Setting &zl = config.lookup("cam_zones");
                    cam1_zone.x = zl[0][0];
                    cam1_zone.y = zl[0][1];
                    cam1_zone.width = zl[0][2];
                    cam1_zone.height = zl[0][3];
                    cam2_zone.x = zl[1][0];
                    cam2_zone.y = zl[1][1];
                    cam2_zone.width = zl[1][2];
                    cam2_zone.height = zl[1][3];
                    cam3_zone.x = zl[2][0];
                    cam3_zone.y = zl[2][1];
                    cam3_zone.width = zl[2][2];
                    cam3_zone.height = zl[2][3];
                    cam4_zone.x = zl[3][0];
                    cam4_zone.y = zl[3][1];
                    cam4_zone.width = zl[3][2];
                    cam4_zone.height = zl[3][3];*/
                    
                    const libconfig::Setting &camurls = config.lookup("cam_urls");
                    for (int n = 0; n < camurls.getLength(); ++n) {
                        cam_urls[n] = camurls[n].c_str();
                    }
                    
                    const libconfig::Setting &camzones = config.lookup("cam_zones");
                    for (int n = 0; n < camzones.getLength(); ++n) {
                        cam_zones[n] = cv::Rect(camzones[n][0],camzones[n][1],camzones[n][2],camzones[n][3]);
                    }
                    
                    
                    
                } catch(libconfig::SettingNotFoundException &e) {
                    cerr << "Error in configuration file" << endl;
                    exit (EXIT_FAILURE);
                }
            }
    };

}
#endif
