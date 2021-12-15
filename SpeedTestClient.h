//
// Created by Francesco Laurita on 5/30/16.
// SPDX-License-Identifier: MIT
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
class SpeedTestClient
{
public:
    explicit SpeedTestClient(const ServerInfo &serverInfo);
    ~SpeedTestClient();

    bool connect();
    bool ping(double &millisec);
    bool upload(long size, long chunk_size, double &millisec);
    bool download(long size, long chunk_size, double &millisec);
    float version();
    const std::pair<std::string, int> hostport();
    void close();

private:
    bool mkSocket();
    ServerInfo mServerInfo;
    int mSocketFd;
    float mServerVersion;
    static bool readLine(int &fd, std::string &buffer);
    static bool writeLine(int &fd, const std::string &buffer);
};

typedef bool (SpeedTestClient::*opFn)(const long size, const long chunk_size, double &millisec);
#endif // SPEEDTEST_SPEEDTESTCLIENT_H
