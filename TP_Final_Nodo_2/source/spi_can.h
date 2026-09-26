/* Copyright 2026, DSI FCEIA UNR - Sistemas Digitales 2
 *    DSI: http://www.dsi.fceia.unr.edu.ar/
 * Copyright 2026, Agustin M. Zuliani (amzuliani02@gmail.com)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef INCLUDE_SPI_H_
#define INCLUDE_SPI_H_

/* Archivos */
#include "SD2_board.h"
#include <stdint.h>
#include "fsl_common.h"

/* Funciones */
/**
 * @brief Inicializacion del spi
 */
extern void spi_init(void);
/**
 * @brief Escritura de datos
 * @param[in] tx_buffer buffer donde se manda la informacion
 * @param[in] n numeros de bytes de datos
 * @return Estado de la transmision
 */
extern status_t spi_write(uint8_t *tx_buffer, uint16_t n);
/**
 * @brief Recepcion de datos
 * @param[out] rx_buffer buffer donde se cargan los datos
 * @param[in] n numeros de bytes
 * @return Estado de la recepcion
 */
extern status_t spi_receive(uint8_t *rx_buffer, uint8_t n);
/**
 * @brief	Pone el pin del chip select de la placa en
 * 			estado alto, finalizando la comunicacion.
 */
extern void spi_cs_high();
/**
 *	@brief	Pone el pin del chip select de la placa en
 *			estado bajo, activando la comunicacion.
 */
extern void spi_cs_low();

#endif /* INCLUDE_SPI_H_ */
