//
// Created by Francesco Laurita on 9/9/16.
//

#ifndef SPEEDTEST_CMDOPTIONS_H
#define SPEEDTEST_CMDOPTIONS_H
#include <getopt.h>

typedef struct program_options_t {
    bool help     = false;
    bool latency  = false;
    bool download = false;
    bool upload   = false;
    bool share    = false;
} ProgramOptions;

static struct option CmdLongOptions[] = {
        {"help",      no_argument,  0,  'h' },
        {"latency",   no_argument,  0,  'l' },
        {"download",  no_argument,  0,  'd' },
        {"upload",    no_argument,  0,  'u' },
        {"share",     no_argument,  0,  's' },
        {0,           0,            0,   0  }
};

const char *optStr = "hldus";

bool ParseOptions(const int argc, const char **argv, ProgramOptions& options){
    int long_index =0;
    int opt = 0;
    while ((opt = getopt_long(argc, (char **)argv, optStr, CmdLongOptions, &long_index )) != -1) {
        switch (opt){
            case 'h':
                options.help     = true;
                break;
            case 'l':
                options.latency  = true;
                break;
            case 'd':
                options.download = true;
                break;
            case 'u':
                options.upload   = true;
                break;
            case 's':
                options.share    = true;
                break;
            default:
                return false;
        }

    }
    return true;
}


#endif //SPEEDTEST_CMDOPTIONS_H
