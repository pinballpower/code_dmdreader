#pragma "once"

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

static unsigned int spi_speed = 10000000;
static int spi_kernel_bufsize = 4096; // can be read from /sys/module/spidev/parameters/bufsiz, but hardcoded now for simplicity

// SPI data transfer buffer
const int spi_buffer_len = 16384; // our SPI packets can't be bigger than 16kB
static uint8_t spi_buffer[spi_buffer_len];

static int spi_fd = 0;
static bool spi_finished = false;

static  struct spi_ioc_transfer spi_transfer;

void spi_open(string spi_device, unsigned int spi_flags = 0);
void spi_close();
void spi_read(unsigned int count, uint8_t *buf = nullptr);