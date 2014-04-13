/*
 * boostrap.c
 *
 *  Created on: Apr 5, 2014
 *      Author: devel
 */

#include <string>
#include <iostream>
#include <build_properties.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

const int kBits = 1024;
const int kExp = 65537;

using namespace std;

std::string replaceAll(std::string subject, const std::string& search,
		const std::string& replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}


/*
 * TODO: a large security bug here. PEM format included into
 * the executable can easily replaced thanks to its headers and
 * footers. Use a binary format instead
 * http://www.openssl.org/docs/crypto/d2i_X509.html
 * http://www.openssl.org/docs/crypto/d2i_RSAPublicKey.html
 *
 */
void writePublicRSAHeader(const std::string& public_fname, RSA* rsa,int random) {
	/* To get the C-string PEM form: */
	//BIO bio_private* = BIO_new_file(const char *filename, "w")
	BIO* bio_private = BIO_new(BIO_s_mem());
	PEM_write_bio_RSAPublicKey(bio_private, rsa);
	int keylen = BIO_pending(bio_private);
	char* pem_key = (char*) (calloc(keylen + 1, 1)); /* Null-terminate */
	BIO_read(bio_private, pem_key, keylen);
	std::string dest = replaceAll(string(pem_key), string("\n"),
			string("\" \\\n\""));
	FILE* fp = fopen(public_fname.c_str(), "w");
	fprintf(fp, "//file generated by bootstrap.c, do not edit.\n\n");
	fprintf(fp, "#ifndef PUBLIC_KEY_H_\n#define PUBLIC_KEY_H_\n");
	fprintf(fp, "#define PUBLIC_KEY \"%s\";\n", dest.c_str());
	fprintf(fp, "#define SHARED_RANDOM %d;\n", abs(random));
	fprintf(fp, "#endif\n");
	fclose(fp);
	BIO_free_all(bio_private);
	free(pem_key);
}

void writePrivateRSAHeader(const std::string& private_fname, RSA* rsa) {
	/* To get the C-string PEM form: */
	//BIO bio_private* = BIO_new_file(const char *filename, "w")
	BIO* bio_private = BIO_new(BIO_s_mem());
	PEM_write_bio_RSAPrivateKey(bio_private, rsa, NULL, NULL, 0, NULL, NULL);
	int keylen = BIO_pending(bio_private);
	char* pem_key = (char*) (calloc(keylen + 1, 1)); /* Null-terminate */
	BIO_read(bio_private, pem_key, keylen);
	std::string dest = replaceAll(string(pem_key), string("\n"),
			string("\\n\" \\\n\""));
	FILE* fp = fopen(private_fname.c_str(), "w");
	fprintf(fp, "//file generated by bootstrap.c, do not edit.\n\n");
	fprintf(fp, "#ifndef PRIVATE_KEY_H_\n#define PRIVATE_KEY_H_\n");
	fprintf(fp, "#define PRIVATE_KEY \"%s\";\n", dest.c_str());
	fprintf(fp, "#endif\n");
	fclose(fp);
	BIO_free_all(bio_private);
	free(pem_key);
}
void generatePk(std::string private_fname,string public_fname) {
	RSA *rsa = RSA_generate_key(kBits, kExp, 0, 0);
	srand(time(NULL)); /* seed random number generator */
	int random=rand();
	/* To get the C-string PEM form: */
	//BIO bio_private* = BIO_new_file(const char *filename, "w")

	writePrivateRSAHeader(private_fname, rsa);
	writePublicRSAHeader(public_fname, rsa, random);
	RSA_free(rsa);
}

void print_usage() {
	printf("usage: bootstrap private-fname public-fname\n");
}

int main(int argc, char** argv) {

	if (argc != 3) {
		print_usage();
		exit(2);
	} else {
		printf("********************************************\n");
		printf("*  Bootstrap!!!                            *\n");
		printf("********************************************\n");

	}
	string private_fname = string(argv[1]);
	string public_fname(argv[2]);

	generatePk(private_fname,public_fname);
	return 0;
}
