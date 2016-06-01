//
// Created by Francesco Laurita on 5/30/16.
//

#include <arpa/inet.h>
#include "SpeedTestClient.h"

SpeedTestClient::SpeedTestClient(const ServerInfo &serverInfo): mServerInfo(serverInfo) {
    mSocketFd = 0;
}

SpeedTestClient::~SpeedTestClient() {
    close();
}

std::time_t SpeedTestClient::now() {

    return std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::system_clock::now().time_since_epoch())
            .count();
}

bool SpeedTestClient::connect() {

    if (mSocketFd){
        return true;
    }

    auto ret = mkSocket();
    if (!ret)
        return ret;

    std::string helo = "HI\n";
    if (write(mSocketFd, helo.c_str(), helo.size()) != helo.size()){
        close();
        return false;
    }

    char buff[200] = {'\0'};
    read(mSocketFd, &buff, 200);
    std::string heloreply(buff);
    std::string cmp = "HELLO";
    if (!heloreply.empty() && heloreply.compare(0, cmp.length(), cmp) == 0){
        return true;
    }
    close();
    return false;
}

void SpeedTestClient::close() {
    if (mSocketFd){
        std::string quit = "QUIT\n";
        write(mSocketFd, quit.c_str(), quit.size());
        ::close(mSocketFd);
    }

}


bool SpeedTestClient::mkSocket() {
    mSocketFd = socket(AF_INET, SOCK_STREAM, 0);

    if (!mSocketFd){
        std::cerr << "ERROR, socket()" << std::endl;
        return false;
    }

    std::size_t found = mServerInfo.host.find(":");
    std::string host = mServerInfo.host.substr(0, found);
    std::string port = mServerInfo.host.substr(found + 1, mServerInfo.host.length() - found);

    struct hostent *server = gethostbyname(host.c_str());
    if (server == nullptr) {
        std::cerr << "ERROR, no such host " << host << std::endl;
        return false;
    }

    int portno = std::atoi(port.c_str());
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, (size_t)server->h_length);

    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if (::connect(mSocketFd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("::connect");
        std::cerr << "ERROR connecting" << std::endl;
        return false;
    }
    return true;
}

bool SpeedTestClient::ping(long *millisec) {
    std::stringstream cmd;
    auto start = now();
    cmd << "PING " << start << "\n";

    char buff[200] = {'\0'};
    if (write(mSocketFd, cmd.str().c_str(), cmd.str().size()) != cmd.str().size()){

        if (millisec)
            *millisec = -1;

        return false;
    }

    read(mSocketFd, &buff, 200);
    auto stop = now();

    if (millisec)
        *millisec = stop - start;

    return true;
}

bool SpeedTestClient::download(const long size, long *millisec) {
    std::stringstream cmd;
    cmd << "DOWNLOAD " << size << "\n";
    if (write(mSocketFd, cmd.str().c_str(), cmd.str().size()) != cmd.str().size()) {

        if (millisec)
            *millisec = -1;

        return false;
    }

    char buff[8192] = {'\0'};
    long missing = 0;
    auto start = now();
    while (missing != size){
        auto current = read(mSocketFd, &buff, 8192);

        if (current <= 0){

            if (millisec)
                *millisec = -1;

            return false;
        }
        missing += current;

    }

    auto stop = now();
    *millisec = stop - start;
    return true;
}
