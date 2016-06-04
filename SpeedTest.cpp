//
// Created by Francesco Laurita on 5/29/16.
//

#include <cmath>
#include "SpeedTest.h"
#include "SpeedTestClient.h"

SpeedTest::SpeedTest() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    mIpInfo = IPInfo();
    mServerList = std::vector<ServerInfo>();
    mLatency = 0;
    mUploadSpeed = 0;
    mDownloadSpeed = 0;
}

SpeedTest::~SpeedTest() {
    curl_global_cleanup();
}

bool SpeedTest::ipInfo(IPInfo *info) {
    if (!info)
        return false;

    if (!mIpInfo.ip_address.empty()){
        *info = mIpInfo;
        return true;
    }

    std::stringstream oss;
    auto code = httpGet(SPEED_TEST_IP_INFO_API_URL, oss);
    if (code == CURLE_OK){
        auto values = SpeedTest::parseQueryString(oss.str());
        mIpInfo = IPInfo();
        mIpInfo.ip_address = values["ip_address"];
        mIpInfo.isp = values["isp"];
        mIpInfo.lat = std::stof(values["lat"]);
        mIpInfo.lon = std::stof(values["lon"]);
        values.clear();
        oss.clear();
        *info = mIpInfo;
        return true;
    }

    return false;
}

const std::vector<ServerInfo>& SpeedTest::serverList() {
    if (!mServerList.empty())
        return mServerList;
    std::stringstream oss;

    auto cres = httpGet(SPEED_TEST_SERVER_LIST_URL, oss);
    if (cres != CURLE_OK)
        return mServerList;

    size_t len = oss.str().length();
    char *xmlbuff = (char*)calloc(len + 1, sizeof(char));
    memcpy(xmlbuff, oss.str().c_str(), len + 1);

    oss.str("");
    oss.clear();
    xmlTextReaderPtr reader = xmlReaderForMemory(xmlbuff, static_cast<int>(len), nullptr, nullptr, 0);

    if (reader != nullptr) {
        IPInfo ipInfo;
        if (!SpeedTest::ipInfo(&ipInfo)){
            xmlFreeTextReader(reader);
            return mServerList;
        }
        auto ret = xmlTextReaderRead(reader);
        while (ret == 1) {
            ServerInfo info = processServerXMLNode(reader);
            if (!info.url.empty()){
                info.distance = harversine(std::make_pair(ipInfo.lat, ipInfo.lon), std::make_pair(info.lat, info.lon));
                mServerList.push_back(info);
            }
            ret = xmlTextReaderRead(reader);
        }
        xmlFreeTextReader(reader);
        if (ret != 0) {
            std::cerr << "Failed to parse" << std::endl;
        }
    } else {
        std::cerr << "Unable to initialize xml parser" << std::endl;
    }

    free(xmlbuff);
    xmlCleanupParser();

    std::sort(mServerList.begin(), mServerList.end(), [](const ServerInfo &a, const ServerInfo &b) -> bool {
        return a.distance < b.distance;
    });
    return mServerList;
}

const ServerInfo SpeedTest::bestServer(const int sample_size) {
    int i = sample_size;
    ServerInfo bestServer = serverList()[0];

    long best_latency = 1000;
    long latency = 0;
    for (auto &server : serverList()){
        auto client = SpeedTestClient(server);

        if (!client.connect()){
            std::cout << "E" << std::flush;
            continue;
        }

        long current_latency = 1000;
        for (int j = 0; j < 20; j++){
            if (client.ping(latency)){
                if (current_latency > latency)
                    current_latency = latency;
            } else {
                std::cout << "E" << std::flush;
            }
        }
        client.close();
        std::cout << "." << std::flush;
        if (best_latency > current_latency ){
            best_latency = current_latency;
            bestServer = server;
            mLatency = best_latency;
        }

        if (i-- < 0){
            break;
        }

    }
    return bestServer;
}

const double SpeedTest::downloadSpeed(const ServerInfo &server, const TestConfig &config) {
    opFn pfunc = &SpeedTestClient::download;
    mDownloadSpeed = execute(server, config, pfunc);
    return mDownloadSpeed;
}

const double SpeedTest::uploadSpeed(const ServerInfo &server, const TestConfig &config) {
    opFn pfunc = &SpeedTestClient::upload;
    mUploadSpeed = execute(server, config, pfunc);
    return mUploadSpeed;
}

const double &SpeedTest::latency() {
    return mLatency;
}

// private

float SpeedTest::execute(const ServerInfo &server, const TestConfig &config, const opFn &pfunc) {
    std::vector<std::thread> workers;
    double overall_speed = 0;
    std::mutex mtx;
    for (int i = 0; i < config.concurrency; i++) {
        workers.push_back(std::thread([&server, &overall_speed, &pfunc, &config, &mtx](){
            long start_size = config.start_size;
            long max_size   = config.max_size;
            long incr_size  = config.incr_size;
            long curr_size  = start_size;

            auto spClient = SpeedTestClient(server);

            if (spClient.connect()) {
                long total_size = 0;
                long total_time = 0;
                auto start = SpeedTestClient::now();
                std::vector<double> partial_results;
                while (curr_size < max_size){
                    long op_time = 0;
                    if ((spClient.*pfunc)(curr_size, config.buff_size, op_time)) {
                        total_size += curr_size;
                        total_time += op_time;
                        double metric = (curr_size * 8) / (static_cast<double>(op_time) / 1000);
                        partial_results.push_back(metric);
                        std::cout << "." << std::flush;
                    } else {
                        std::cout << "E" << std::flush;
                    }
                    curr_size += incr_size;
                    if ((SpeedTestClient::now() - start) > config.min_test_time_ms)
                        break;
                }

                spClient.close();
                std::sort(partial_results.begin(), partial_results.end());

                size_t skip = 0;
                size_t drop = 0;
                if (partial_results.size() >= 10){
                    skip = partial_results.size() / 4;
                    drop = 2;
                }

                size_t iter = 0;
                double real_sum = 0;
                for (auto it = partial_results.begin() + skip; it != partial_results.end() - drop; ++it ){
                    iter++;
                    real_sum += (*it);
                }
                mtx.lock();
                overall_speed += (real_sum / iter);
                mtx.unlock();
            } else {
                std::cout << "E" << std::flush;
            }
        }));

    }
    for (auto &t : workers){
        t.join();
    }

    workers.clear();

    return overall_speed / 1000 / 1000;
}

template<typename T>
T SpeedTest::deg2rad(T n) {
    return (n * M_PI / 180);
}

template<typename T>
T SpeedTest::harversine(std::pair<T, T> n1, std::pair<T, T> n2) {
    T lat1r = deg2rad(n1.first);
    T lon1r = deg2rad(n1.second);
    T lat2r = deg2rad(n2.first);
    T lon2r = deg2rad(n2.second);
    T u = std::sin((lat2r - lat1r) / 2);
    T v = std::sin((lon2r - lon1r) / 2);
    return 2.0 * EARTH_RADIUS_KM * std::asin(std::sqrt(u * u + std::cos(lat1r) * std::cos(lat2r) * v * v));
}

CURLcode SpeedTest::httpGet(const std::string &url, std::stringstream &ss, CURL *handler, long timeout) {

    CURLcode code(CURLE_FAILED_INIT);
    CURL* curl = SpeedTest::curl_setup(handler);


    if (curl){
        if (CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_FILE, &ss))
            && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout))
            && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_URL, url.c_str()))) {
            code = curl_easy_perform(curl);
        }
        if (handler == nullptr)
            curl_easy_cleanup(curl);
    }
    return code;
}

CURLcode SpeedTest::httpPost(const std::string &url, const std::string &postdata, std::stringstream &os, void *handler,
                             long timeout) {

    CURLcode code(CURLE_FAILED_INIT);
    CURL* curl = SpeedTest::curl_setup(handler);

    if (curl){
        if (CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_FILE, &os))
            && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout))
            && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_URL, url.c_str()))
            && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata.c_str()))) {
            code = curl_easy_perform(curl);
        }
        if (handler == nullptr)
            curl_easy_cleanup(curl);
    }
    return code;
}

CURL *SpeedTest::curl_setup(CURL *handler) {
    CURL* curl = handler == nullptr ? curl_easy_init() : handler;
    if (curl){
        if (curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeFunc) == CURLE_OK
            && curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L) == CURLE_OK
            && curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L) == CURLE_OK
            && curl_easy_setopt(curl, CURLOPT_USERAGENT, SPEED_TEST_USER_AGENT) == CURLE_OK){
            return curl;
        } else {
            curl_easy_cleanup(handler);
            return nullptr;
        }
    }
    return nullptr;


}

size_t SpeedTest::writeFunc(void *buf, size_t size, size_t nmemb, void *userp) {

    if (userp){
        std::stringstream &os = *static_cast<std::stringstream *>(userp);
        std::streamsize len = size * nmemb;
        if(os.write(static_cast<char*>(buf), len))
            return  static_cast<size_t>(len);
    }
    return 0;
}

std::map<std::string, std::string> SpeedTest::parseQueryString(const std::string &query) {
    auto map = std::map<std::string, std::string>();
    auto pairs = splitString(query, '&');
    for (auto &p : pairs){
        auto kv = splitString(p, '=');
        if (kv.size() == 2){
            map[kv[0]] = kv[1];
        }
    }
    return map;
}

std::vector<std::string> SpeedTest::splitString(const std::string &instr, const char separator) {
    if (instr.empty())
        return std::vector<std::string>();

    std::vector<std::string> tokens;
    std::size_t start = 0, end = 0;
    while ((end = instr.find(separator, start)) != std::string::npos) {
        std::string temp = instr.substr(start, end - start);
        if (temp != "")
            tokens.push_back(temp);
        start = end + 1;
    }
    std::string temp = instr.substr(start);
    if (temp != "")
        tokens.push_back(temp);
    return tokens;

}

ServerInfo SpeedTest::processServerXMLNode(xmlTextReaderPtr reader) {

    auto name = xmlTextReaderConstName(reader);
    auto nodeName = std::string((char*)name);

    if (!name || nodeName != "server"){
        return ServerInfo();
    }

    if (xmlTextReaderAttributeCount(reader) > 0){
        auto info = ServerInfo();
        auto server_url     = xmlTextReaderGetAttribute(reader, BAD_CAST "url");
        auto server_lat     = xmlTextReaderGetAttribute(reader, BAD_CAST "lat");
        auto server_lon     = xmlTextReaderGetAttribute(reader, BAD_CAST "lon");
        auto server_name    = xmlTextReaderGetAttribute(reader, BAD_CAST "name");
        auto server_county  = xmlTextReaderGetAttribute(reader, BAD_CAST "country");
        auto server_cc      = xmlTextReaderGetAttribute(reader, BAD_CAST "cc");
        auto server_host    = xmlTextReaderGetAttribute(reader, BAD_CAST "host");
        auto server_id      = xmlTextReaderGetAttribute(reader, BAD_CAST "id");
        auto server_sponsor = xmlTextReaderGetAttribute(reader, BAD_CAST "sponsor");

        info.name.append((char*)server_name);
        info.url.append((char*)server_url);
        info.country.append((char*)server_county);
        info.country_code.append((char*)server_cc);
        info.host.append((char*)server_host);
        info.sponsor.append((char*)server_sponsor);
        info.id  = std::atoi((char*)server_id);
        info.lat = std::stof((char*)server_lat);
        info.lon = std::stof((char*)server_lon);
        xmlFree(server_url);
        xmlFree(server_lat);
        xmlFree(server_lon);
        xmlFree(server_name);
        xmlFree(server_county);
        xmlFree(server_cc);
        xmlFree(server_host);
        xmlFree(server_id);
        xmlFree(server_sponsor);
        return info;
    }

    return ServerInfo();
}
