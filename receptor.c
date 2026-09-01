#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXSIZE 128

void die(char *s)
{
    perror(s);
    exit(1);
}

struct msgbuf
{
    long mtype;
    char mtext[MAXSIZE];
};

int main(void)
{
    int msqid;
    key_t key = 1234;
    struct msgbuf rcvbuffer;

    /* Crear o conectarse a la cola de mensajes */
    if ((msqid = msgget(key, IPC_CREAT | 0666)) < 0)
        die("msgget()");

    printf("=== PROCESO RECEPTOR (CONSUMIDOR) INICIADO ===\n");
    printf("Esperando mensajes en la cola IPC (msqid: %d)...\n\n", msqid);

    /* Loop infinito para procesar mensajes de forma continua */
    while (1) {
        /*
         * El 4to parametro en '0' permite extraer cualquier tipo de mensaje (1, 2 o 3)
         * en el orden estricto en que fueron llegando a la cola (FIFO).
         * La llamada es bloqueante hasta que llegue un mensaje.
         */
        if (msgrcv(msqid, &rcvbuffer, MAXSIZE, 0, 0) < 0) {
            die("msgrcv");
        }

        printf("[Receptor] Mensaje extraido -> Tipo: %ld | Contenido: \"%s\"\n", rcvbuffer.mtype, rcvbuffer.mtext);
        printf("[Receptor] Procesando... (esperando 5 segundos)\n\n");
        
        sleep(5);
    }

    return 0;
}
