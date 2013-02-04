#include "cm_util.h"
#include "lh_httpd.h"
#include <openssl/sha.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

const unsigned BASE64_MAX_LEN = 1024*1024;

Sha1::Sha1(const void *input, unsigned len) {
	if (!input || len == 0)
		return;
	SHA_CTX s;
    SHA1_Init(&s);
    SHA1_Update(&s, input, len);
    SHA1_Final((unsigned char*)buf, &s);
}


static const char encodeCharacterTable[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char decodeCharacterTable[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
    ,-1,62,-1,-1,-1,63,52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21
    ,22,23,24,25,-1,-1,-1,-1,-1,-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
    ,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1
};


Base64::Base64(const void *src, unsigned inlen) {
	if (!src || inlen == 0 || inlen > BASE64_MAX_LEN)
		return;
		
	unsigned char *input = (unsigned char*)src;
    char buff1[3];
    char buff2[4];
    unsigned char i=0, j;
    unsigned input_cnt=0;
    unsigned output_cnt=0;
    
    size_t output_length = (size_t) (4.0 * ceil((double) inlen / 3.0));
	if (output_length > BASE64_MAX_LEN)
		return;
    char *buf = (char*)malloc(output_length+1);
    if (buf == NULL)
		return;
    buf[output_length] = 0;
    
    while(input_cnt<inlen) {
        buff1[i++] = input[input_cnt++];
        if (i==3) {
            buf[output_cnt++] = encodeCharacterTable[(buff1[0] & 0xfc) >> 2];
            buf[output_cnt++] = encodeCharacterTable[((buff1[0] & 0x03) << 4) + ((buff1[1] & 0xf0) >> 4)];
            buf[output_cnt++] = encodeCharacterTable[((buff1[1] & 0x0f) << 2) + ((buff1[2] & 0xc0) >> 6)];
            buf[output_cnt++] = encodeCharacterTable[buff1[2] & 0x3f];
            i=0;
        }
    }
    if (i) {
        for(j=i; j<3; j++) {
            buff1[j] = '\0';
        }
        buff2[0] = (buff1[0] & 0xfc) >> 2;
        buff2[1] = ((buff1[0] & 0x03) << 4) + ((buff1[1] & 0xf0) >> 4);
        buff2[2] = ((buff1[1] & 0x0f) << 2) + ((buff1[2] & 0xc0) >> 6);
        buff2[3] = buff1[2] & 0x3f;
        for (j=0; j<(i+1); j++) {
            buf[output_cnt++] = encodeCharacterTable[(int)buff2[j]];
        }
        while(i++<3) {
            buf[output_cnt++] = '=';
        }
    }
	str = buf;
	free(buf);
}

Unbase64::Unbase64(const char *src)
:buf(NULL), len(0) {
	if (!src)
		return;
	size_t input_length = strlen(src);
	if (input_length > BASE64_MAX_LEN || input_length % 4 != 0)
		return;
		
	char buff1[4];
    char buff2[4];
    unsigned char i=0, j;
    unsigned input_cnt=0;
    unsigned output_cnt=0;
    
    len = input_length / 4 * 3;
    if (src[input_length - 1] == '=') (len)--;
    if (src[input_length - 2] == '=') (len)--;

    buf = (char*)malloc(len+1);
    buf[len] = 0;
    
    while(input_cnt<input_length) {
        buff2[i] = src[input_cnt++];
        if (buff2[i] == '=') {
            break;
        }
        if (++i==4) {
            for (i=0; i!=4; i++) {
                buff2[i] = decodeCharacterTable[(int)buff2[i]];
            }
            buf[output_cnt++] = (char)((buff2[0] << 2) + ((buff2[1] & 0x30) >> 4));
            buf[output_cnt++] = (char)(((buff2[1] & 0xf) << 4) + ((buff2[2] & 0x3c) >> 2));
            buf[output_cnt++] = (char)(((buff2[2] & 0x3) << 6) + buff2[3]);
            i=0;
        }
    }
    if (i) {
        for (j=i; j<4; j++) {
            buff2[j] = '\0';
        }
        for (j=0; j<4; j++) {
            buff2[j] = decodeCharacterTable[(int)buff2[j]];
        }
        buff1[0] = (buff2[0] << 2) + ((buff2[1] & 0x30) >> 4);
        buff1[1] = ((buff2[1] & 0xf) << 4) + ((buff2[2] & 0x3c) >> 2);
        buff1[2] = ((buff2[2] & 0x3) << 6) + buff2[3];
        for (j=0; j<(i-1); j++) {
            buf[output_cnt++] = (char)buff1[j];
        }
    }
}

Unbase64::~Unbase64() {
	if (buf)
		free(buf);
}

Autofree::Autofree(void* p, Freefunc func)
:_p(p), _freeFunc(func) {
    
}

Autofree::~Autofree() {
    free();
}

void Autofree::set(void* p, Freefunc func) {
    free();
    _p = p;
    _freeFunc = func;
}

void Autofree::free() {
    if (_p && _freeFunc) {
        _freeFunc(_p);
        _p = NULL;
    }
}