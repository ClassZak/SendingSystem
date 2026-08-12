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
