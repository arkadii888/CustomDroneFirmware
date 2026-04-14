#include "SPI.h"

SPI::SPI() {
    spi_bus_config_t buscfg = {};
    buscfg.miso_io_num = 43;
    buscfg.mosi_io_num = 14;
    buscfg.sclk_io_num = 44;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 8193;

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
}
