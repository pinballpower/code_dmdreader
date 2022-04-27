#pragma once

#include <string>
#include <cstdint>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

using namespace std;

class SPIException : public exception {

public:
    SPIException(const string msg);
    ~SPIException();

    string msg;
};

// can be read from /sys/module/spidev/parameters/bufsiz, but hardcoded now for simplicity
constexpr int spi_kernel_bufsize = 4096; 

class SPI {

public:
    void openDevice(const string spiDevice, unsigned int spiSpeed, unsigned int spiFlags = 0);
    void closeDevice();
    void readData(uint8_t* buf, unsigned int count) const;

private:
    unsigned int speed;
    int fileDescriptor = 0;
};