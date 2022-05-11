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

    /* initialize serial port */    
    string port = "COM3";
    unsigned long baud = 9600;

    // port, baudrate, timeout in milliseconds
    cout << "initializing serial port..." << endl;
    serial::Serial my_serial(port, baud, serial::Timeout::simpleTimeout(1000));

    if(my_serial.isOpen())
    {
        cout << "serial port opened" << endl;
    }
    else
    {
        cout << "serial port failed to open" << endl;
        cout << " Available ports:" << endl;
        enumerate_ports();
    }

    /* initialize tcp socket */
    cout << "initializing tcp socket..." << endl;
    // create a socket
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock == INVALID_SOCKET)
    {
        cerr << "Error: could not create socket" << endl;
        return 1;
    }

    // set the address of the server
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8000);
    
    // bind the socket to the address
    cout << "binding socket to address..." << endl;
    if(bind(sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        cerr << "bind failed" << endl;
        return 1;
    }

    // listen for incoming connections
    cout << "listening for connections..." << endl;
    if(listen(sock, SOMAXCONN) == SOCKET_ERROR)
    {
        cerr << "listen failed" << endl;
        return 1;
    }

    // accept a connection
    cout << "accepting connection..." << endl;
    sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
    SOCKET client_sock = accept(sock, (sockaddr*)&client_addr, &client_addr_len);
    if(client_sock == INVALID_SOCKET)
    {
        cerr << "accept failed" << endl;
        return 1;
    }  
    cout << "connected\n" << endl;

    /* forward data */
    // read data from the serial port
    string data;
    cout << "reading data from serial port and forwarding to tcp socket..." << endl;
    while(1)
    {
        data = my_serial.read(100);
        if(data.length() > 0)
        {
            // send data to the client
            send(client_sock, data.c_str(), data.length(), 0);
            cout << data;
        }
    }
    return 0;
}