#include <curl_response.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct response_string* response_string_init() {
    struct response_string* s = new struct response_string;
    if (s == NULL) {
        printf("malloc() failed\n");
        exit(EXIT_FAILURE);
    }

    s->len = 0;
    s->ptr = (char*) malloc(1);
    if (s->ptr == NULL) {
        printf("malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';

    return s;
}

void response_string_cleanup(struct response_string* s) {
    free(s->ptr);
    free(s);
}

size_t response_string_writefunc(void *ptr, size_t size, size_t nmemb, struct response_string *s) {
    size_t new_len = s->len + size * nmemb;
    s->ptr = (char*) realloc(s->ptr, new_len + 1);
    if (s->ptr == NULL) {
        printf("realloc() failed\n");
        return 0;
    }
    memcpy(s->ptr + s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size * nmemb;
}
