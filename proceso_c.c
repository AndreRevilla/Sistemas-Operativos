#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_BUF 1024

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "[Proceso C] Error: Se debe proporcionar el descriptor del pipe.\n");
        return 1;
    }

    /* Convertir el argumento de texto a entero (Patrón de codigo4.c) */
    int fd_pipe = atoi(argv[1]);
    char buffer[MAX_BUF];

    printf("=== PROCESO C INICIADO (PID: %d) ===\n", getpid());
    printf("[Proceso C] Descriptor de Pipe recibido vía argv: %d\n", fd_pipe);
    printf("[Proceso C] Esperando datos desde el Proceso A...\n\n");

    while (1) {
        /* Lectura bloqueante sobre la tubería */
        ssize_t n = read(fd_pipe, buffer, sizeof(buffer) - 1);

        if (n > 0) {
            buffer[n] = '\0';
            printf("[Proceso C - PID %d] Mensaje recibido vía Pipe: \"%s\"", getpid(), buffer);
        } else if (n == 0) {
            printf("[Proceso C] El Proceso A cerró el Pipe. Finalizando...\n");
            break;
        } else {
            perror("[Proceso C] Error en read()");
            break;
        }
    }

    close(fd_pipe);
    return 0;
}
