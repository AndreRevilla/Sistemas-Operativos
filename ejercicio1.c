#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>

static sigjmp_buf salto_sigint;
static sigjmp_buf salto_sigusr1;
static sigjmp_buf salto_sigusr2;

void handler_sigint(int)
{
    printf("\n[ SIGINT recibida ]\n\n");
    printf(">>> Ejecutando salto no local #1 \n");
    siglongjmp(salto_sigint, 1);
}
void handler_sigusr1(int)
{
    printf("\n[ SIGUSR1 recibida ]\n\n");
    printf(">>> Ejecutando salto no local #2 \n");
    siglongjmp(salto_sigusr1, 1);
}
void handler_sigusr2(int)
{
    printf("\n[ SIGUSR2 recibida ]\n\n");
    printf(">>> Ejecutando salto no local #3 \n");
    siglongjmp(salto_sigusr2, 1);
}

int main(void)
{
    // Registrar el manejador para SIGINT (Ctrl+C)
    signal(SIGINT, handler_sigint);
    signal(SIGUSR1, handler_sigusr1);
    signal(SIGUSR2, handler_sigusr2);

    printf("PID del proceso: %d \n", getpid());
    printf("Programa iniciado.\n");

    while(1){

    if (sigsetjmp(salto_sigint, 1) != 0) {
        printf(">>> Recuperacion en punto SIGINT \n\n");
        printf("Programa continua ejecutandose desde el punto de recuperación...\n\n");
    }

    if (sigsetjmp(salto_sigusr1, 1) != 0) {
        printf(">>> Recuperacion en punto SIGUSR1 \n\n");
        printf("Programa continua ejecutandose desde el punto de recuperación...\n\n");
    }

    if (sigsetjmp(salto_sigusr2, 1) != 0) {
        printf(">>> Recuperacion en punto SIGUSR2 \n\n");
        printf("Programa continua ejecutandose desde el punto de recuperación...\n\n");
    }

    printf("Esperando señales...\n");
    sleep(2);

    }

    printf("Fin del programa.\n");

    return 0;
}
