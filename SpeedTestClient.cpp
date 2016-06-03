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

    std::string hello = "HI\n";
    if (write(mSocketFd, hello.c_str(), hello.size()) != static_cast<ssize_t>(hello.size())){
        close();
        return false;
    }

    char buff[100] = {'\0'};
    ssize_t r = read(mSocketFd, &buff, 100);
    if (r <= 0){
        close();
        return false;
    }

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

    serv_addr.sin_port = htons(static_cast<uint16_t>(portno));

    /* Dial */
    if (::connect(mSocketFd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("::connect");
        std::cerr << "ERROR connecting" << std::endl;
        return false;
    }
    return true;
}

bool SpeedTestClient::ping(long &millisec) {
    std::stringstream cmd;
    auto start = now();
    cmd << "PING " << start << "\n";

    char buff[200] = {'\0'};
    if (write(mSocketFd, cmd.str().c_str(), cmd.str().size()) != (int)cmd.str().size()){
        return false;
    }

    read(mSocketFd, &buff, 200);
    if (std::string(buff).substr(0, 5) == "PONG "){
        auto stop = now();
        millisec = stop - start;
        return true;
    }

    return false;
}

bool SpeedTestClient::download(const long size, const long chunk_size, long &millisec) {
    std::stringstream cmd;
    cmd << "DOWNLOAD " << size << "\n";

    auto cmd_len = cmd.str().length();
    if (write(mSocketFd, cmd.str().c_str(), cmd_len) != static_cast<ssize_t>(cmd_len)) {
        return false;
    }

    char buff[chunk_size];
    for (size_t i = 0; i < static_cast<size_t>(chunk_size); i++)
        buff[i] = '\0';

    long missing = 0;
    auto start = now();
    while (missing != size){
        auto current = read(mSocketFd, &buff, static_cast<size_t>(chunk_size));

        if (current <= 0){
            return false;
        }
        missing += current;
    }

    auto stop = now();
    millisec = stop - start;
    return true;
}

bool SpeedTestClient::upload(const long size, const long chunk_size, long &millisec) {
    std::stringstream cmd;
    cmd << "UPLOAD " << size << "\n";
    auto cmd_len = cmd.str().length();

    char buff[chunk_size];
    for(size_t i = 0; i < static_cast<size_t>(chunk_size); i++)
        buff[i] = static_cast<char>(rand() % 256);

    long missing = size;
    auto start = now();
    ssize_t w = 0;
    w = write(mSocketFd, cmd.str().c_str(), cmd_len);

    if (w != static_cast<ssize_t>(cmd_len)){
        return false;
    }

    missing -= w;

    while(missing > 0){
        if (missing - chunk_size > 0){
            w = write(mSocketFd, &buff, static_cast<size_t>(chunk_size));
            if (w != chunk_size){
                return false;
            }
            missing -= w;
        } else {
            buff[missing - 1] = '\n';
            w = write(mSocketFd, &buff, static_cast<size_t>(missing));
            if (w != missing){
                return false;
            }
            missing -= w;
        }

    }

    std::stringstream ss;
    ss << "OK " << size << " ";
    char ret[200] = {'\0'};
    read(mSocketFd, &ret, 200);

    auto stop = now();
    millisec = stop - start;
    auto res = std::string(ret);
    return res.substr(0, ss.str().length()) == ss.str();

}
