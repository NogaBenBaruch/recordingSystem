#include "client.h"
#include <iostream>
#include <WS2tcpip.h>
#include <thread>
#pragma comment (lib, "ws2_32.lib")
using namespace std;


int main(int argc, char** argv) {
	// Create a random ids array, and a random number of the data packages for each header array
	for (int i = 0; i < NUM_OF_HEADERS; i++) {
		ids_arr[i] = rand() % 100; //ids can be 0-99
		num_of_dp_arr[i] = rand() % 6; //number of data arrays for each header can be 0-5
	}

	// Initialize and run the sender threads 
	thread channel_1_sender(channel1_send);
	thread channel_2_sender(channel2_send);
	channel_1_sender.join();
	channel_2_sender.join();
}

void channel1_send(){
	// UDP connection
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	int wsOk = WSAStartup(version, &data);
	if (wsOk != 0){
		cout << "Can't start Winsock! " << wsOk;
		return;
	}
	sockaddr_in server;
	server.sin_family = AF_INET; // AF_INET = IPv4 addresses
	server.sin_port = htons(54000); // Little to big endian conversion
	inet_pton(AF_INET, "127.0.0.1", &server.sin_addr); // Convert from string to byte array
	SOCKET out = socket(AF_INET, SOCK_DGRAM, 0);


	// Create data packages and send
	for (int i = NUM_OF_HEADERS-1 ; i > -1; i--) {
		for (int j = 0; j<num_of_dp_arr[i]; j++) {
			struct data_package* d;
			d = (data_package*)malloc(sizeof(struct data_package));
			d->id = ids_arr[i];
			for (int k = 0; k < 124; k++) {
				d->data[k] = rand();
			}
			int sendOk = sendto(out, (const char*)d, sizeof(struct data_package), 0, (sockaddr*)&server, sizeof(server));
			if (sendOk == SOCKET_ERROR) {
				cout << "That didn't work! " << WSAGetLastError() << endl;
			}
			cout << "Sender 1: send a data package with id: " << d->id  << endl;
		}
	}


	// Close socket and Winsock
	closesocket(out);
	WSACleanup();
}

void channel2_send() {
	// UDP connection
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	int wsOk = WSAStartup(version, &data);
	if (wsOk != 0) {
		cout << "Can't start Winsock! " << wsOk;
		return;
	}
	sockaddr_in server;
	server.sin_family = AF_INET; // AF_INET = IPv4 addresses
	server.sin_port = htons(64000); // Little to big endian conversion
	inet_pton(AF_INET, "127.0.0.1", &server.sin_addr); // Convert from string to byte array
	SOCKET out = socket(AF_INET, SOCK_DGRAM, 0);


	// Create header packages and send 
	for (int i = 0; i < NUM_OF_HEADERS; i++) {
		struct header_package* h;
		h = (header_package*)malloc(sizeof(struct header_package));
		h->id = ids_arr[i];
		h->num_of_data_packages = num_of_dp_arr[i];
		for (int j = 0; j < 10; j++) {
			h->data[j] = rand();
		}
		int sendOk = sendto(out, (const char*)h, sizeof(struct header_package), 0, (sockaddr*)&server, sizeof(server));
		if (sendOk == SOCKET_ERROR) {
			cout << "That didn't work! " << WSAGetLastError() << endl;
		}
		cout << "Sender 2: send a header package with id: " << h->id << endl;
	}


	// Close socket and Winsock
	closesocket(out);
	WSACleanup();
}