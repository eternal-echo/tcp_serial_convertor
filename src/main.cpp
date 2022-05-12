#include <string>
#include <iostream>
#include <cstdio>
#include <thread>
#include <functional>
#include <WinSock2.h>
#include "serial/serial.h"

using std::string;
using std::exception;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::thread;




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

// thread function for reposting data from serial port to tcp client
void tcp_repost(serial::Serial &serial_port, SOCKET &client_socket)
{
    char *buffer = new char[1024];
    int bytes_read;
    memset(buffer, 0, 1024);

    while(1)
    {
        // read data from serial port
        bytes_read = serial_port.read((uint8_t *)buffer, 1024);

        if(bytes_read > 0)
        {
            // send data to tcp client
            send(client_socket, buffer, bytes_read, 0);

            // print data to console
            cout.write(buffer, bytes_read);

            // clear buffer
            memset(buffer, 0, 1024);

            // sleep for a bit
            // std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

// thread function for reposting data from tcp client to serial port
void serial_repost(serial::Serial &serial_port, SOCKET &client_socket)
{
    char *buffer = new char[1024];
    int bytes_read;

    while(1)
    {
        // read data from tcp client
        bytes_read = recv(client_socket, buffer, 1024, 0);

        if(bytes_read > 0)
        {
            // send data to serial port
            serial_port.write((uint8_t *)buffer, bytes_read);

            // print data to console
            cout.write(buffer, bytes_read);

            // clear buffer
            memset(buffer, 0, 1024);
        }
    }
}




int main()
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
    cout << "connected" << endl;
    cout << endl;

    /* forward data */
    cout << "transfering data..." << endl;

    // repost data from serial port to tcp client
    thread serial_to_tcp(tcp_repost, std::ref(my_serial), std::ref(client_sock));

    // repost data from tcp client to serial port
    thread tcp_to_serial(serial_repost, std::ref(my_serial), std::ref(client_sock));

    // wait for threads to finish
    serial_to_tcp.join();
    tcp_to_serial.join();

    return 0;
}
/*

*/