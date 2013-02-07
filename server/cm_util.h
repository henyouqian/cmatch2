#ifndef CM_UTIL_H
#define CM_UTIL_H

#include <string>
#include <stdint.h>

namespace cm {

template<typename T>
void unuse(T& t) {}

struct Sha1 {
	Sha1(const void *src, unsigned len);
	char buf[16];
};

struct Base64 {
	Base64(const void *src, unsigned len);
	std::string str;
};

struct Unbase64 {
	Unbase64(const char *src);
	~Unbase64();
	char *buf;
	unsigned len;
};

//autofree
typedef void (*Freefunc) (void*);
class Autofree {
public:
    Autofree(void* p, Freefunc func);
    ~Autofree();
    void set(void* p, Freefunc func);
    void free();
    
private:
    void *_p;
    Freefunc _freeFunc;
};

struct MemIO{
    char* p0;
    char* p;
    unsigned capacity;
    bool overflow;
    
    MemIO();
    MemIO(char* ptr, unsigned capacity);
    ~MemIO();
    void set(char* ptr, unsigned capacity);
    unsigned remain();
    unsigned length();
    
    void            write(const void *src, int nbytes); //return writed data ptr, so you can set value later
    void            writeChar(char v);
    void            writeUchar(unsigned char v);
    void            writeShort(short v);
    void            writeUshort(unsigned short v);
    void            writeInt(int v);
    void            writeUint(unsigned int v);
    void            writeInt64(int64_t v);
    void            writeUint64(uint64_t v);
    void            writeFloat(float v);
    void            writeString(const char *str);
    void            printf(const char *fmt, ...);
    
    void            read(void* buf, unsigned int nbytes);
    void*           read(unsigned int nbytes);
    char            readChar();
    unsigned char   readUchar();
    short           readShort();
    unsigned short  readUshort();
    int             readInt();
    unsigned int    readUint();
    int64_t         readInt64();
    uint64_t        readUint64();
    float           readFloat();
    const char*     readString();
};

int32_t     s2int32(const char* str, int* err = NULL);
uint32_t    s2uint32(const char* str, int* err = NULL);
int64_t     s2int64(const char* str, int* err = NULL);
uint64_t    s2uint64(const char* str, int* err = NULL);
float       s2float(const char* str, int* err = NULL);
double      s2double(const char* str, int* err = NULL);


}

#endif // CM_UTIL_H
