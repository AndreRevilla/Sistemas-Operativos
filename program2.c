#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define MAXSIZE 1024

#define KEY_QUEUE1 1234
#define KEY_QUEUE2 5678

/* Estructura para las Message Queues */
struct msgbuf
{
    long mtype;
    char mtext[MAXSIZE];
};

int main(void)
{
    int msqid1;
    int msqid2;

    struct msgbuf mensaje;

    /*
     * Conectarse a la primera Message Queue.
     * Esta es la cola utilizada por Program 1 -> Program 2.
     */
    msqid1 = msgget(KEY_QUEUE1, IPC_CREAT | 0666);

    if (msqid1 < 0)
    {
        perror("msgget queue 1");
        exit(1);
    }

    /*
     * Crear/conectarse a la segunda Message Queue.
     * Esta será utilizada por Program 2 -> Program 3.
     */
    msqid2 = msgget(KEY_QUEUE2, IPC_CREAT | 0666);

    if (msqid2 < 0)
    {
        perror("msgget queue 2");
        exit(1);
    }

    printf("=== PROGRAM 2 INICIADO ===\n");
    printf("PID del proceso: %d\n", getpid());
    printf("Message Queue 1: %d\n", msqid1);
    printf("Message Queue 2: %d\n\n", msqid2);

    while (1)
    {
        /*
         * Esperar un mensaje de tipo 3 proveniente
         * de Program 1.
         */
        if (msgrcv(msqid1, &mensaje, MAXSIZE, 3, 0) < 0)
        {
            perror("msgrcv");
            exit(1);
        }

        printf("[Program 2] Mensaje recibido de Program 1\n");
        printf("[Program 2] Tipo de mensaje: %ld\n",
               mensaje.mtype);
        printf("[Program 2] Contenido: \"%s\"\n",
               mensaje.mtext);

        /*
         * Cambiar el tipo del mensaje a 4 antes
         * de enviarlo a Program 3.
         */
        mensaje.mtype = 4;

        if (msgsnd(msqid2, &mensaje,
                   strlen(mensaje.mtext) + 1, 0) < 0)
        {
            perror("msgsnd");
            exit(1);
        }

        printf("[Program 2] Mensaje enviado a Program 3\n");
        printf("[Program 2] Tipo de mensaje: %ld\n\n",
               mensaje.mtype);
    }

    return 0;
}
