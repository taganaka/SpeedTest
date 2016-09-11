//
// Created by Francesco Laurita on 5/30/16.
//

#include <arpa/inet.h>
#include "SpeedTestClient.h"

SpeedTestClient::SpeedTestClient(const ServerInfo &serverInfo, bool qualityHost): mServerInfo(serverInfo),
                                                                                  mSocketFd(0),
                                                                                  mQualityHost(qualityHost),
                                                                                  mServerVersion(-1.0){}

SpeedTestClient::~SpeedTestClient() {
    close();
}

// It returns current timestamp in ms
std::time_t SpeedTestClient::now() {

    return std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::system_clock::now().time_since_epoch())
            .count();
}

// It connects and initiates client/server handshaking
bool SpeedTestClient::connect() {

    if (mSocketFd){
        return true;
    }

    auto ret = mkSocket();
    if (!ret)
        return ret;

    std::string reply;

    if (!SpeedTestClient::writeLine(mSocketFd, "HI")){
        close();
        return false;
    }


    if (SpeedTestClient::readLine(mSocketFd, reply)){
        std::stringstream reply_stream(reply);
        std::string hello;
        reply_stream >> hello >> mServerVersion;
        if (reply_stream.fail()) {
            close();
            return false;
        }

        if (!reply.empty() && "HELLO" == hello){
            return true;
        }

    }
    close();
    return false;
}

// It closes a connection
void SpeedTestClient::close() {
    if (mSocketFd){
        SpeedTestClient::writeLine(mSocketFd, "QUIT");
        ::close(mSocketFd);
    }

}

// It executes PING command
bool SpeedTestClient::ping(long &millisec) {
    if (!mSocketFd)
        return false;

    std::stringstream cmd;
    std::string reply;

    auto start = now();
    cmd << "PING " << start;

    if (!SpeedTestClient::writeLine(mSocketFd, cmd.str())){
        return false;
    }

    if (SpeedTestClient::readLine(mSocketFd, reply)){
        if (reply.substr(0, 5) == "PONG "){
            auto stop = now();
            millisec = stop - start;
            return true;
        }
    }

    close();
    return false;
}

// It executes DOWNLOAD command
bool SpeedTestClient::download(const long size, const long chunk_size, long &millisec) {
    std::stringstream cmd;
    cmd << "DOWNLOAD " << size;

    if (!SpeedTestClient::writeLine(mSocketFd, cmd.str())){
        return false;
    }


    char *buff = new char[chunk_size];
    for (size_t i = 0; i < static_cast<size_t>(chunk_size); i++)
        buff[i] = '\0';

    long missing = 0;
    auto start = now();
    while (missing != size){
        auto current = read(mSocketFd, buff, static_cast<size_t>(chunk_size));

        if (current <= 0){
            delete[] buff;
            return false;
        }
        missing += current;
    }

    auto stop = now();
    millisec = stop - start;
    delete[] buff;
    return true;
}

// It executes UPLOAD command
bool SpeedTestClient::upload(const long size, const long chunk_size, long &millisec) {
    std::stringstream cmd;
    cmd << "UPLOAD " << size << "\n";
    auto cmd_len = cmd.str().length();

    char *buff = new char[chunk_size];
    for(size_t i = 0; i < static_cast<size_t>(chunk_size); i++)
        buff[i] = static_cast<char>(rand() % 256);

    long missing = size;
    auto start = now();

    if (!SpeedTestClient::writeLine(mSocketFd, cmd.str())){
        delete[] buff;
        return false;
    }

    ssize_t w = cmd_len;
    missing -= w;

    while(missing > 0){
        if (missing - chunk_size > 0){
            w = write(mSocketFd, buff, static_cast<size_t>(chunk_size));
            if (w != chunk_size){
                delete[] buff;
                return false;
            }
            missing -= w;
        } else {
            buff[missing - 1] = '\n';
            w = write(mSocketFd, buff, static_cast<size_t>(missing));
            if (w != missing){
                delete[] buff;
                return false;
            }
            missing -= w;
        }

    }
    std::string reply;
    if (!SpeedTestClient::readLine(mSocketFd, reply)){
        delete[] buff;
        return false;
    }
    auto stop = now();

    std::stringstream ss;
    ss << "OK " << size << " ";
    millisec = stop - start;
    delete[] buff;
    return reply.substr(0, ss.str().length()) == ss.str();

}

bool SpeedTestClient::mkSocket() {
    mSocketFd = socket(AF_INET, SOCK_STREAM, 0);

    if (!mSocketFd){
        std::cerr << "ERROR, socket()" << std::endl;
        return false;
    }

    auto hostp = hostport();

    struct hostent *server = gethostbyname(hostp.first.c_str());
    if (server == nullptr) {
        std::cerr << "ERROR, no such host " << hostp.first << std::endl;
        return false;
    }

    int portno = hostp.second;
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, (size_t)server->h_length);

    serv_addr.sin_port = htons(static_cast<uint16_t>(portno));

    /* Dial */
    if (::connect(mSocketFd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("::connect() error");
        return false;
    }
    return true;
}

bool SpeedTestClient::ploss(const int size, const int wait_millisec, int &nploss) {
    connect();
    auto hostp = hostport();
    struct sockaddr_in serveraddr;
    int sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd_udp < 0){
        std::cerr << "ERROR UDP opening socket" << std::endl;
        return false;
    }


    auto server = gethostbyname(hostp.first.c_str());
    if (server == NULL) {
        std::cerr << "ERROR, no such host as " << hostp.first << std::endl;
        return false;
    }

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *)&serveraddr.sin_addr.s_addr, (size_t)server->h_length);
    serveraddr.sin_port = htons(hostp.second);

    auto cookie = now();

    std::string reply;
    std::stringstream cmd;
    cmd << "ME " << cookie << " 0 settings\n";

    if (!SpeedTestClient::writeLine(mSocketFd, cmd.str())){
        std::cerr << cmd.str() << " " << " fail" << std::endl;
        return false;
    }

    if (SpeedTestClient::readLine(mSocketFd, reply)){
        if (reply.find("YOURIP") == std::string::npos)
            return false;
    }


    std::stringstream ss;
    ss << "LOSS " << cookie;
    auto payload_len = static_cast<ssize_t >(ss.str().length());
    socklen_t serverlen = sizeof(serveraddr);
    /* send the message to the server */
    for (long i = 0; i < size; i++){
        auto n = sendto(sockfd_udp, ss.str().c_str(), payload_len, 0, (const sockaddr *) &serveraddr, serverlen);
        if (n != payload_len){
            std::cerr << "E";
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(wait_millisec));
//    usleep(static_cast<useconds_t >( * 1000));
    cmd.str("");
    cmd.clear();

    cmd << "PLOSS\n";

    if (!SpeedTestClient::writeLine(mSocketFd, cmd.str())){
        std::cerr << cmd.str() << " " << " fail" << std::endl;
        return false;
    }

    if (!SpeedTestClient::readLine(mSocketFd, reply)){
        std::cerr << cmd.str() << " " << " read fail" << std::endl;
        return false;
    }
    size_t pos;
    if ((pos = reply.find(" ")) != std::string::npos){
        if (reply.substr(0, pos) == "PLOSS"){
            auto packet_loss = std::atoi(reply.substr(pos + 1).c_str());
            nploss = size - packet_loss;
        } else {
            return false;
        }
    } else {
        return false;
    }
    return true;
}

float SpeedTestClient::version() {
    return mServerVersion;
}

const std::pair<std::string, int> SpeedTestClient::hostport() {
    std::string targetHost = mQualityHost ? mServerInfo.linequality : mServerInfo.host;
    std::size_t found = targetHost.find(":");
    std::string host  = targetHost.substr(0, found);
    std::string port  = targetHost.substr(found + 1, targetHost.length() - found);
    return std::pair<std::string, int>(host, std::atoi(port.c_str()));
}

bool SpeedTestClient::readLine(int &fd, std::string &buffer) {
    buffer.clear();
    if (!fd)
        return false;
    char c;
    while(true){
        auto n = read(fd, &c, 1);
        if (n == -1)
            return false;
        if (c == '\n' || c == '\r')
            break;

        buffer += c;

    }
    return true;
}

bool SpeedTestClient::writeLine(int &fd, const std::string &buffer) {
    if (!fd)
        return false;

    auto len = static_cast<ssize_t >(buffer.length());
    if (len == 0)
        return false;

    std::string buff_copy = buffer;

    if (buff_copy.find_first_of('\n') == std::string::npos){
        buff_copy += '\n';
        len += 1;
    }
    auto n = write(fd, buff_copy.c_str(), len);
    return n == len;
}




