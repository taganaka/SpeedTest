//
// Created by Francesco Laurita on 5/30/16.
//

#ifndef SPEEDTEST_SPEEDTESTCLIENT_H
#define SPEEDTEST_SPEEDTESTCLIENT_H


#include "SpeedTest.h"
#include <ctime>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <unistd.h>
class SpeedTestClient {
public:
    SpeedTestClient(const ServerInfo& serverInfo);
    ~SpeedTestClient();

    bool connect();
    bool ping(long *millisec);
    bool upload(const long size);
    bool download(const long size, long *millisec);
    void close();

private:
    std::time_t now();
    bool mkSocket();
    ServerInfo mServerInfo;
    int mSocketFd;
};


#endif //SPEEDTEST_SPEEDTESTCLIENT_H
