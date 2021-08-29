FROM ubuntu:latest as builder

RUN mkdir -p /tmp/build /tmp/src
COPY *.h *.cpp *.h.in CMakeLists.txt /tmp/src/


RUN apt-get update && apt-get install -y g++ cmake make libcurl4-openssl-dev libxml2-dev libssl-dev && \
    cd /tmp/build && cmake -DCMAKE_BUILD_TYPE=Release ../src && make install

FROM ubuntu:latest
RUN apt-get update && apt-get install -y libxml2 libcurl4
COPY --from=builder /tmp/build/SpeedTest /usr/local/bin

ENTRYPOINT ["/usr/local/bin/SpeedTest"]
