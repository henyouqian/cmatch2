#include "cm_util.h"
#include "lh_httpd.h"
#include <openssl/sha.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

namespace cm {

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

MemIO::MemIO(){
    p = p0 = NULL;
    capacity = 0;
    overflow = false;
}

MemIO::~MemIO(){
    
}

MemIO::MemIO(char* ptr, unsigned _capacity) {
    p = p0 = ptr;
    capacity = _capacity;
}

void MemIO::set(char* ptr, unsigned _capacity){
    p = p0 = ptr;
    capacity = _capacity;
}

unsigned MemIO::remain(){
    int remain = p0+capacity-p;
    remain = std::max(0, remain);
    return remain;
}

unsigned MemIO::length(){
    return p - p0;
}

void MemIO::write(const void *src, int size){
    assert(src && size > 0);
    if ( p + size > p0 + capacity ){
        overflow = true;
        return;
    }
    if ( src ){
        memcpy(p, src, size);
    }
    p += size;
}

void MemIO::writeChar(char v){
    write(&v, sizeof(v));
}

void MemIO::writeUchar(unsigned char v){
    write(&v, sizeof(v));
}

void MemIO::writeShort(short v){
    write(&v, sizeof(v));
}

void MemIO::writeUshort(unsigned short v){
    write(&v, sizeof(v));
}

void MemIO::writeInt(int v){
    write(&v, sizeof(v));
}

void MemIO::writeUint(unsigned int v){
    write(&v, sizeof(v));
}

void MemIO::writeInt64(int64_t v){
    write(&v, sizeof(v));
}

void MemIO::writeUint64(uint64_t v){
    write(&v, sizeof(v));
}

void MemIO::writeFloat(float f){
    int n;
    memcpy(&n, &f, 4);
    write(&n, sizeof(n));
}

void MemIO::writeString(const char *str){
    assert(str);
    int size = strlen(str)+1;
    write(str, size);
}

void MemIO::printf(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int n = vsnprintf(p, remain(), format, ap);
    va_end(ap);
    p += n+1;
    if (p >= p0 + capacity)
        overflow = true;
}

void MemIO::read(void* buf, unsigned int nbytes){
    assert(nbytes > 0 && buf);
    if ( p + nbytes > p0 + capacity ){
        return;
    }
    memcpy(buf, p, nbytes);
    p += nbytes;
}

void* MemIO::read(unsigned int nbytes){
    assert(nbytes > 0);
    if ( p + nbytes > p0 + capacity ){
        return NULL;
    }
    void *pp = p;
    p += nbytes;
    return pp;
}

char MemIO::readChar(){
    char* p = (char*)read(sizeof(char));
    return *p;
}

unsigned char MemIO::readUchar(){
    unsigned char* p = (unsigned char*)read(sizeof(unsigned char));
    return *p;
}

short MemIO::readShort(){
    short* p = (short*)read(sizeof(short));
    return *p;
}

unsigned short MemIO::readUshort(){
    unsigned short* p = (unsigned short*)read(sizeof(unsigned short));
    return *p;
}

int MemIO::readInt(){
    int* p = (int*)read(sizeof(int));
    return *p;
}

unsigned int MemIO::readUint(){
    unsigned int* p = (unsigned int*)read(sizeof(unsigned int));
    return *p;
}

int64_t MemIO::readInt64(){
    int64_t* p = (int64_t*)read(sizeof(int64_t));
    return *p;
}

uint64_t MemIO::readUint64(){
    uint64_t* p = (uint64_t*)read(sizeof(uint64_t));
    return *p;
}

float MemIO::readFloat(){
    float f;
    int n = readInt();
    memcpy(&f, &n, 4);
    return f;
}

const char* MemIO::readString(){
    int len = strlen(p)+1;
    char *p= (char*)read(len);
    return p;
}

#define _check_str_null     if (str == NULL) {      \
                                if (err)            \
                                    *err = 1;       \
                                return 0;           \
                            }

int32_t s2int32(const char* str, int* err) {
    _check_str_null
    char* pEnd = NULL;
    int32_t n = strtol(str, &pEnd, 0);
    if ((*str != '\0' && *pEnd == '\0')) {
        return n;
    } else {
        if (err)
            *err = 1;
        return 0;
    }
}

uint32_t s2uint32(const char* str, int* err) {
    _check_str_null
    char* pEnd = NULL;
    uint32_t n = strtoul(str, &pEnd, 0);
    if ((*str != '\0' && *pEnd == '\0')) {
        return n;
    } else {
        if (err)
            *err = 1;
        return 0;
    }
}

int64_t s2int64(const char* str, int* err) {
    _check_str_null
    char* pEnd = NULL;
    int64_t n = strtoll(str, &pEnd, 0);
    if ((*str != '\0' && *pEnd == '\0')) {
        return n;
    } else {
        if (err)
            *err = 1;
        return 0;
    }
}

uint64_t s2uint64(const char* str, int* err) {
    _check_str_null
    char* pEnd = NULL;
    uint64_t n = strtoull(str, &pEnd, 0);
    if ((*str != '\0' && *pEnd == '\0')) {
        return n;
    } else {
        if (err)
            *err = 1;
        return 0;
    }
}

float s2float(const char* str, int* err) {
    _check_str_null
    char* pEnd = NULL;
    float n = strtof(str, &pEnd);
    if ((*str != '\0' && *pEnd == '\0')) {
        return n;
    } else {
        if (err)
            *err = 1;
        return 0.f;
    }
}

double s2double(const char* str, int* err) {
    _check_str_null
    char* pEnd = NULL;
    double n = strtod(str, &pEnd);
    if ((*str != '\0' && *pEnd == '\0')) {
        return n;
    } else {
        if (err)
            *err = 1;
        return 0.0;
    }
}

}