#include "SPI.h"

SPI::SPI() {
    spi_bus_config_t config = {};
    config.miso_io_num = 43;
    config.mosi_io_num = 14;
    config.sclk_io_num = 44;
    config.quadwp_io_num = -1;
    config.quadhd_io_num = -1;
    config.max_transfer_sz = 8193;

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &config, SPI_DMA_CH_AUTO));
}
