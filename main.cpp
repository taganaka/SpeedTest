#include <iostream>
#include <map>
#include "SpeedTest.h"
#include "SpeedTestClient.h"
#include <thread>
std::mutex mtx;
int main() {

    auto sp = SpeedTest();
    IPInfo info;
    if (!sp.ipInfo(&info)){
        return EXIT_FAILURE;
    }

    std::cout << "IP: " << info.ip_address << std::endl;
    std::cout << "Lat / Lon: " << info.lat << " / " << info.lon << std::endl;
    std::cout << "Provider: " << info.isp  << std::endl;
    std::cout << "Finding closest server..." << std::endl;
    auto serverList = sp.serverList();
    std::cout << serverList.size() << " Servers online" << std::endl;
    std::cout << "Closest: " << serverList[0].name << " (" << serverList[0].distance << " km)" << std::endl;

    std::cout << "Finding fastest server..." << std::endl;
    ServerInfo serverInfo = sp.bestServer(5);
    std::cout << "Fastest: " << serverInfo.name << " (" << serverInfo.distance << " km)" << std::endl;

    std::cout << "Latency: " << sp.latency() << " ms" << std::endl;

    std::vector<std::thread> workers;
    float overall_speed = 0;
    for (int i = 0; i < 32; i++) {
        workers.push_back(std::thread([&serverInfo, &overall_speed](){

            auto spClient = SpeedTestClient(serverInfo);

            if (spClient.connect()) {
                std::cout << "Connected to " << serverInfo.host << std::endl;
                long download_time;
                if (spClient.download(20000000, &download_time)) {
                    float speed = ((20000000 * 8) / 1000 / 1000) / (static_cast<float>(download_time) / 1000);
                    std::cout << "Download complete: " << download_time << " Speed: " << speed << std::endl;
                    mtx.lock();
                    overall_speed += speed;
                    mtx.unlock();

                }
                spClient.close();
            }
        }));
    }

    std::for_each(workers.begin(), workers.end(),[](std::thread &t){
        t.join();
    });
    std::cout << "Total speed: " << overall_speed << std::endl;

//    auto spClient = SpeedTestClient(serverInfo);
//
//    if (spClient.connect()){
//        std::cout << "Connected to " << serverInfo.host << std::endl;
//        long download_time;
//        if (spClient.download(50000000, &download_time)){
//            std::cout << "Download complete: " << download_time << std::endl;
//
//        }
//
//        spClient.close();
//    }
    return 0;
}