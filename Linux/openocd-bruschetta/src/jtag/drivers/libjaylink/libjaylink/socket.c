/*
 * This file is part of the libjaylink project.
 *
 * Copyright (C) 2016-2017 Marc Schink <jaylink-dev@marcschink.de>
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

#include <errno.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#include "libjaylink.h"
#include "libjaylink-internal.h"

/**
 * @file
 *
 * Socket abstraction layer.
 */

/**
 * Initiate a connection on a socket.
 *
 * @param[in] sock Socket descriptor.
 * @param[in] address Address to establish the connection.
 * @param[in] length Length of the structure pointed to by @p address in bytes.
 * @param[in] timeout Connection timeout in milliseconds, 0 for no timeout.
 *
 * @retval JAYLINK_OK Success.
 * @retval JAYLINK_ERR_ARG Invalid arguments.
 * @retval JAYLINK_ERR_TIMEOUT A timeout occurred.
 * @retval JAYLINK_ERR Other error conditions.
 */
JAYLINK_PRIV int socket_connect(int sock, const struct sockaddr *address,
		size_t address_length, size_t timeout)
{
	int ret;
	fd_set fds;
	struct timeval tv;
	int socket_error;
	size_t option_length;

	if (!address)
		return JAYLINK_ERR_ARG;

	if (!socket_set_blocking(sock, false))
		return JAYLINK_ERR;
#ifdef _WIN32
	ret = connect(sock, address, address_length);

	if (ret != 0 && WSAGetLastError() != WSAEWOULDBLOCK)
		return JAYLINK_ERR;
#else
	errno = 0;
	ret = connect(sock, address, address_length);

	if (ret != 0 && errno != EINPROGRESS)
		return JAYLINK_ERR;
#endif
	if (!ret)
		return JAYLINK_OK;

	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;

	ret = select(sock + 1, NULL, &fds, NULL, &tv);

	socket_set_blocking(sock, true);

	if (!ret)
		return JAYLINK_ERR_TIMEOUT;

	option_length = sizeof(socket_error);

	if (!socket_get_option(sock, SOL_SOCKET, SO_ERROR, &socket_error,
			&option_length))
		return JAYLINK_ERR;

	if (!socket_error)
		return JAYLINK_OK;

	return JAYLINK_ERR;
}

/**
 * Close a socket.
 *
 * @param[in] sock Socket descriptor.
 *
 * @return Whether the socket was successfully closed.
 */
JAYLINK_PRIV bool socket_close(int sock)
{
	int ret;

#ifdef _WIN32
	ret = closesocket(sock);
#else
	ret = close(sock);
#endif

	if (!ret)
		return true;

	return false;
}

/**
 * Bind an address to a socket.
 *
 * @param[in] sock Socket descriptor.
 * @param[in] address Address to be bound to the socket.
 * @param[in] length Length of the structure pointed to by @p address in bytes.
 *
 * @return Whether the address was successfully assigned to the socket.
 */
JAYLINK_PRIV bool socket_bind(int sock, const struct sockaddr *address,
		size_t length)
{
	int ret;

	ret = bind(sock, address, length);

#ifdef _WIN32
	if (ret == SOCKET_ERROR)
		return false;
#else
	if (ret < 0)
		return false;
#endif

	return true;
}

/**
 * Send a message on a socket.
 *
 * @param[in] sock Socket descriptor.
 * @param[in] buffer Buffer of the message to be sent.
 * @param[in,out] length Length of the message in bytes. On success, the value
 *                       gets updated with the actual number of bytes sent. The
 *                       value is undefined on failure.
 * @param[in] flags Flags to modify the function behaviour. Use bitwise OR to
 *                  specify multiple flags.
 *
 * @return Whether the message was sent successfully.
 */
JAYLINK_PRIV bool socket_send(int sock, const void *buffer, size_t *length,
		int flags)
{
	ssize_t ret;

	ret = send(sock, buffer, *length, flags);
#ifdef _WIN32
	if (ret == SOCKET_ERROR)
		return false;
#else
	if (ret < 0)
		return false;
#endif
	*length = ret;

	return true;
}

/**
 * Receive a message from a socket.
 *
 * @param[in] sock Socket descriptor.
 * @param[out] buffer Buffer to store the received message on success. Its
 *                    content is undefined on failure.
 * @param[in,out] length Maximum length of the message in bytes. On success,
 *                       the value gets updated with the actual number of
 *                       received bytes. The value is undefined on failure.
 * @param[in] flags Flags to modify the function behaviour. Use bitwise OR to
 *                  specify multiple flags.
 *
 * @return Whether a message was successfully received.
 */
JAYLINK_PRIV bool socket_recv(int sock, void *buffer, size_t *length,
		int flags)
{
	ssize_t ret;

	ret = recv(sock, buffer, *length, flags);

#ifdef _WIN32
	if (ret == SOCKET_ERROR)
		return false;
#else
	if (ret < 0)
		return false;
#endif

	*length = ret;

	return true;
}

/**
 * Send a message on a socket.
 *
 * @param[in] sock Socket descriptor.
 * @param[in] buffer Buffer to send message from.
 * @param[in,out] length Number of bytes to send. On success, the value gets
 *                       updated with the actual number of bytes sent. The
 *                       value is undefined on failure.
 * @param[in] flags Flags to modify the function behaviour. Use bitwise OR to
 *                  specify multiple flags.
 * @param[in] address Destination address of the message.
 * @param[in] address_length Length of the structure pointed to by @p address
 *                           in bytes.
 *
 * @return Whether the message was successfully sent.
 */
JAYLINK_PRIV bool socket_sendto(int sock, const void *buffer, size_t *length,
		int flags, const struct sockaddr *address,
		size_t address_length)
{
	ssize_t ret;

	ret = sendto(sock, buffer, *length, flags, address, address_length);

#ifdef _WIN32
	if (ret == SOCKET_ERROR)
		return false;
#else
	if (ret < 0)
		return false;
#endif

	*length = ret;

	return true;
}

/**
 * Receive a message from a socket.
 *
 * @param[in] sock Socket descriptor.
 * @param[out] buffer Buffer to store the received message on success. Its
 *                    content is undefined on failure.
 * @param[in,out] length Maximum length of the message in bytes. On success,
 *                       the value gets updated with the actual number of
 *                       received bytes. The value is undefined on failure.
 * @param[in] flags Flags to modify the function behaviour. Use bitwise OR to
 *                  specify multiple flags.
 * @param[out] address Structure to store the source address of the message on
 *                     success. Its content is undefined on failure.
 *                     Can be NULL.
 * @param[in,out] address_length Length of the structure pointed to by
 *                               @p address in bytes. On success, the value
 *                               gets updated with the actual length of the
 *                               structure. The value is undefined on failure.
 *                               Should be NULL if @p address is NULL.
 *
 * @return Whether a message was successfully received.
 */
JAYLINK_PRIV bool socket_recvfrom(int sock, void *buffer, size_t *length,
		int flags, struct sockaddr *address, size_t *address_length)
{
	ssize_t ret;
#ifdef _WIN32
	int tmp;

	tmp = *address_length;
	ret = recvfrom(sock, buffer, *length, flags, address, &tmp);

	if (ret == SOCKET_ERROR)
		return false;
#else
	socklen_t tmp;

	tmp = *address_length;
	ret = recvfrom(sock, buffer, *length, flags, address, &tmp);

	if (ret < 0)
		return false;
#endif

	*address_length = tmp;
	*length = ret;

	return true;
}

/**
 * Get the value of a socket option.
 *
 * @param[in] sock Socket descriptor.
 * @param[in] level Level at which the option is defined.
 * @param[in] option Option to get the value for.
 * @param[in] value Buffer to store the value.
 * @param[in] length Length of the value buffer in bytes.
 *
 * @return Whether the option value was retrieved successfully.
 */
JAYLINK_PRIV bool socket_get_option(int sock, int level, int option,
		void *value, size_t *length)
{
	if (!getsockopt(sock, level, option, value, (socklen_t *)length))
		return true;

	return false;
}

/**
 * Set the value of a socket option.
 *
 * @param[in] sock Socket descriptor.
 * @param[in] level Level at which the option is defined.
 * @param[in] option Option to set the value for.
 * @param[in] value Buffer of the value to be set.
 * @param[in] length Length of the value buffer in bytes.
 *
 * @return Whether the option value was set successfully.
 */
JAYLINK_PRIV bool socket_set_option(int sock, int level, int option,
		const void *value, size_t length)
{
	if (!setsockopt(sock, level, option, value, length))
		return true;

	return false;
}

/**
 * Set the blocking mode of a socket.
 *
 * @param[in] sock Socket descriptor.
 * @param[in] blocking Blocking mode.
 *
 * @return Whether the blocking mode was set successfully.
 */
JAYLINK_PRIV bool socket_set_blocking(int sock, bool blocking)
{
	int ret;
#ifdef _WIN32
	u_long mode;

	mode = !blocking;
	ret = ioctlsocket(sock, FIONBIO, &mode);

	if (ret != NO_ERROR)
		return false;
#else
	int flags;

	flags = fcntl(sock, F_GETFL, 0);

	if (flags < 0)
		return false;

	if (blocking)
		flags &= ~O_NONBLOCK;
	else
		flags |= O_NONBLOCK;

	ret = fcntl(sock, F_SETFL, flags);

	if (ret != 0)
		return false;
#endif
	return true;
}
