/*
 *  Lightweight UDP sockets abstraction for rapid implementation of message-oriented protocols
 *  Copyright (c) 2019 Stanislav Denisov
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#ifndef NANOSOCKETS_H
#define NANOSOCKETS_H

#include <stdint.h>

#define NANOSOCKETS_VERSION_MAJOR 1
#define NANOSOCKETS_VERSION_MINOR 0
#define NANOSOCKETS_VERSION_PATCH 1

#ifdef _WIN32
	#define NANOSOCKETS_WINDOWS 1
#elif defined __unix__
	#define NANOSOCKETS_UNIX 2
#elif defined __APPLE__
	#define NANOSOCKETS_MAC 3
#endif

#if defined(NANOSOCKETS_WINDOWS) && defined(NANOSOCKETS_DLL)
	#ifdef NANOSOCKETS_IMPLEMENTATION
		#define NANOSOCKETS_API __declspec(dllexport)
	#else
		#define NANOSOCKETS_API __declspec(dllimport)
	#endif
#else
	#define NANOSOCKETS_API extern
#endif

#ifdef NANOSOCKETS_WINDOWS
	#include <ws2tcpip.h>
#else
	#include <netinet/in.h>
#endif

#define NANOSOCKETS_HOSTNAME_SIZE 1025

// API

#ifdef __cplusplus
extern "C" {
#endif

	typedef int64_t NanoSocket;

	typedef enum _NanoStatus {
		NANOSOCKETS_STATUS_OK = 0,
		NANOSOCKETS_STATUS_ERROR = -1
	} NanoStatus;

	typedef struct _NanoAddress {
		union {
			struct in6_addr ipv6;
			struct {
				uint8_t zeros[10];
				uint16_t ffff;
				struct in_addr ip;
			} ipv4;
		};
		uint16_t port;
	} NanoAddress;

	NANOSOCKETS_API NanoStatus nanosockets_initialize(void);

	NANOSOCKETS_API void nanosockets_deinitialize(void);

	NANOSOCKETS_API NanoSocket nanosockets_create(int, int);

	NANOSOCKETS_API void nanosockets_destroy(NanoSocket*);

	NANOSOCKETS_API int nanosockets_bind(NanoSocket, const NanoAddress*);

	NANOSOCKETS_API int nanosockets_connect(NanoSocket, const NanoAddress*);

	NANOSOCKETS_API NanoStatus nanosockets_set_nonblocking(NanoSocket);

	NANOSOCKETS_API int nanosockets_poll(NanoSocket, long);

	NANOSOCKETS_API int nanosockets_send(NanoSocket, const NanoAddress*, const uint8_t*, int);

	NANOSOCKETS_API int nanosockets_receive(NanoSocket, NanoAddress*, uint8_t*, int);

	NANOSOCKETS_API NanoStatus nanosockets_address_is_equal(const NanoAddress*, const NanoAddress*);

	NANOSOCKETS_API NanoStatus nanosockets_address_set_ip(NanoAddress*, const char*);

	NANOSOCKETS_API NanoStatus nanosockets_address_get_ip(const NanoAddress*, char*, int);

	NANOSOCKETS_API NanoStatus nanosockets_address_set_hostname(NanoAddress*, const char*);

	NANOSOCKETS_API NanoStatus nanosockets_address_get_hostname(const NanoAddress*, char*, int);

#ifdef __cplusplus
}
#endif

#if defined(NANOSOCKETS_IMPLEMENTATION) && !defined(NANOSOCKETS_IMPLEMENTATION_DONE)
	#define NANOSOCKETS_IMPLEMENTATION_DONE 1

	#include <string.h>

	#ifndef NANOSOCKETS_WINDOWS
		#include <arpa/inet.h>
		#include <fcntl.h>
		#include <netdb.h>
		#include <unistd.h>
		#include <sys/socket.h>
	#endif

	// Macros

	#define NANOSOCKETS_HOST_TO_NET_16(value) (htons(value))
	#define NANOSOCKETS_HOST_TO_NET_32(value) (htonl(value))
	#define NANOSOCKETS_NET_TO_HOST_16(value) (ntohs(value))
	#define NANOSOCKETS_NET_TO_HOST_32(value) (ntohl(value))

	// Functions

	inline static int nanosockets_array_is_zeroed(const uint8_t* array, int length) {
		for (size_t i = 0; i < length; i++) {
			if (array[i] != 0)
				return -1;
		}

		return 0;
	}

	inline static size_t nanosockets_string_copy(char* destination, const char* source, size_t length) {
		char *d = destination;
		const char *s = source;
		size_t n = length;

		if (n != 0 && --n != 0) {
			do {
				if ((*d++ = *s++) == 0)
					break;
			}

			while (--n != 0);
		}

		if (n == 0) {
			if (length != 0)
				*d = '\0';

			while (*s++);
		}

		return (s - source - 1);
	}

	NanoStatus nanosockets_initialize(void) {
		#ifdef NANOSOCKETS_WINDOWS
			WSADATA wsaData = { 0 };

			if (WSAStartup(MAKEWORD(2, 2), &wsaData))
				return NANOSOCKETS_STATUS_ERROR;

			if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
				WSACleanup();

				return NANOSOCKETS_STATUS_ERROR;
			}
		#endif

		return NANOSOCKETS_STATUS_OK;
	}

	void nanosockets_deinitialize(void) {
		#ifdef NANOSOCKETS_WINDOWS
			WSACleanup();
		#endif
	}

	NanoSocket nanosockets_create(int sendBufferSize, int receiveBufferSize) {
		int socketType = SOCK_DGRAM;

		#ifdef SOCK_CLOEXEC
			socketType |= SOCK_CLOEXEC;
		#endif

		NanoSocket socketHandle = socket(PF_INET6, socketType, 0);

		if (socketHandle > -1) {
			int onlyIPv6 = 0;

			if (setsockopt(socketHandle, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&onlyIPv6, sizeof(onlyIPv6)) != 0)
				goto destroy;

			if (setsockopt(socketHandle, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, sizeof(sendBufferSize)) != 0)
				goto destroy;

			if (setsockopt(socketHandle, SOL_SOCKET, SO_RCVBUF, (char*)&receiveBufferSize, sizeof(receiveBufferSize)) != 0)
				goto destroy;

			goto create;

			destroy:

			nanosockets_destroy(&socketHandle);

			return -1;
		}

		create:

		return socketHandle;
	}

	void nanosockets_destroy(NanoSocket* socket) {
		if (*socket > 0) {
			#if NANOSOCKETS_WINDOWS
				closesocket(*socket);
			#else
				close(*socket);
			#endif

			*socket = 0;
		}
	}

	int nanosockets_bind(NanoSocket socket, const NanoAddress* address) {
		struct sockaddr_in6 socketAddress = { 0 };

		socketAddress.sin6_family = AF_INET6;

		if (address == NULL) {
			socketAddress.sin6_addr = in6addr_any;
			socketAddress.sin6_port = 0;
		} else {
			socketAddress.sin6_addr = address->ipv6;
			socketAddress.sin6_port = NANOSOCKETS_HOST_TO_NET_16(address->port);
		}

		return bind(socket, (struct sockaddr*)&socketAddress, sizeof(socketAddress));
	}

	int nanosockets_connect(NanoSocket socket, const NanoAddress* address) {
		struct sockaddr_in6 socketAddress = { 0 };

		socketAddress.sin6_family = AF_INET6;
		socketAddress.sin6_addr = address->ipv6;
		socketAddress.sin6_port = NANOSOCKETS_HOST_TO_NET_16(address->port);

		return connect(socket, (struct sockaddr*)&socketAddress, sizeof(socketAddress));
	}

	NanoStatus nanosockets_set_nonblocking(NanoSocket socket) {
		#ifdef NANOSOCKETS_WINDOWS
			DWORD nonBlocking = 1;

			if (ioctlsocket(socket, FIONBIO, &nonBlocking) != 0)
				return NANOSOCKETS_STATUS_ERROR;
		#else
			int nonBlocking = 1;

			if (fcntl(socket, F_SETFL, O_NONBLOCK, nonBlocking) == -1)
				return NANOSOCKETS_STATUS_ERROR;
		#endif

		return NANOSOCKETS_STATUS_OK;
	}

	int nanosockets_poll(NanoSocket socket, long timeout) {
		fd_set set = { 0 };
		struct timeval time = { 0 };

		FD_ZERO(&set);
		FD_SET(socket, &set);

		time.tv_sec = timeout / 1000;
		time.tv_usec = (timeout % 1000) * 1000;

		#pragma warning(suppress: 4244)
		return select(socket + 1, &set, NULL, NULL, &time);
	}

	int nanosockets_send(NanoSocket socket, const NanoAddress* address, const uint8_t* buffer, int bufferLength) {
		struct sockaddr_in6 socketAddress = { 0 };

		if (address != NULL) {
			socketAddress.sin6_family = AF_INET6;
			socketAddress.sin6_addr = address->ipv6;
			socketAddress.sin6_port = NANOSOCKETS_HOST_TO_NET_16(address->port);
		}

		return sendto(socket, (const char *)buffer, bufferLength, 0, (address != NULL ? (struct sockaddr*)&socketAddress : NULL), sizeof(socketAddress));
	}

	int nanosockets_receive(NanoSocket socket, NanoAddress* address, uint8_t* buffer, int bufferLength) {
		struct sockaddr_storage addressStorage = { 0 };
		socklen_t addressLength = sizeof(addressStorage);

		int socketBytes = recvfrom(socket, (char*)buffer, bufferLength, 0, (struct sockaddr*)&addressStorage, &addressLength);

		if (address != NULL) {
			if (addressStorage.ss_family == AF_INET) {
				struct sockaddr_in* socketAddress = (struct sockaddr_in*)&addressStorage;

				memset(address, 0, sizeof(address->ipv4.zeros));

				address->ipv4.ffff = 0xFFFF;
				address->ipv4.ip = socketAddress->sin_addr;
				address->port = NANOSOCKETS_NET_TO_HOST_16(socketAddress->sin_port);
			} else if (addressStorage.ss_family == AF_INET6) {
				struct sockaddr_in6* socketAddress = (struct sockaddr_in6*)&addressStorage;

				address->ipv6 = socketAddress->sin6_addr;
				address->port = NANOSOCKETS_NET_TO_HOST_16(socketAddress->sin6_port);
			}
		}

		return socketBytes;
	}

	NanoStatus nanosockets_address_is_equal(const NanoAddress* left, const NanoAddress* right) {
		if (memcmp(left, right, sizeof(struct in6_addr)) == 0 && left->port == right->port)
			return NANOSOCKETS_STATUS_OK;
		else
			return NANOSOCKETS_STATUS_ERROR;
	}

	NanoStatus nanosockets_address_set_ip(NanoAddress* address, const char* ip) {
		int type = AF_INET6;
		void* destination = &address->ipv6;

		if (strchr(ip, ':') == NULL) {
			type = AF_INET;

			memset(address, 0, sizeof(address->ipv4.zeros));

			address->ipv4.ffff = 0xFFFF;
			destination = &address->ipv4.ip;
		}

		if (!inet_pton(type, ip, destination))
			return NANOSOCKETS_STATUS_ERROR;

		return NANOSOCKETS_STATUS_OK;
	}

	NanoStatus nanosockets_address_get_ip(const NanoAddress* address, char* ip, int ipLength) {
		if (inet_ntop(AF_INET6, &address->ipv6, ip, ipLength) == NULL)
			return NANOSOCKETS_STATUS_ERROR;

		if (nanosockets_array_is_zeroed(address->ipv4.zeros, sizeof(address->ipv4.zeros)) == 0 && address->ipv4.ffff == 0xFFFF)
			nanosockets_string_copy(ip, ip + 7, ipLength);

		return NANOSOCKETS_STATUS_OK;
	}

	NanoStatus nanosockets_address_set_hostname(NanoAddress* address, const char* name) {
		struct addrinfo addressInfo = { 0 }, *result = NULL, *resultList = NULL;

		addressInfo.ai_family = AF_UNSPEC;

		if (getaddrinfo(name, NULL, &addressInfo, &resultList) != 0)
			return NANOSOCKETS_STATUS_ERROR;

		for (result = resultList; result != NULL; result = result->ai_next) {
			if (result->ai_addr != NULL && result->ai_addrlen >= sizeof(struct sockaddr_in)) {
				if (result->ai_family == AF_INET) {
					struct sockaddr_in* socketAddress = (struct sockaddr_in*)result->ai_addr;

					memset(address, 0, sizeof(address->ipv4.zeros));

					address->ipv4.ffff = 0xFFFF;
					address->ipv4.ip.s_addr = socketAddress->sin_addr.s_addr;

					freeaddrinfo(resultList);

					return NANOSOCKETS_STATUS_OK;
				} else if (result->ai_family == AF_INET6) {
					struct sockaddr_in6* socketAddress = (struct sockaddr_in6*)result->ai_addr;

					address->ipv6 = socketAddress->sin6_addr;

					freeaddrinfo(resultList);

					return NANOSOCKETS_STATUS_OK;
				}
			}
		}

		if (resultList != NULL)
			freeaddrinfo(resultList);

		return nanosockets_address_set_ip(address, name);
	}

	NanoStatus nanosockets_address_get_hostname(const NanoAddress* address, char* name, int nameLength) {
		struct sockaddr_in6 socketAddress = { 0 };

		socketAddress.sin6_family = AF_INET6;
		socketAddress.sin6_addr = address->ipv6;
		socketAddress.sin6_port = NANOSOCKETS_HOST_TO_NET_16(address->port);

		int error = getnameinfo((struct sockaddr*)&socketAddress, sizeof(socketAddress), name, nameLength, NULL, 0, NI_NAMEREQD);

		if (!error) {
			if (name != NULL && nameLength > 0 && !memchr(name, '\0', nameLength))
				return NANOSOCKETS_STATUS_ERROR;

			return NANOSOCKETS_STATUS_OK;
		}

		if (error != EAI_NONAME)
			return NANOSOCKETS_STATUS_ERROR;

		return nanosockets_address_get_ip(address, name, nameLength);
	}

#endif // NANOSOCKETS_IMPLEMENTATION

#endif // NANOSOCKETS_H
