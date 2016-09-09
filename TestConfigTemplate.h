//
// Created by Francesco Laurita on 6/2/16.
//

#ifndef SPEEDTEST_TESTCONFIGTEMPLATE_H
#define SPEEDTEST_TESTCONFIGTEMPLATE_H

#include "SpeedTest.h"

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
        32,        // concurrency

};

const TestConfig broadbandConfigUpload = {
        1000000,  // start_size
        70000000, // max_size
        250000,   // inc_size
        65536,    // buff_size
        20000,    // min_test_time_ms
        8,        // concurrency
};

const TestConfig fiberConfigDownload = {
        5000000,   // start_size
        120000000, // max_size
        950000,    // inc_size
        65536,     // buff_size
        20000,     // min_test_time_ms
        32,        // concurrency
};

const TestConfig fiberConfigUpload = {
        1000000,  // start_size
        70000000, // max_size
        250000,   // inc_size
        65536,    // buff_size
        20000,    // min_test_time_ms
        12,       // concurrency
};

#endif //SPEEDTEST_TESTCONFIGTEMPLATE_H
