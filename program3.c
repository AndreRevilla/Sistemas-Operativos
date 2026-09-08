#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define MAXSIZE 1024

#define KEY_QUEUE2 5678

struct msgbuf
{
    long mtype;
    char mtext[MAXSIZE];
};

int main(void)
{
    int msqid;
    struct msgbuf mensaje;

    /*
     * Conectarse a la segunda Message Queue.
     */
    msqid = msgget(KEY_QUEUE2, IPC_CREAT | 0666);

    if (msqid < 0)
    {
        perror("msgget");
        exit(1);
    }

    printf("=== PROGRAM 3 INICIADO ===\n");
    printf("PID del proceso: %d\n", getpid());
    printf("Message Queue: %d\n", msqid);
    printf("Esperando mensajes de tipo 4...\n\n");

    while (1)
    {
        /*
         * Esperar específicamente mensajes de tipo 4.
         */
        if (msgrcv(msqid, &mensaje, MAXSIZE, 4, 0) < 0)
        {
            perror("msgrcv");
            exit(1);
        }

        printf("[Program 3] Mensaje recibido\n");
        printf("[Program 3] Tipo de mensaje: %ld\n",
               mensaje.mtype);
        printf("[Program 3] Mensaje mostrado: \"%s\"\n\n",
               mensaje.mtext);
    }

    return 0;
}
