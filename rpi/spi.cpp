#include "spi.h"

#include <boost/log/trivial.hpp>


void openDevice(string spi_device, unsigned int spi_flags)
{
    int i;
    char  spi_mode;
    char  spi_bits = 8;
    char dev[32];

    spi_mode = spi_flags & 3;

    if ((spi_fd = open(spi_device.c_str(), O_RDWR)) < 0)
    {
        throw SPIException("couldn't open SPI device " + spi_device);
    }

    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &spi_mode) < 0)
    {
        close(spi_fd);
        throw SPIException("couldn't set SPI mode");
    }

    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bits) < 0)
    {
        close(spi_fd);
        throw SPIException("couldn't set SPI bits/word");
    }

    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed) < 0)
    {
        close(spi_fd);
        throw SPIException("couldn't set SPI speed");
    }

    memset(&spi_transfer, 0, sizeof(spi_transfer));

    spi_transfer.tx_buf = 0;
    spi_transfer.rx_buf = (__u64)spi_buffer;
    spi_transfer.len = 0;
    spi_transfer.speed_hz = spi_speed;
    spi_transfer.delay_usecs = 0;
    spi_transfer.bits_per_word = 8;
    spi_transfer.cs_change = 0;
}

void closeDevice()
{
    close(spi_fd);
}

void readData(unsigned int count, uint8_t* buf)
{
    if (buf == nullptr) {
        buf = spi_buffer;
        if (count > spi_buffer_len) {
            count = spi_buffer_len;
        }
    }

    // Count might be longer than the maximal SPI transfer size. In this case, split into chunks
    while (count > 0) {

        if (count > spi_kernel_bufsize) {
            spi_transfer.len = spi_kernel_bufsize;
            count -= spi_kernel_bufsize;
        }
        else {
            spi_transfer.len = count;
            count = 0;
        }
        spi_transfer.rx_buf = (__u64)buf;

        int res = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi_transfer);
        if (res < 0) {
            int err = errno;
            BOOST_LOG_TRIVIAL(error) << "[spisource] SPI ioctl error: " << err;
            switch (err) {
            case EMSGSIZE:
                throw std::ios::failure("Package too long");
            }
            throw std::ios::failure("Unknown error");
        }

        // position to next chunk (might not be needed)
        spi_transfer.rx_buf += spi_kernel_bufsize;
    }

}

SPIException::SPIException(const string msg)
{
}

SPIException::~SPIException()
{
}
