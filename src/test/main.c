#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include "general/utils.h"

int main() {

    char str[] = "113";
    int i = string_to_int(str, sizeof(str));
    printf("%d", i);

    return 0;
}