//
// Created by Francesco Laurita on 6/2/16.
//

#ifndef SPEEDTEST_TESTCONFIGTEMPLATE_H
#define SPEEDTEST_TESTCONFIGTEMPLATE_H

#include "SpeedTest.h"

//long start_size;
//long max_size;
//long incr_size;
//long buff_size;
//long min_test_time_ms;
//int  concurrency;


//downloadConfig.buff_size = 65536;
//downloadConfig.concurrency = 16;
//downloadConfig.start_size = 1000000;
//downloadConfig.incr_size = 750000;
//downloadConfig.min_test_time_ms = 20000;
//downloadConfig.max_size = 100000000;
//
//
//TestConfig uploadConfig;
//uploadConfig.buff_size = 65536;
//uploadConfig.concurrency = concurrency;
//uploadConfig.start_size = 1000000;
//uploadConfig.incr_size = 250000;
//uploadConfig.min_test_time_ms = 20000;
//uploadConfig.max_size = 70000000;

const TestConfig preflightConfigDownload = {
         600000, // start_size
        2000000, // max_size
         125000, // inc_size
           4096, // buff_size
          10000, // min_test_time_ms
              2, // Concurrency
};

const TestConfig slowConfigDownload = {
         100000, // start_size
         500000, // max_size
          10000, // inc_size
           1024, // buff_size
          20000, // min_test_time_ms
              2, // Concurrency
};

const TestConfig slowConfigUpload = {
          50000, // start_size
          80000, // max_size
           1000, // inc_size
           1024, // buff_size
          20000, // min_test_time_ms
              2, // Concurrency
};


const TestConfig narrowConfigDownload = {
          1000000, // start_size
        100000000, // max_size
           750000, // inc_size
            4096, // buff_size
            20000, // min_test_time_ms
                2, // Concurrency
};

const TestConfig narrowConfigUpload = {
        1000000, // start_size
        100000000, // max_size
        550000, // inc_size
        4096, // buff_size
        20000, // min_test_time_ms
        2, // Concurrency
};

const TestConfig broadbandConfigDownload = {
        1000000,   // start_size
        100000000, // max_size
        750000,    // inc_size
        65536,     // buff_size
        20000,     // min_test_time_ms
        16,        // concurrency

};

const TestConfig broadbandConfigUpload = {
        1000000,   // start_size
        70000000, // max_size
        250000,    // inc_size
        65536,     // buff_size
        20000,     // min_test_time_ms
        8,        // concurrency
};

const TestConfig fiberConfigDownload = {
};

const TestConfig fiberConfigUpload = {
};

#endif //SPEEDTEST_TESTCONFIGTEMPLATE_H
