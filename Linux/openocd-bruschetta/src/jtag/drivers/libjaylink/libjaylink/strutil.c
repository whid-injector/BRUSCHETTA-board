/*
 * This file is part of the libjaylink project.
 *
 * Copyright (C) 2016 Marc Schink <jaylink-dev@marcschink.de>
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

#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include "libjaylink.h"

/**
 * @file
 *
 * String utility functions.
 */

/**
 * Convert a string representation of a serial number to an integer.
 *
 * The string representation of the serial number must be in decimal form.
 *
 * @param[in] str String representation to convert.
 * @param[out] serial_number Serial number on success, and undefined on
 *                           failure.
 *
 * @retval JAYLINK_OK Success.
 * @retval JAYLINK_ERR_ARG Invalid arguments.
 * @retval JAYLINK_ERR Conversion error. Serial number is invalid or string
 *                     representation contains invalid character(s).
 *
 * @since 0.1.0
 */
JAYLINK_API int jaylink_parse_serial_number(const char *str,
		uint32_t *serial_number)
{
	char *end_ptr;
	unsigned long long tmp;

	if (!str || !serial_number)
		return JAYLINK_ERR_ARG;

	errno = 0;
	tmp = strtoull(str, &end_ptr, 10);

	if (*end_ptr != '\0' || errno != 0 || tmp > UINT32_MAX)
		return JAYLINK_ERR;

	*serial_number = tmp;

	return JAYLINK_OK;
}

/**
 * Get the string representation of a hardware type.
 *
 * @param[in] type Hardware type.
 *
 * @return The string representation of the given hardware type, or NULL
 *         for an unknown type.
 *
 * @since 0.3.0
 */
JAYLINK_API const char *jaylink_hardware_type_string(
		enum jaylink_hardware_type type)
{
	switch (type) {
	case JAYLINK_HW_TYPE_JLINK:
		return "J-Link";
	case JAYLINK_HW_TYPE_FLASHER:
		return "Flasher";
	case JAYLINK_HW_TYPE_JLINK_PRO:
		return "J-Link PRO";
	default:
		break;
	}

	return NULL;
}

/**
 * Get the string representation of a target interface.
 *
 * @param[in] iface Target interface.
 *
 * @return The string representation of the given target interface, or NULL
 *         for an unknown interface.
 *
 * @since 0.3.0
 */
JAYLINK_API const char *jaylink_target_interface_string(
		enum jaylink_target_interface iface)
{
	switch (iface) {
	case JAYLINK_TIF_JTAG:
		return "JTAG";
	case JAYLINK_TIF_SWD:
		return "SWD";
	case JAYLINK_TIF_BDM3:
		return "BDM3";
	case JAYLINK_TIF_FINE:
		return "FINE";
	case JAYLINK_TIF_2W_JTAG_PIC32:
		return "2-wire JTAG for PIC32";
	case JAYLINK_TIF_SPI:
		return "SPI";
	case JAYLINK_TIF_C2:
		return "C2";
	case JAYLINK_TIF_CJTAG:
		return "cJTAG";
	default:
		break;
	}

	return NULL;
}
