#ifndef CM_UTIL_H
#define CM_UTIL_H

#include <string>

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

#endif // CM_UTIL_H
