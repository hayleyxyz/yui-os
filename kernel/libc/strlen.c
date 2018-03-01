#include <strlen.h>

size_t strlen(char const *s)
{
    size_t i;

    i= 0;
    while (s[i]) {
        i+= 1;
    }

    return i;
}
