/*
 * sec_conf.h
 *
 *  Created on: 2018. jan. 9.
 *      Author: sassb
 */

#ifndef EXAMPLE_USER_SEC_CONF_H_
#define EXAMPLE_USER_SEC_CONF_H_

#define DEFAULT_TIMEOUT			1000
#define MAX_BUFFER_SIZE         512

#include <wolfmqtt/mqtt_client.h>
#include "stdio.h"
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <wolfssl/certs_test.h>
#include <wolfssl/wolfio.h>
#include <wolfssl_net.h>
#include "main.h"


typedef struct WolfSocketContext {
	uint32_t id;
} WolfSocketContext;

int WolfsslReadCallback(WOLFSSL* ssl, char* buf, int sz, void* context);
int WolfsslWriteCallback(WOLFSSL* ssl, char* buf, int sz, void* context);
static int Wolfssl_TlsConnect(const char *host, int port);
void wolfSSL_Logging_cb_f(const int logLevel, const char * const logMessage);


#endif /* EXAMPLE_USER_SEC_CONF_H_ */
