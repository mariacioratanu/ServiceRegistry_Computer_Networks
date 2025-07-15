#include "logging.h"

#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>

using namespace std;

ofstream log_file("server.log", ios::app);

/**
 * Logs a message to a log file including the timestamp.
 * Opens the file in append mode, writes the message, and closes it.
 */
void log_message(string message) {
    if (message == "start") {
        log_file << "----------------------------------------------------------" << endl;
        return;
    }
    else if (message == "end") {
        log_file << "----------------------------------------------------------" << endl;
        return;
    }
    time_t now = time(nullptr) ;
    if (message[message.length() - 1] == '\n')
        message = message.substr(0, message.length() - 1);
    log_file << "[" << put_time(localtime(&now), "%T") << "] " << message << endl ;
}
