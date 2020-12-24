#include "recordingSystem.h"
#include <thread>
#include <iostream>
#include<stdio.h>
#include <fstream>
#include <WS2tcpip.h>
#include <string>
#pragma comment (lib, "ws2_32.lib")
using namespace std;


int main(int argc, char** argv) {
    // Initialize and run the listener threads
    thread channel_1_listener(channel1_listen);
    thread channel_2_listener(channel2_listen);
    cout << "Two threads are now liseting to channels 1 & 2" << endl;
    channel_2_listener.join();
    channel_1_listener.join();
}

void channel1_listen() {
    // UDP connection
    WSADATA data;
    WORD version = MAKEWORD(2, 2);
    int wsOk = WSAStartup(version, &data);
    if (wsOk != 0) {
        cout << "Can't start Winsock! " << wsOk;
        return;
    }
    SOCKET in = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in serverHint;
    serverHint.sin_addr.S_un.S_addr = ADDR_ANY; // Us any IP address available on the machine
    serverHint.sin_family = AF_INET; // Address format is IPv4
    serverHint.sin_port = htons(54000); // Convert from little to big endian
    if (bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR) {
        cout << "Can't bind socket! " << WSAGetLastError() << endl;
        return;
    }
    sockaddr_in client; // Use to hold the client information (port / ip address)
    int clientLength = sizeof(client); // The size of the client information


    // Receiving loop
    while (true) {
        // Receive message to the data buffer
        ZeroMemory(&client, clientLength); // Clear the client structure
        ZeroMemory(data_buff, 1024); // Clear the receive buffer
        int bytesIn = recvfrom(in, data_buff, 1024, 0, (sockaddr*)&client, &clientLength); // Wait for message
        if (bytesIn == SOCKET_ERROR) {
            cout << "Error receiving from client " << WSAGetLastError() << endl;
            continue;
        }
       

        // Convert buffer to a data package struct
        struct data_package* d;
        d = (data_package*)malloc(sizeof(struct data_package));
        if (d != NULL) {
            memcpy(d, &data_buff, sizeof(data_package));
            cout << "Listener 1: recv a data package with id: " << d->id << endl;
        }
        else {
            cout << "Listener 1: malloc didn't work" << endl;
        }
 

        // Insert the package to the main data structure- the hash map:
        // If the header package with this id allready arrived
        map_mtx.lock();
        if (mp.find(d->id) != mp.end()) {
            mp[d->id].second.push_back(d); //add the data package to the data pakages vector
        }

        // If the header package with this id DID NOT arrive yet
        else {
            vector<struct data_package*> vec; //build an empty data package vector
            vec.push_back(d); //add the data package to the vector
            mp[d->id] = make_pair(nullptr, vec); // insert the new record to the hashmap
        }
        map_mtx.unlock();

        // If all packages for this id arrived at this point
        if (received_all_data(d->id)) {
            write_to_disk(d->id); //save the header+data packages to disk
            mp.erase(d->id); //delete record from map 
        } 
    }
}

void channel2_listen() {
    // UDP connection
    WSADATA data;
    WORD version = MAKEWORD(2, 2);
    int wsOk = WSAStartup(version, &data);
    if (wsOk != 0) {
        cout << "Can't start Winsock! " << wsOk;
        return;
    }
    SOCKET in = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in serverHint;
    serverHint.sin_addr.S_un.S_addr = ADDR_ANY; // Us any IP address available on the machine
    serverHint.sin_family = AF_INET; // Address format is IPv4
    serverHint.sin_port = htons(64000); // Convert from little to big endian
    if (bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR) {
        cout << "Can't bind socket! " << WSAGetLastError() << endl;
        return;
    }
    sockaddr_in client; // Use to hold the client information (port / ip address)
    int clientLength = sizeof(client); // The size of the client information


    // Receiving loop
    while (true) {
        // Receive message to the data buffer
        ZeroMemory(&client, clientLength); // Clear the client structure
        ZeroMemory(header_buff, 1024); // Clear the receive buffer
        int bytesIn = recvfrom(in, header_buff, 1024, 0, (sockaddr*)&client, &clientLength);// Wait for message
        if (bytesIn == SOCKET_ERROR) {
            cout << "Error receiving from client " << WSAGetLastError() << endl;
            continue;
        }
        

        // Convert buffer to a header package struct
        struct header_package* h;
        h = (header_package*)malloc(sizeof(struct header_package));
        if (h != NULL) {
            memcpy(h, &header_buff, sizeof(header_package));
            cout << "Listener 2: recv a header package with id: " << h->id << endl;
        }
        else {
            cout << "Listener 2: malloc didn't work" << endl;
        }


        // Insert the package to the main data structure- the hash map:
        // If a data package with this id allready arrived
        map_mtx.lock();
        if (mp.find(h->id) != mp.end()) {
            mp[h->id].first = h; //insert the header package pointer to its place
        }

        // If a data package with this id DID NOT arrive yet
        else {
            vector<struct data_package*> vec; //build an empty data package vector
            mp[h->id] = make_pair(h, vec); //insert a new record with this id to the map
        }
        map_mtx.unlock();

        // If all packages for this id arrived at this point
        if (received_all_data(h->id)) {
            write_to_disk(h->id); //save the header+data packages to disk
            mp.erase(h->id); //delete record from map 
        }
    }
}

bool received_all_data(int id) {
    map_mtx.lock();
    header_package* h = mp[id].first;
    if (h != nullptr) { //if the header arrived
        if (h->num_of_data_packages == mp[id].second.size()) { //if all the data packages arrived
            map_mtx.unlock();
            return true;
        }
        else {
            map_mtx.unlock();
            return false;
        }
    }
    else {
        map_mtx.unlock();
        return false;
    }
}

void write_to_disk(int id) {
    output_mtx.lock();
    cout << "write to disk" << endl;
    ofstream myfile;
    myfile.open("output.txt", ios::app);
    myfile << "---------------------------------------------------" << endl;
    //write header
    string hd = "";
    for (int i = 0; i < 10; i++) {
        hd = hd + to_string(mp[id].first->data[i]);
    }
    myfile << "---------HEADER PACKAGE---------\n ID: " << id << "\nNUM OF DATA PACKAGES: " << mp[id].first->num_of_data_packages << "\nDATA: " << hd << endl;
    //write data
    for (struct data_package* d :mp[id].second) {
        string dd = "";
        for (int i = 0; i < 124; i++) {
            dd = dd + to_string(d->data[i]);
        }
        myfile << "-----DATA PACKAGE-----\n ID: " << id << "\nDATA: " << dd << endl;
    }
    myfile.close();
    output_mtx.unlock();
}
