#include <stdio.h>
#include <stdlib.h>

#include "server.h"

int main()
{
    printf("Starting\n");
    server_start(8080);

    return EXIT_SUCCESS;
}
