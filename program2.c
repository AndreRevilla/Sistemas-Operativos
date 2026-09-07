/*
 * PROGRAMA 2 - Laboratorio 03 (Sistemas Operativos)
 *
 * Comportamiento requerido:
 *   - Espera un mensaje de Programa 1 en la Cola de Mensajes #1, type = 3
 *   - Al recibirlo, lo reenvia a Programa 3 usando una Cola de Mensajes #2
 *     DISTINTA, con type = 4
 *
 * Ejecucion:  ./program2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MSGQ1_KEY   1234   /* Cola Programa1 -> Programa2 (type 3) */
#define MSGQ2_KEY   5678   /* Cola Programa2 -> Programa3 (type 4) */
#define MAXSIZE     128

struct msgbuf {
    long mtype;
    char mtext[MAXSIZE];
};

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

int main(void) {
    int msqid1 = msgget(MSGQ1_KEY, IPC_CREAT | 0666);
    if (msqid1 < 0) die("msgget (cola 1)");

    int msqid2 = msgget(MSGQ2_KEY, IPC_CREAT | 0666);
    if (msqid2 < 0) die("msgget (cola 2)");

    printf("=== PROGRAMA 2 INICIADO (PID %d) ===\n", getpid());
    printf("Esperando mensaje type = 3 en la cola %d...\n\n", msqid1);

    struct msgbuf rbuf;
    struct msgbuf sbuf;

    while (1) {
        /* El 3er parametro = 3 filtra especificamente el tipo de mensaje 3 */
        if (msgrcv(msqid1, &rbuf, MAXSIZE, 3, 0) < 0) {
            die("msgrcv");
        }
        printf("[Programa 2] Mensaje recibido -> type = 3, contenido: \"%s\"\n",
               rbuf.mtext);

        sbuf.mtype = 4;
        snprintf(sbuf.mtext, MAXSIZE, "%s", rbuf.mtext);

        if (msgsnd(msqid2, &sbuf, strlen(sbuf.mtext) + 1, 0) < 0) {
            die("msgsnd");
        }
        printf("[Programa 2] Mensaje reenviado -> type = 4, contenido: \"%s\"\n\n",
               sbuf.mtext);
    }

    return 0;
}
