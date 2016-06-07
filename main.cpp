#include <iostream>
#include <map>
#include "SpeedTest.h"
#include "TestConfigTemplate.h"

void banner(){
    std::cout << "SpeedTest++ version " << SpeedTest_VERSION_MAJOR << "." << SpeedTest_VERSION_MINOR << std::endl;
    std::cout << "speedtest.net command line interface" << std::endl;
    std::cout << "Author: " << SpeedTest_AUTHOR << std::endl;
}

void usage(const char* name){
    std::cout << "usage: " << name << " ";
    std::cout << "[--latency] [--download] [--upload] [--help]" << std::endl;
    std::cout << "optional arguments:" << std::endl;
    std::cout << "\t--help      Show this message and exit\n";
    std::cout << "\t--latency   Perform latency test only\n";
    std::cout << "\t--download  Perform download test only. It includes latency test\n";
    std::cout << "\t--upload    Perform upload test only. It includes latency test\n";
    std::cout << "\t--share     Generate and provide a URL to the speedtest.net share results image\n";
}

int main(const int argc, const char **argv) {

    bool download_only = false;
    bool upload_only   = false;
    bool latency_only  = false;
    bool share         = false;

    std::vector<std::string> options;

    for (int i = 0; i < argc; i++)
        options.push_back(std::string(argv[i]));

    banner();
    std::cout << std::endl;

    if (std::find(options.begin(), options.end(), "--help") != options.end()) {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if (std::find(options.begin(), options.end(), "--latency") != options.end())
        latency_only = true;

    if (std::find(options.begin(), options.end(), "--download") != options.end())
        download_only = true;

    if (std::find(options.begin(), options.end(), "--upload") != options.end())
        upload_only = true;

    if (std::find(options.begin(), options.end(), "--share") != options.end())
        share = true;


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
    std::cout << "Ping: " << sp.latency() << " ms." << std::endl;

    long tjitter = 0;
    std::cout << "Jitter: " << std::flush;
    if (sp.jitter(serverInfo, tjitter)){
        std::cout << tjitter << " ms." << std::endl;
    } else {
        std::cout << "Jitter measurement is unable at this time." << std::endl;
    }

    if (latency_only)
        return EXIT_SUCCESS;


    std::cout << "Determine line type (" << preflightConfigDownload.concurrency << ") "  << std::flush;
    double preSpeed = 0;
    if (!sp.downloadSpeed(serverInfo, preflightConfigDownload, preSpeed)){
        std::cerr << "Pre-flight check failed." << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << std::endl;

    TestConfig uploadConfig   = slowConfigUpload;
    TestConfig downloadConfig = slowConfigDownload;
    if (preSpeed <= 4){
        std::cout << "Very-slow-line line type detected: profile selected slowband" << std::endl;
    } else if (preSpeed > 4 && preSpeed <= 30){
        std::cout << "Buffering-lover line type detected: profile selected narrowband" << std::endl;
        downloadConfig = narrowConfigDownload;
        uploadConfig   = narrowConfigUpload;
    } else if (preSpeed > 30 && preSpeed < 150) {
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
        double downloadSpeed = 0;
        if (sp.downloadSpeed(serverInfo, downloadConfig, downloadSpeed)){
            std::cout << std::endl;
            std::cout << "Download: " << downloadSpeed << " Mbit/s" << std::endl;
        } else {
            std::cerr << "Download test failed." << std::endl;
            return EXIT_FAILURE;
        }


    }

    if (download_only)
        return EXIT_SUCCESS;


    std::cout << "Testing upload speed (" << uploadConfig.concurrency << ") "  << std::flush;
    double uploadSpeed = 0;
    if (sp.uploadSpeed(serverInfo, uploadConfig, uploadSpeed)){
        std::cout << std::endl;
        std::cout << "Upload: " << uploadSpeed << " Mbit/s" << std::endl;
    } else {
        std::cerr << "Upload test failed." << std::endl;
        return EXIT_FAILURE;
    }


    if (share){
        std::string share_it;
        if (sp.share(serverInfo, share_it)){
            std::cout << "Results image: " << share_it << std::endl;
        }
    }

    return EXIT_SUCCESS;
}