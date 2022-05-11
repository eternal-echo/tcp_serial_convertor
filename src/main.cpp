#include <string>
#include <iostream>
#include <cstdio>
#include <WinSock2.h>
#include "serial/serial.h"

using std::string;
using std::exception;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;



void enumerate_ports()
{
	vector<serial::PortInfo> devices_found = serial::list_ports();

	vector<serial::PortInfo>::iterator iter = devices_found.begin();

	while( iter != devices_found.end() )
	{
		serial::PortInfo device = *iter++;

		printf( "(%s, %s, %s)\n", device.port.c_str(), device.description.c_str(),
     device.hardware_id.c_str() );
	}
}

int main(int argc, char **argv)
{
    cout << "tcp serial converter" << endl;
    cout << "=====================" << endl;

    string port = "COM3";
    unsigned long baud = 9600;

    // port, baudrate, timeout in milliseconds
    serial::Serial my_serial(port, baud, serial::Timeout::simpleTimeout(1000));

    cout << "Is the serial port open?";
    if(my_serial.isOpen())
    {
        cout << " Yes." << endl;
    }
    else
    {
        cout << " No." << endl;
        cout << " Available ports:" << endl;
        enumerate_ports();
    }

    // read data from the serial port
    string data;
    while(1)
    {
        data = my_serial.read(1);
        if(data.length() > 0)
        {
            cout << data;
        }
    }
    return 0;
}