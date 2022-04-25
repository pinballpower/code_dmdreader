#include "spihelper.h"

#include <boost/log/trivial.hpp>

bool spi_open(string spi_device, unsigned int spi_flags)
{
    int i;
    char  spi_mode;
    char  spi_bits = 8;
    char dev[32];

    spi_mode = spi_flags & 3;

    if ((spi_fd = open(spi_device.c_str(), O_RDWR)) < 0)
    {
        BOOST_LOG_TRIVIAL(error) << "[spisource] couldn't open SPI device " << spi_device;
        return false;
    }

    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &spi_mode) < 0)
    {
        close(spi_fd);
        BOOST_LOG_TRIVIAL(error) << "[spisource] couldn't set SPI mode";
        return false;
    }

    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bits) < 0)
    {
        close(spi_fd);
        BOOST_LOG_TRIVIAL(error) << "[spisource] couldn't set SPI bits/word";
        return false;
    }

    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed) < 0)
    {
        close(spi_fd);
        BOOST_LOG_TRIVIAL(error) << "[spisource] couldn't set SPI speed";
        return false;
    }

    memset(&spi_transfer, 0, sizeof(spi_transfer));

    spi_transfer.tx_buf = 0;
    spi_transfer.rx_buf = (__u64)spi_buffer;
    spi_transfer.len = 0;
    spi_transfer.speed_hz = spi_speed;
    spi_transfer.delay_usecs = 0;
    spi_transfer.bits_per_word = 8;
    spi_transfer.cs_change = 0;

    return true;
}

int spi_close()
{
    return close(spi_fd);
}

int spi_read(unsigned int count)
{
    spi_transfer.len = count;
    return ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi_transfer);
}