//
// Created by Francesco Laurita on 5/29/16.
//

#ifndef SPEEDTEST_SPEEDTEST_H
#define SPEEDTEST_SPEEDTEST_H

#include <map>
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <libxml/xmlreader.h>
#include <math.h>
#include <vector>
#include <algorithm>

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
    std::string latency_url;
    float lat;
    float lon;
    float distance;

} ServerInfo;

class SpeedTest {
public:
    SpeedTest();
    ~SpeedTest();
    CURLcode httpGet(const std::string& url, std::ostream& os, CURL *handler = nullptr, long timeout = 30);
    static std::map<std::string, std::string> parseQueryString(const std::string& query);
    bool ipInfo(IPInfo *info);
    const std::vector<ServerInfo>& serverList();
    const ServerInfo bestServer(const int sample_size = 5);
    const double &latency();
    const double downloadSpeed(const ServerInfo& server);
private:
    static size_t writeFunc(void* buf, size_t size, size_t nmemb, void* userp);
    static ServerInfo processServerXMLNode(xmlTextReaderPtr reader);
    template <typename T>
        static T deg2rad(T n);
    template <typename T>
        static T harversine(std::pair<T, T> n1, std::pair<T, T> n2);

    IPInfo mIpInfo;
    std::vector<ServerInfo> mServerList;
    double mLatency;

};


#endif //SPEEDTEST_SPEEDTEST_H
