#include <iostream>
#include <cstdlib>
#include <fstream>
#include <SerialPort.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <chrono>
#include <thread>
#include "camconfig.h"

using namespace LibSerial;
using namespace CamConfigClass;

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

int main() {
    const char* configfile = "cam.cfg";
    const char* ledfile = ".led";
    SerialPort my_serial_port;
    CamConfig camvars;
    camvars.Load(configfile);

    cout << "Connecting serial device " << camvars.serial_device << endl;
    try{
        my_serial_port.Open( camvars.serial_device ) ;
        my_serial_port.SetBaudRate( BaudRate::BAUD_9600) ;
        my_serial_port.SetCharacterSize( LibSerial::CharacterSize::CHAR_SIZE_8 ) ;
        my_serial_port.SetFlowControl( LibSerial::FlowControl::FLOW_CONTROL_NONE ) ;
        my_serial_port.SetParity( LibSerial::Parity::PARITY_NONE ) ;
        my_serial_port.SetStopBits(LibSerial::StopBits::STOP_BITS_1) ;
        const int BUFFER_SIZE = 9;
        string input_buffer;
        my_serial_port.Read( input_buffer, BUFFER_SIZE );
        if (input_buffer == "LED_READY") {
            cout << "Serial Alarm Connected" << endl;
        }
    }
    catch(const std::runtime_error& re)
    {
        cout << "Serial device busy" << endl;
        camvars.serial_alert = false;
    }
    
    int lastled = 1;
    int lastchange = time(0);
    string ledc = ReadTextFile(ledfile);
    //WriteTextFile(".led","1");
    
    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ledc = ReadTextFile(ledfile);
        
        if (ledc[0]== '1' and lastled!=1) {
            lastled = 1;
            my_serial_port.Write("1\0");
            cout << "Led Off" << endl;
        }
        else if (ledc[0]== '2' and lastled!=2) {
            lastled = 2;
            my_serial_port.Write("2\0");
            cout << "Led On" << endl;
        }
        else if (ledc[0]== '3' and lastled!=3) {
            lastled = 3;
            my_serial_port.Write("3\0");
            cout << "Led Flash" << endl;
        }
        else {
            lastled = 1;
            my_serial_port.Write("1\0");
            cout << "Led Off Undefined" << endl;
        }
            
        
    }
        
    my_serial_port.Write("1\0");
    my_serial_port.Close();

}
