
#include "lib/utils/systemUtils.h"

#include <iostream>
#include <fstream>
#include <string>

std::string SystemUtils::getRaspberryPiSerial() 
{
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    std::string serial;

    if (cpuinfo.is_open()) {
        while (std::getline(cpuinfo, line)) {
            if (line.find("Serial") != std::string::npos) {
                serial = line.substr(line.find(":") + 1);
                break;
            }
        }
        cpuinfo.close();
    } else {
        std::cerr << "Error abriendo /proc/cpuinfo" << std::endl;
    }

    return serial;
}