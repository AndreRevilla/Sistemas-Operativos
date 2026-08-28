#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>

static sigjmp_buf punto_salto;
static sigjmp_buf salto_sigusr1;
static sigjmp_buf salto_sigusr2;

void handler_sigint(int signal)
{
    printf("\nSe recibió la señal %d SIGINT\n", signal);

    // Salto de ejecución al punto establecido con sigsetjmp()
    siglongjmp(punto_salto, 1);
}
void handler_sigusr1(int signal)
{
    printf("\nSe recibió la señal %d SIGUSR1\n", signal);

    // Salto de ejecución al punto establecido con sigsetjmp()
    siglongjmp(salto_sigusr1, 1);
}
void handler_sigusr2(int signal)
{
    printf("\nSe recibió la señal %d SIGUSR2\n", signal);

    // Salto de ejecución al punto establecido con sigsetjmp()
    siglongjmp(salto_sigusr2, 1);
}

int main(void)
{
    // Registrar el manejador para SIGINT (Ctrl+C)
    signal(SIGINT, handler_sigint);
    signal(SIGUSR1, handler_sigusr1);
    signal(SIGUSR2, handler_sigusr2);

    if (sigsetjmp(punto_salto, 1) == 0) {
        printf("Programa iniciado.\n");

        while (1) {
            printf("Ejecutando código normal...\n");
            sleep(2);
        }
    }   else {
        // La ejecución llega aquí después de recibir SIGINT
        printf("¡Se realizó el salto de código!\n");
        printf("Continuando desde el punto de recuperación...\n");
    }

    printf("Fin del programa.\n");

    return 0;
}
