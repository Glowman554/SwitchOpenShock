#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

struct response_string {
    char* ptr;
    size_t len;
};

struct response_string* response_string_init();
void response_string_cleanup(struct response_string* s);
size_t response_string_writefunc(void *ptr, size_t size, size_t nmemb, struct response_string *s);


#ifdef __cplusplus
}
#endif