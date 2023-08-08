/*
 * This file is part of the libjaylink project.
 *
 * Copyright (C) 2016-2020 Marc Schink <dev@zapb.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>

#include "libjaylink.h"
#include "libjaylink-internal.h"

/**
 * @file
 *
 * Serial Peripheral Interface (SPI) functions.
 */

/** @cond PRIVATE */
#define CMD_SPI			0x15

#define SPI_CMD_IO		0x01
/** @endcond */

/**
 * Perform SPI I/O operation.
 *
 * The device acts as master and operates in mode 3 (CPOL = 1, CPHA = 1). Data
 * is transferred with the most significant bit (MSB) first.
 *
 * @note This function must only be used if the device has the
 *       #JAYLINK_DEV_CAP_SPI capability and if the #JAYLINK_TIF_SPI interface
 *       is available and selected.
 *
 * @param[in,out] devh Device handle.
 * @param[in] mosi Buffer to read MOSI data from. Can be NULL.
 * @param[out] miso Buffer to store MISO data on success. Its content
 *                  is undefined on failure. The buffer must be large enough to
 *                  contain at least the specified number of bytes to transfer.
 *                  Can be NULL.
 * @param[in] length Number of bytes to transfer.
 * @param[in] flags Flags, see #jaylink_spi_flag for more details.
 *
 * @retval JAYLINK_OK Success.
 * @retval JAYLINK_ERR_ARG Invalid arguments.
 * @retval JAYLINK_ERR_TIMEOUT A timeout occurred.
 * @retval JAYLINK_ERR_PROTO Protocol violation.
 * @retval JAYLINK_ERR Other error conditions.
 *
 * @since 0.3.0
 */
JAYLINK_API int jaylink_spi_io(struct jaylink_device_handle *devh,
		const uint8_t *mosi, uint8_t *miso, uint32_t length,
		uint32_t flags)
{
	int ret;
	struct jaylink_context *ctx;
	uint8_t buf[20];
	uint32_t mosi_length;
	uint32_t miso_length;
	uint32_t num_transferred_bytes;

	if (!devh || !length)
		return JAYLINK_ERR_ARG;

	if (!mosi && !miso)
		return JAYLINK_ERR_ARG;

	mosi_length = (mosi) ? length : 0;
	miso_length = (miso) ? length : 0;
	ctx = devh->dev->ctx;

	buf[0] = CMD_SPI;
	buf[1] = SPI_CMD_IO;
	buf[2] = 0x00;
	buf[3] = 0x00;

	buffer_set_u32(buf, mosi_length + 8, 4);
	buffer_set_u32(buf, miso_length + 4, 8);
	buffer_set_u32(buf, length * 8, 12);
	buffer_set_u32(buf, flags, 16);

	ret = transport_start_write_read(devh, 20 + mosi_length,
		miso_length + 4, true);

	if (ret != JAYLINK_OK) {
		log_err(ctx, "transport_start_write_read() failed: %s",
			jaylink_strerror(ret));
		return ret;
	}

	ret = transport_write(devh, buf, 20);

	if (ret != JAYLINK_OK) {
		log_err(ctx, "transport_write() failed: %s",
			jaylink_strerror(ret));
		return ret;
	}

	if (mosi) {
		ret = transport_write(devh, mosi, mosi_length);

		if (ret != JAYLINK_OK) {
			log_err(ctx, "transport_write() failed: %s",
				jaylink_strerror(ret));
			return ret;
		}
	}

	if (miso) {
		ret = transport_read(devh, miso, miso_length);

		if (ret != JAYLINK_OK) {
			log_err(ctx, "transport_read() failed: %s",
				jaylink_strerror(ret));
			return ret;
		}
	}

	ret = transport_read(devh, buf, 4);

	if (ret != JAYLINK_OK) {
		log_err(ctx, "transport_read() failed: %s",
			jaylink_strerror(ret));
		return ret;
	}

	num_transferred_bytes = buffer_get_u32(buf, 0);

	if (num_transferred_bytes != length) {
		log_err(ctx, "Unexpected number of transferred bytes");
		return JAYLINK_ERR_PROTO;
	}

	return JAYLINK_OK;
}
