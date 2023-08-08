/*
 * This file is part of the libjaylink project.
 *
 * Copyright (C) 2021-2022 Marc Schink <dev@zapb.de>
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
 * Silicon Labs 2-wire (C2) interface functions.
 */

/** @cond PRIVATE */
#define CMD_C2_IO		0x17

#define C2_CMD_DATA_READ	0x00
#define C2_CMD_DATA_WRITE	0x01
#define C2_CMD_ADDR_READ	0x02
#define C2_CMD_ADDR_WRITE	0x03
/** @endcond */

/**
 * Read the C2 address register.
 *
 * @note This function must only be used if the #JAYLINK_TIF_C2 interface is
 *       available and selected.
 *
 * @param[in,out] devh Device handle.
 * @param[out] address Value read from the address register on success, and
 *                     undefined on failure.
 *
 * @retval JAYLINK_OK Success.
 * @retval JAYLINK_ERR_ARG Invalid arguments.
 * @retval JAYLINK_ERR_TIMEOUT A timeout occurred.
 * @retval JAYLINK_ERR_IO Input/output error.
 * @retval JAYLINK_ERR_DEV Unspecified device error.
 * @retval JAYLINK_ERR Other error conditions.
 *
 * @since 0.3.0
 */
JAYLINK_API int jaylink_c2_read_address(struct jaylink_device_handle *devh,
		uint8_t *address)
{
	int ret;
	uint8_t buf[6];
	struct jaylink_context *ctx;

	if (!devh || !address)
		return JAYLINK_ERR_ARG;

	ctx = devh->dev->ctx;

	ret = transport_start_write_read(devh, 5, 1 + 4, true);

	if (ret != JAYLINK_OK) {
		log_err(ctx, "transport_start_write_read() failed: %s",
			jaylink_strerror(ret));
		return ret;
	}

	buf[0] = CMD_C2_IO;
	buf[1] = C2_CMD_ADDR_READ;
	buf[2] = 0x00;
	/* Length is always 1 for this command. */
	buffer_set_u16(buf, 1, 3);

	ret = transport_write(devh, buf, 5);

	if (ret != JAYLINK_OK) {
		log_err(ctx, "transport_write() failed: %s",
			jaylink_strerror(ret));
		return ret;
	}

	ret = transport_read(devh, address, 1);

	if (ret != JAYLINK_OK) {
		log_err(ctx, "transport_read() failed: %s",
			jaylink_strerror(ret));
		return ret;
	}

	ret = transport_read(devh, buf, 4);

	if (ret != JAYLINK_OK) {
		log_err(ctx, "transport_read() failed: %s",
			jaylink_strerror(ret));
		return ret;
	}

	uint32_t err = buffer_get_u32(buf, 0);

	if (err != 0) {
		return JAYLINK_ERR_DEV;
	}

	return JAYLINK_OK;
}

/**
 * Write to the C2 address register.
 *
 * @note This function must only be used if the #JAYLINK_TIF_C2 interface is
 *       available and selected.
 *
 * @param[in,out] devh Device handle.
 * @param[in] address Value to write into the address register.
 *
 * @retval JAYLINK_OK Success.
 * @retval JAYLINK_ERR_ARG Invalid arguments.
 * @retval JAYLINK_ERR_TIMEOUT A timeout occurred.
 * @retval JAYLINK_ERR_IO Input/output error.
 * @retval JAYLINK_ERR_DEV Unspecified device error.
 * @retval JAYLINK_ERR Other error conditions.
 *
 * @since 0.3.0
 */
JAYLINK_API int jaylink_c2_write_address(struct jaylink_device_handle *devh,
		uint8_t address)
{
	int ret;
	uint8_t buf[6];
	struct jaylink_context *ctx;

	if (!devh)
		return JAYLINK_ERR_ARG;

	ctx = devh->dev->ctx;

	ret = transport_start_write_read(devh, 5 + 1, 4, true);

	if (ret != JAYLINK_OK) {
		log_err(ctx, "transport_start_write_read() failed: %s",
			jaylink_strerror(ret));
		return ret;
	}

	buf[0] = CMD_C2_IO;
	buf[1] = C2_CMD_ADDR_WRITE;
	/* Length is always 1 for this command. */
	buffer_set_u16(buf, 1, 2);
	buf[4] = 0x00;
	buf[5] = address;

	ret = transport_write(devh, buf, 6);

	if (ret != JAYLINK_OK) {
		log_err(ctx, "transport_write() failed: %s",
			jaylink_strerror(ret));
		return ret;
	}

	ret = transport_read(devh, buf, 4);

	if (ret != JAYLINK_OK) {
		log_err(ctx, "transport_read() failed: %s",
			jaylink_strerror(ret));
		return ret;
	}

	uint32_t err = buffer_get_u32(buf, 0);

	if (err != 0) {
		return JAYLINK_ERR_DEV;
	}

	return JAYLINK_OK;
}

/**
 * Read the C2 data register.
 *
 * @note This function must only be used if the #JAYLINK_TIF_C2 interface is
 *       available and selected.
 *
 * @param[in,out] devh Device handle.
 * @param[out] data Buffer to store the read data on success. Its content is
 *                  undefined on failure.
 * @param[in] length Number of bytes to read, but not more than
 *                   #JAYLINK_C2_MAX_LENGTH.
 *
 * @retval JAYLINK_OK Success.
 * @retval JAYLINK_ERR_ARG Invalid arguments.
 * @retval JAYLINK_ERR_TIMEOUT A timeout occurred.
 * @retval JAYLINK_ERR_IO Input/output error.
 * @retval JAYLINK_ERR_DEV Unspecified device error.
 * @retval JAYLINK_ERR Other error conditions.
 *
 * @since 0.3.0
 */
JAYLINK_API int jaylink_c2_read_data(struct jaylink_device_handle *devh,
		uint8_t *data, uint8_t length)
{
	int ret;
	struct jaylink_context *ctx;
	uint8_t buf[5];

	if (!devh || !data)
		return JAYLINK_ERR_ARG;

	if (length > JAYLINK_C2_MAX_LENGTH)
		return JAYLINK_ERR_ARG;

	ctx = devh->dev->ctx;

	ret = transport_start_write_read(devh, 5, 4 + length, true);

	if (ret != JAYLINK_OK) {
		log_err(ctx, "transport_start_write_read() failed: %s",
			jaylink_strerror(ret));
		return ret;
	}

	buf[0] = CMD_C2_IO;
	buf[1] = C2_CMD_DATA_READ;
	buf[2] = 0x00;
	buffer_set_u16(buf, length, 3);

	ret = transport_write(devh, buf, 5);

	if (ret != JAYLINK_OK) {
		log_err(ctx, "transport_write() failed: %s",
			jaylink_strerror(ret));
		return ret;
	}

	ret = transport_read(devh, data, length);

	if (ret != JAYLINK_OK) {
		log_err(ctx, "transport_read() failed: %s",
			jaylink_strerror(ret));
		return ret;
	}

	ret = transport_read(devh, buf, 4);

	if (ret != JAYLINK_OK) {
		log_err(ctx, "transport_read() failed: %s",
			jaylink_strerror(ret));
		return ret;
	}

	uint32_t err = buffer_get_u32(buf, 0);

	if (err != 0) {
		return JAYLINK_ERR_DEV;
	}

	return JAYLINK_OK;
}

/**
 * Write to the C2 data register.
 *
 * @note This function must only be used if the #JAYLINK_TIF_C2 interface is
 *       available and selected.
 *
 * @param[in,out] devh Device handle.
 * @param[in] data Buffer to be written into the data register.
 * @param[in] length Number of bytes to write, but not more than
 *                   #JAYLINK_C2_MAX_LENGTH.
 *
 * @retval JAYLINK_OK Success.
 * @retval JAYLINK_ERR_ARG Invalid arguments.
 * @retval JAYLINK_ERR_TIMEOUT A timeout occurred.
 * @retval JAYLINK_ERR_IO Input/output error.
 * @retval JAYLINK_ERR_DEV Unspecified device error.
 * @retval JAYLINK_ERR Other error conditions.
 *
 * @since 0.3.0
 */
JAYLINK_API int jaylink_c2_write_data(struct jaylink_device_handle *devh,
		const uint8_t *data, uint8_t length)
{
	int ret;
	uint8_t buf[5];
	struct jaylink_context *ctx;

	if (!devh || !data)
		return JAYLINK_ERR_ARG;

	if (length > JAYLINK_C2_MAX_LENGTH)
		return JAYLINK_ERR_ARG;

	ctx = devh->dev->ctx;

	ret = transport_start_write_read(devh, 5 + length, 4, true);

	if (ret != JAYLINK_OK) {
		log_err(ctx, "transport_start_write_read() failed: %s",
			jaylink_strerror(ret));
		return ret;
	}

	buf[0] = CMD_C2_IO;
	buf[1] = C2_CMD_DATA_WRITE;
	buffer_set_u16(buf, length, 2);
	buf[4] = 0x00;

	ret = transport_write(devh, buf, 5);

	if (ret != JAYLINK_OK) {
		log_err(ctx, "transport_write() failed: %s",
			jaylink_strerror(ret));
		return ret;
	}

	ret = transport_write(devh, data, length);

	if (ret != JAYLINK_OK) {
		log_err(ctx, "transport_write() failed: %s",
			jaylink_strerror(ret));
		return ret;
	}

	ret = transport_read(devh, buf, 4);

	if (ret != JAYLINK_OK) {
		log_err(ctx, "transport_read() failed: %s",
			jaylink_strerror(ret));
		return ret;
	}

	uint32_t err = buffer_get_u32(buf, 0);

	if (err != 0) {
		return JAYLINK_ERR_DEV;
	}

	return JAYLINK_OK;
}
