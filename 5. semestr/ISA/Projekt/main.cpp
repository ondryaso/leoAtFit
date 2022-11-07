// main.cpp
// Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)

#include <fstream>
#include <filesystem>
#include <unistd.h>

#include "utils.h"
#include "sender.h"
#include "receiver.h"
#include "secure_string.h"

void print_help(char *exe_name) {
    std::cerr << "Usage: " << exe_name << " [-r filename] [-s ip/hostname] [-l[o]] [-v] [-q] [-k]" << std::endl
              << "Specify both -r and -s to send a file." << std::endl
              << "Use -l to receive a file. Use -o to allow overwriting an existing file. -o can only be used together with -l."
              << std::endl
              << "-r/-s and -l cannot be combined." << std::endl
              << "Use -v to enable verbose output. Use -q to disable all output on stdout."
              << std::endl
              << "Use -k to provide a custom encryption key." << std::endl;
}

int main(int argc, char **argv) {
    int opt;
    std::string file_name, destination;
    bool is_sender = false, is_receiver = false, quiet = false, enter_password = false, enable_overwrite = false,
            verbose = false;

    while ((opt = getopt(argc, argv, "r:s:lvqko")) != -1) {
        switch (opt) {
            case 'r':
                file_name = std::string(optarg);
                is_sender = true;
                break;
            case 's':
                destination = std::string(optarg);
                is_sender = true;
                break;
            case 'l':
                is_receiver = true;
                break;
            case 'v':
                verbose = true;
                break;
            case 'q':
                quiet = true;
                break;
            case 'k':
                enter_password = true;
                break;
            case 'o':
                enable_overwrite = true;
                break;
            default:
                print_help(argv[0]);
                return EXIT_FAILURE;
        }
    }

    if ((verbose && quiet) || (is_sender && (is_receiver || file_name.empty() || destination.empty())) ||
        (!is_sender && !is_receiver)) {
        print_help(argv[0]);
        return EXIT_FAILURE;
    }

    if (quiet) {
        enable_info = false;
        enable_warn = false;
    } else if (verbose) {
        enable_verbose = true;
    }

    if (enter_password) {
        std::cout << "Enter encryption key:" << std::endl;
    }

    secure_string password = enter_password ? read_password() : DEFAULT_PASSWORD;

    try {
        if (is_sender) {
            auto file_path = std::filesystem::path(file_name);
            if (!std::filesystem::exists(file_path)) {
                log_err("The specified file doesn't exist");
                return EXIT_FAILURE;
            }

            std::fstream fs(file_name, std::fstream::in | std::fstream::binary);
            Sender sender(destination);
            sender.send(fs, file_path.filename(), password);
        } else {
            Receiver receiver(enable_overwrite);
            receiver.accept(password);
        }

        return EXIT_SUCCESS;
    } catch (std::exception const &e) {
        log_err(e.what());
        return EXIT_FAILURE;
    }
}

