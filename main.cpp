#include <iostream>
#include <map>
#include "SpeedTest.h"
#include "SpeedTestClient.h"
#include <mutex>
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

    auto concurrency = std::thread::hardware_concurrency();
    if (concurrency <= 0)
        concurrency = 4;

    std::cout << "Testing download speed (" << concurrency << ") "  << std::flush;
    std::vector<std::thread> workers;
    float overall_speed = 0;
    for (int i = 0; i < concurrency; i++) {
        workers.push_back(std::thread([&serverInfo, &overall_speed](){
            std::vector<long> samples = {1000000, 2000000, 3500000, 5000000, 7000000, 10000000};
            auto spClient = SpeedTestClient(serverInfo);

            if (spClient.connect()) {
                long total_size = 0;
                long total_time = 0;
                for (auto const &s : samples) {
                    long download_time;
                    if (spClient.download(s, &download_time)) {
                        total_size += s;
                        total_time += download_time;
                        std::cout << "." << std::flush;
                    } else {
                        std::cout << "E" << std::flush;
                    }
                }
                float speed = ((total_size * 8) / 1000 / 1000) / (static_cast<float>(total_time) / 1000);
                mtx.lock();
                overall_speed += speed;
                mtx.unlock();
                spClient.close();

            } else {
                std::cout << "E" << std::flush;
            }

        }));
    }

    std::for_each(workers.begin(), workers.end(),[](std::thread &t){
        t.join();
    });
    std::cout << std::endl;
    std::cout << "Download speed: " << overall_speed << " MBit/s" << std::endl;

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