//
// Created by Francesco Laurita on 5/30/16.
//

#ifndef SPEEDTEST_SPEEDTESTCLIENT_H
#define SPEEDTEST_SPEEDTESTCLIENT_H


#include <ctime>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <unistd.h>
#include "SpeedTest.h"
#include "DataTypes.h"
class SpeedTestClient {
public:
    SpeedTestClient(const ServerInfo& serverInfo, bool qualityHost = false);
    ~SpeedTestClient();

    bool connect();
    bool ping(long &millisec);
    bool upload(const long size, const long chunk_size, long &millisec);
    bool download(const long size, const long chunk_size, long &millisec);
    bool ploss(const int size, const int wait_millisec, int &nploss);
    float version();
    const std::pair<std::string, int> hostport();
    void close();
    static std::time_t now();

private:
    bool mkSocket();
    ServerInfo mServerInfo;
    int mSocketFd;
    bool mQualityHost;
    float mServerVersion;
    static bool readLine(int& fd, std::string& buffer);
    static bool writeLine(int& fd, const std::string& buffer);
};

typedef bool (SpeedTestClient::*opFn)(const long size, const long chunk_size, long &millisec);
#endif //SPEEDTEST_SPEEDTESTCLIENT_H
