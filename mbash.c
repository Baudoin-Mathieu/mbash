// SAE S3 Système mbash IUT CHARLEMAGNE 2025
// BAUDOIN Mathieu
// EYER Nathan

// Pour compiler : gcc mbash.c -o mbash
// Pour executer : ./mbash

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc, char **argv)
{
    char cmd[1024];

    while(1) {
        fprintf(stderr, "$ ");

        fgets(cmd, sizeof(cmd), stdin);

        if(cmd[0] == '\0' || strcmp(cmd, "\n") == 0) {
            continue;
        }

        if(strcmp(cmd, "exit\n") == 0) {
            break;
        }

        printf("%s", cmd);
    };

    exit(EXIT_SUCCESS);
}