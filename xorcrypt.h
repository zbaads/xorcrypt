#ifndef _xorcrypt_h
#define _db_h

void die(const char *message);
void readBytesFromFile(int start, int length, const char *filename, unsigned char *bytes);
void appendBytesToFile(int length, const char *filename, unsigned char *bytes);
void xorBytes(int length, unsigned char *unencryptedbytes, unsigned char *randombytes, unsigned char *encryptedbytes);
void clearFile(const char *filename);
void secureDelete(int buffer, int filelength, unsigned char *randombytes, const char *random, const char *filename);

#endif
