#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>

#define MAXSIZE 128

/* Buffer para saltos no locales */
static sigjmp_buf env;

/* Estructura para la cola de mensajes */
struct msgbuf
{
    long mtype;
    char mtext[MAXSIZE];
};

void die(char *s)
{
    perror(s);
    exit(1);
}

/* Manejadores de señales (Signal Handlers) */
void handle_sig1(int sig) {
    siglongjmp(env, 1);
}

void handle_sig2(int sig) {
    siglongjmp(env, 2);
}

void handle_sig3(int sig) {
    siglongjmp(env, 3);
}

int main(void)
{
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key = 1234;
    struct msgbuf sbuf;
    size_t buflen;

    /* Registrar manejadores de señales: SIGUSR1 (1), SIGUSR2 (2), SIGALRM (3) */
    signal(SIGUSR1, handle_sig1);
    signal(SIGUSR2, handle_sig2);
    signal(SIGALRM, handle_sig3);

    if ((msqid = msgget(key, msgflg)) < 0)
        die("msgget");

    printf("=== PROCESO EMISOR INICIADO ===\n");
    printf("PID del proceso emisor: %d\n", getpid());
    printf("Esperando senales:\n");
    printf("  - kill -SIGUSR1 %d  -> Mensaje Tipo 1\n", getpid());
    printf("  - kill -SIGUSR2 %d  -> Mensaje Tipo 2\n", getpid());
    printf("  - kill -SIGALRM %d  -> Mensaje Tipo 3\n\n", getpid());

    while (1) {
        /* Punto de retorno para salto no local */
        int signal_code = sigsetjmp(env, 1);

        if (signal_code != 0) {
            /* Asignar el tipo de mensaje segun la senal recibida */
            sbuf.mtype = signal_code;
            snprintf(sbuf.mtext, MAXSIZE, "Mensaje de Tipo %d enviado desde PID %d", signal_code, getpid());
            buflen = strlen(sbuf.mtext) + 1;

            if (msgsnd(msqid, &sbuf, buflen, IPC_NOWAIT) < 0) {
                die("msgsnd");
            } else {
                printf("[Emisor] Senal recibida (%d) -> Mensaje Tipo %d enviado a la Message Queue\n", signal_code, signal_code);
            }
        }

        /* Suspende el proceso hasta recibir la siguiente senal */
        pause();
    }

    return 0;
}
