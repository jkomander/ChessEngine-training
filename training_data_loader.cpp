#include<iostream>

#if defined (__x86_64__)
#define EXPORT
#define CDECL
#else
#if defined (_MSC_VER)
#define EXPORT __declspec(dllexport)
#define CDECL __cdecl
#else
#define EXPORT
#define CDECL __attribute__ ((__cdecl__))
#endif
#endif

struct Test {
    int32_t i = 42;
    char c = 'j';
};

extern "C" {

    EXPORT Test* CDECL create_test() {
        return new Test;
    }

    EXPORT void CDECL destroy_test(Test* test) {
        delete test;
    }

} // extern "C"