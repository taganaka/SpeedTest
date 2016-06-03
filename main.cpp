#include <iostream>
#include <map>
#include "SpeedTest.h"
#include "TestConfigTemplate.h"

int main(const int argc, const char **argv) {

    bool download_only = false;
    bool upload_only   = false;
    bool latency_only  = false;

    std::vector<std::string> options;

    for (int i = 0; i < argc; i++)
        options.push_back(std::string(argv[i]));

    if (std::find(options.begin(), options.end(), "--latency") != options.end())
        latency_only = true;

    if (std::find(options.begin(), options.end(), "--download") != options.end())
        download_only = true;

    if (std::find(options.begin(), options.end(), "--upload") != options.end())
        upload_only = true;

    auto sp = SpeedTest();
    IPInfo info;
    if (!sp.ipInfo(&info)){
        std::cerr << "Unable to retrieve your IP info. Try again later" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "IP: " << info.ip_address
        << " ( " << info.isp << " ) "
        << "Location: [" << info.lat << ", " << info.lon << "]" << std::endl;

    std::cout << "Finding fastest server... " << std::flush;
    auto serverList = sp.serverList();
    if (serverList.empty()){
        std::cerr << "Unable to download server list. Try again later" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << serverList.size() << " Servers online" << std::endl;


    ServerInfo serverInfo = sp.bestServer(10);
    std::cout << std::endl;
    std::cout << "Server: " << serverInfo.name
        << " by " << serverInfo.sponsor
        << " (" << serverInfo.distance << " km from you): "
        << sp.latency() << " ms" << std::endl;

    if (latency_only)
        return EXIT_SUCCESS;


    std::cout << "Determine line type (" << preflightConfigDownload.concurrency << ") "  << std::flush;
    auto preSpeed = sp.downloadSpeed(serverInfo, preflightConfigDownload);
    std::cout << std::endl;

    TestConfig uploadConfig   = slowConfigUpload;
    TestConfig downloadConfig = slowConfigDownload;
    if (preSpeed <= 4){
        std::cout << "Very-slow-line line type detected: profile selected slowband" << std::endl;
    } else if (preSpeed > 4 && preSpeed <= 20){
        std::cout << "Buffering-lover line type detected: profile selected narrowband" << std::endl;
        downloadConfig = narrowConfigDownload;
        uploadConfig   = narrowConfigUpload;
    } else if (preSpeed > 20 && preSpeed < 150) {
        std::cout << "Broadband line type detected: profile selected broadband" << std::endl;
        downloadConfig = broadbandConfigDownload;
        uploadConfig   = broadbandConfigUpload;
    } else if (preSpeed >= 150) {
        std::cout << "Fiber / Lan line type detected: profile selected fiber" << std::endl;
        downloadConfig = fiberConfigDownload;
        uploadConfig   = fiberConfigUpload;
    }

    if (!upload_only){
        std::cout << std::endl;
        std::cout << "Testing download speed (" << downloadConfig.concurrency << ") "  << std::flush;
        auto downloadSpeed = sp.downloadSpeed(serverInfo, downloadConfig);

        std::cout << std::endl;
        std::cout << "Download: " << downloadSpeed << " Mbit/s" << std::endl;
    }

    if (download_only)
        return EXIT_SUCCESS;


    std::cout << "Testing upload speed (" << uploadConfig.concurrency << ") "  << std::flush;
    auto uploadSpeed = sp.uploadSpeed(serverInfo, uploadConfig);
    std::cout << std::endl;
    std::cout << "Upload: " << uploadSpeed << " Mbit/s" << std::endl;


    return EXIT_SUCCESS;
}