#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "xorcrypt.h"

void die(const char *message)
{
        if(errno){
                perror(message);
        } else {
                printf("ERROR:%s\n", message);
        }

        exit(1);
}

void readBytesFromFile(int start, int length, const char *filename, unsigned char *bytes)
{
	FILE *file;

	file = fopen(filename, "rb");
	if(!file) die("Could not open the file at this time");

        fseek(file, start, SEEK_SET);

	int freadsuccess = fread(bytes, sizeof(char), length, file);
	if(!freadsuccess) die("Read unsuccessful");

	fclose(file);
}

void appendBytesToFile(int length, const char *filename, unsigned char *bytes)
{
        FILE *file;

        file = fopen(filename, "a");
        if(!file) die("Could not open the file at this time");

        int fwritesuccess = fwrite(bytes, sizeof(char), length, file);
        if(!fwritesuccess) die ("Write unsuccessful at this time");

        fclose(file);
}

void xorBytes(int length, unsigned char *unencryptedbytes, unsigned char *randombytes, unsigned char *encryptedbytes)
{
	int i;
	for(i = 0;i <= length;i++){
		*(encryptedbytes + i) = *(unencryptedbytes + i) ^ *(randombytes + i);
	}
}

void clearFile(const char *filename)
{
	FILE *file;

	file = fopen(filename, "w");
        if(!file) die("Could not open the file at this time");

	fclose(file);
}

void secureDelete(int buffer, int filelength, unsigned char *randombytes, const char *random, const char *filename)
{
	clearFile(filename);

	int i;
	int breakout = 0;
	for(i = 0; breakout == 0; i++){

		if((i + 1) * buffer >= filelength){
			breakout = 1;
			buffer = filelength - (i * buffer);
		}

		readBytesFromFile(0, buffer, random, randombytes);

		appendBytesToFile(buffer, filename, randombytes);
	}

	remove(filename);
}

int main(int argc, char *argv[])
{
	const char *outfile = "out.xor";
	int securedelete = 0;
	const char *random = "/dev/random";
	const char *rndfile = "rnd.key";
	int decrypt = 0;
	int buffer = 1048576;
	int outset = 0;
	int c;

	while ((c = getopt(argc, argv, "o:sux:b:r:")) != -1){
		switch(c){
			case 'o':
				outset = 1;
				outfile = optarg;
				break;
			case 's':
				if(decrypt) die("conflicting arguments");

				securedelete = 1;
				break;
			case 'u':
				if(decrypt) die("conflicting arguments");

				random = "/dev/urandom";
				break;
			case 'x':
				if(strcmp(rndfile, "rnd.key")) die("conflicting arguments");
				if(!strcmp(random, "/dev/urandom")) die("conflicting arguments");
				if(securedelete) die("conflicting arguments");

				decrypt = 1;
				rndfile = optarg;
				random = rndfile;
				if(!outset) outfile = "out";
				break;
			case 'b':
				buffer = atoi(optarg);
				break;
			case 'r':
				if(decrypt) die("conflicting arguments");

				rndfile = optarg;
				break;
			case '?':
				return 1;
		}
	}

	unsigned char *unencryptedbytes;
	unsigned char *randombytes;
	unsigned char *encryptedbytes;

	FILE *file;
	long filelength;

	if (!argv[optind]) die("Usage: xorcrypt file");

	const char *filename = argv[optind];

	//grabs the length of the unencrypted file
	file = fopen(filename, "rb");
	fseek(file, 0L, SEEK_END);
	filelength = ftell(file);
	rewind(file);
	fclose(file);

	//allocates memory for encrypted, unencrypted, and random bytes
	unencryptedbytes = calloc(sizeof(char), buffer);
	randombytes = calloc(sizeof(char), buffer);
	encryptedbytes = calloc(sizeof(char), buffer);

	remove(outfile);
	if(!decrypt) remove(rndfile);

	int i;
	int breakout = 0;
	int start;
	int permbuffer = buffer;
	for(i = 0; breakout == 0; i++){

		start = i*buffer;

		if((i + 1) * buffer >= filelength){
			breakout = 1;
			buffer = filelength - (i * buffer);
		}

		readBytesFromFile(start, buffer, filename, unencryptedbytes);
		if(decrypt) readBytesFromFile(start, buffer, random, randombytes);
		else readBytesFromFile(0, buffer, random, randombytes);

		xorBytes(buffer, unencryptedbytes, randombytes, encryptedbytes);

		appendBytesToFile(buffer, outfile, encryptedbytes);
		if(!decrypt) appendBytesToFile(buffer, rndfile, randombytes);
	}
	buffer = permbuffer;
	
	if(securedelete && !decrypt) secureDelete(buffer, filelength, randombytes, random, filename);

	free(unencryptedbytes);
	free(encryptedbytes);
	free(randombytes);

	return 0;
}
