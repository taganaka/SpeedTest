//
// Created by Francesco Laurita on 9/9/16.
//

#ifndef SPEEDTEST_CMDOPTIONS_H
#define SPEEDTEST_CMDOPTIONS_H
#include <getopt.h>

enum OutputType { verbose, text, json };
enum LineType { automatic, slow, narrow, broad, fiber, gigasym };


typedef struct program_options_t {
    bool help     = false;
    bool latency  = false;
    bool download = false;
    bool upload   = false;
    bool share    = false;
    std::string selected_server = "";
    OutputType output_type = OutputType::verbose;
    LineType line_type = LineType::automatic;
} ProgramOptions;

static struct option CmdLongOptions[] = {
        {"help",        no_argument,       0, 'h' },
        {"latency",     no_argument,       0, 'l' },
        {"download",    no_argument,       0, 'd' },
        {"upload",      no_argument,       0, 'u' },
        {"share",       no_argument,       0, 's' },
        {"test-server", required_argument, 0, 't' },
        {"output",      required_argument, 0, 'o' },
        {"line-type",   required_argument, 0, 'i' },
        {0,             0,                 0,  0  }
};

const char *optStr = "hldusqt:o:";

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
            case 't':
                options.selected_server.append(optarg);
                break;
            case 'o':
                if (strcmp(optarg, "verbose") == 0)
                    options.output_type = OutputType::verbose;
                else if (strcmp(optarg, "text") == 0)
                    options.output_type = OutputType::text;
                else if (strcmp(optarg, "json") == 0)
                    options.output_type = OutputType::json;
                else {
                    std::cerr << "Unsupported output type " << optarg << std::endl;
                    std::cerr << "Supported output type: default, text, json" <<std::endl;
                    return false;
                }

                break;
            case 'i':
                if (strcmp(optarg, "auto") == 0)
                    options.line_type = LineType::automatic;
                else if (strcmp(optarg, "slow") == 0)
                    options.line_type = LineType::slow;
                else if (strcmp(optarg, "narrow") == 0)
                    options.line_type = LineType::narrow;
                else if (strcmp(optarg, "broad") == 0)
                    options.line_type = LineType::broad;
                else if (strcmp(optarg, "fiber") == 0)
                    options.line_type = LineType::fiber;
                else if (strcmp(optarg, "gigasym") == 0)
                    options.line_type = LineType::gigasym;
                else {
                    std::cerr << "Unsupported line type " << optarg << std::endl;
                    std::cerr << "Supported line type: auto, slow, narrow, broad, fiber" <<std::endl;
                    return false;
                }

                break;
            default:
                return false;
        }

    }
    return true;
}


#endif //SPEEDTEST_CMDOPTIONS_H
