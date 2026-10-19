/*
 * Sending System
 * Copyright (C) 2025-2026 ClassZak
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "OpenSSL.h"
#include "../PrintProcedures/PrintProcedures.h"
#include <openssl/err.h>




/*
 * This function is used to set up SSL certificate for server
 * @param publicKeyFile is full path of public key
 * @param privateKeyFile is full path of private key
 * @returns SSL context pointer
 * */
SSL_CTX* set_up_server_SSL_certificate(const char* publicKeyFile, const char* privateKeyFile) {
	SSL_CTX* ctx;
	if (!(ctx = SSL_CTX_new(TLS_server_method()))) {
		return NULL;
	}

	if (1 != SSL_CTX_use_certificate_file(ctx, publicKeyFile, SSL_FILETYPE_PEM)) {
		SSL_CTX_free(ctx);
		return NULL;
	}

	if (1 != SSL_CTX_use_PrivateKey_file(ctx, privateKeyFile, SSL_FILETYPE_PEM)) {
		SSL_CTX_free(ctx);
		return NULL;
	}

	if (1 != SSL_CTX_check_private_key(ctx)) {
		SSL_CTX_free(ctx);
		return NULL;
	}

	return ctx;
}




/*
 * This function is used to print all SSL errors
 * */
void print_all_SSL_errors() {
	BIO *bio = BIO_new(BIO_s_mem());
	if (!bio) {
		print_error("Failed to create BIO to print SSL errors\n");
		return;
	}

	ERR_print_errors(bio);

	char* print_data_ptr = NULL;
	long length = BIO_get_mem_data(bio, print_data_ptr);

	if (length == 0 || print_data_ptr == NULL) {
		print_error("Failed to receive errors from BIO\n");
		return;
	}

	print_error(print_data_ptr);

	BIO_free(bio);
}
