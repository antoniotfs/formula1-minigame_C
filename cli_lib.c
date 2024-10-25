#include <stdio.h>
#include <string.h>
#include "cli_lib.h"

void process_args(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <comando> <nome>\n", argv[0]);
        return;
    }

    if (strcmp(argv[1], "--greet") == 0 && argc == 3) {
        printf("Olá, %s!\n", argv[2]);
    } else {
        printf("Comando inválido ou parâmetros insuficientes.\n");
    }
}