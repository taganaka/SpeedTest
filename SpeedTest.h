//
// Created by Francesco Laurita on 5/29/16.
//

#ifndef SPEEDTEST_SPEEDTEST_H
#define SPEEDTEST_SPEEDTEST_H

#include "SpeedTestConfig.h"
#include <libxml/xmlreader.h>
#include <math.h>
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>


static const float EARTH_RADIUS_KM = 6371.0;

typedef struct ip_info_t {
    std::string ip_address;
    std::string isp;
    float lat;
    float lon;
} IPInfo;

typedef struct server_info_t {
    std::string url;
    std::string name;
    std::string country;
    std::string country_code;
    std::string host;
    std::string sponsor;
    int   id;
    float lat;
    float lon;
    float distance;

} ServerInfo;

typedef struct test_config_t {
    long start_size;
    long max_size;
    long incr_size;
    long buff_size;
    long min_test_time_ms;
    int  concurrency;
} TestConfig;

class SpeedTestClient;
typedef bool (SpeedTestClient::*opFn)(const long size, const long chunk_size, long &millisec);
class SpeedTest {
public:
    SpeedTest();
    ~SpeedTest();
    CURLcode httpGet(const std::string& url, std::stringstream& os, CURL *handler = nullptr, long timeout = 30);
    CURLcode httpPost(const std::string& url, const std::string& postdata, std::stringstream& os, CURL *handler = nullptr, long timeout = 30);
    static std::map<std::string, std::string> parseQueryString(const std::string& query);
    static std::vector<std::string> splitString(const std::string& instr, const char separator);
    bool ipInfo(IPInfo *info);
    const std::vector<ServerInfo>& serverList();
    const ServerInfo bestServer(const int sample_size = 5);
    const double &latency();
    const double downloadSpeed(const ServerInfo& server, const TestConfig& config);
    const double uploadSpeed(const ServerInfo& server, const TestConfig& config);
    const double jitter(const ServerInfo& server, const int sample = 40);
    bool share(const ServerInfo& server, std::string& image_url);
private:
    static CURL* curl_setup(CURL* curl = nullptr);
    static size_t writeFunc(void* buf, size_t size, size_t nmemb, void* userp);
    static ServerInfo processServerXMLNode(xmlTextReaderPtr reader);
    double execute(const ServerInfo &server, const TestConfig &config, const opFn &fnc);
    template <typename T>
        static T deg2rad(T n);
    template <typename T>
        static T harversine(std::pair<T, T> n1, std::pair<T, T> n2);

    IPInfo mIpInfo;
    std::vector<ServerInfo> mServerList;
    double mLatency;
    double mUploadSpeed;
    double mDownloadSpeed;

};


#endif //SPEEDTEST_SPEEDTEST_H
