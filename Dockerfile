FROM ubuntu:latest as builder

COPY *.h *.cpp *.h.in CMakeLists.txt /tmp/src/

RUN apt-get update && apt-get install -y g++ cmake make libcurl4-openssl-dev libxml2-dev libssl-dev && \
    cd /tmp/src && cmake -DCMAKE_BUILD_TYPE=Release . && make && \
    make install DESTDIR=/tmp/build && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

FROM ubuntu:latest
RUN apt-get update && apt-get install -y libxml2 libcurl4 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

COPY --from=builder /tmp/build/ /

ENTRYPOINT ["/usr/local/bin/SpeedTest"]
