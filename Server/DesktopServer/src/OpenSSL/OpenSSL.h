/*
 * Encrypt.h
 * This file is used to declare base encrypt functions
 * */


#pragma once
/* Core libraries */
#include <openssl/ssl.h>




SSL_CTX* set_up_server_SSL_certificate(const char* publicKeyFile, const char* privateKeyFile);
void print_all_SSL_errors();

