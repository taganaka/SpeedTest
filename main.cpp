#include <iostream>
#include <map>
#include "SpeedTest.h"
#include "SpeedTestClient.h"


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

    TestConfig downloadConfig;
    downloadConfig.buff_size = 65536;
    downloadConfig.concurrency = concurrency;
    downloadConfig.start_size = 1000000;
    downloadConfig.incr_size = 750000;
    downloadConfig.min_test_time_ms = 20000;
    downloadConfig.max_size = 100000000;


    TestConfig uploadConfig;
    uploadConfig.buff_size = 65536;
    uploadConfig.concurrency = concurrency / 2;
    uploadConfig.start_size = 1000000;
    uploadConfig.incr_size = 250000;
    uploadConfig.min_test_time_ms = 20000;
    uploadConfig.max_size = 70000000;

    std::cout << "Testing download speed (" << downloadConfig.concurrency << ") "  << std::flush;
    auto downloadSpeed = sp.downloadSpeed(serverInfo, downloadConfig);

    std::cout << std::endl;
    std::cout << "Download speed: " << downloadSpeed << " MBit/s" << std::endl;
    std::cout << std::endl;

    std::cout << "Testing upload speed (" << uploadConfig.concurrency << ") "  << std::flush;
    auto uploadSpeed = sp.uploadSpeed(serverInfo, uploadConfig);
    std::cout << std::endl;
    std::cout << "Upload speed: " << uploadSpeed << " MBit/s" << std::endl;
    std::cout << std::endl;
    
    return 0;
}