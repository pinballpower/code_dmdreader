#include "spi.h"

#include <ios>

#include <boost/log/trivial.hpp>


void SPI::openDevice(const string spiDevice, unsigned int spiSpeed, unsigned int spiFlags)
{
    int i;
    char  spiMode;
    char  spiBits = 8;
    char dev[32];

    spiMode = spiFlags & 3;

    if ((fileDescriptor = open(spiDevice.c_str(), O_RDWR)) < 0)
    {
        throw SPIException("couldn't open SPI device " + spiDevice);
    }

    if (ioctl(fileDescriptor, SPI_IOC_WR_MODE, &spiMode) < 0)
    {
        closeDevice();
        throw SPIException("couldn't set SPI mode");
    }

    if (ioctl(fileDescriptor, SPI_IOC_WR_BITS_PER_WORD, &spiBits) < 0)
    {
        closeDevice();
        throw SPIException("couldn't set SPI bits/word");
    }

    if (ioctl(fileDescriptor, SPI_IOC_WR_MAX_SPEED_HZ, &spiSpeed) < 0)
    {
        closeDevice();
        throw SPIException("couldn't set SPI speed");
    }

    speed = spiSpeed;
}

void SPI::closeDevice()
{
    close(fileDescriptor);
    fileDescriptor = 0;
}

void SPI::readData(uint8_t *buf, unsigned int count) const
{
    if (fileDescriptor <= 0) {
        throw SPIException("SPI not ready");
    }

    static  struct spi_ioc_transfer spi_transfer;
    memset(&spi_transfer, 0, sizeof(spi_transfer));
    spi_transfer.tx_buf = 0;
    spi_transfer.rx_buf = 0;
    spi_transfer.len = 0;
    spi_transfer.speed_hz = this->speed;
    spi_transfer.delay_usecs = 0;
    spi_transfer.bits_per_word = 8;
    spi_transfer.cs_change = 0;


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

        int res = ioctl(fileDescriptor, SPI_IOC_MESSAGE(1), &spi_transfer);
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
