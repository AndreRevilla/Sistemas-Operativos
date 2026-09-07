/*
 * PROGRAMA 1 - Laboratorio 03 (Sistemas Operativos)
 *
 * Comportamiento requerido:
 *   - Senal 2  (SIGINT)  -> lee datos del FIFO /tmp/fifoLab
 *   - Senal 10 (SIGUSR1) -> envia un mensaje (type = 3) a Programa 2
 *                            usando una cola de mensajes System V
 *
 * Ejecucion:  ./program1
 * Para probar desde otra terminal:
 *   kill -SIGINT  <PID>
 *   kill -SIGUSR1 <PID>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define FIFO_PATH   "/tmp/fifoLab"
#define MSGQ1_KEY   1234   /* Cola Programa1 -> Programa2 */
#define MAXSIZE     128

struct msgbuf {
    long mtype;
    char mtext[MAXSIZE];
};

/* Flags que los manejadores activan; el main loop los procesa */
static volatile sig_atomic_t flag_fifo = 0;
static volatile sig_atomic_t flag_send = 0;

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

static void handler_signal2(int sig) {
    (void) sig;
    printf("\n[Programa 1] Senal 2 (SIGINT) recibida\n");
    flag_fifo = 1;
}

static void handler_signal10(int sig) {
    (void) sig;
    printf("\n[Programa 1] Senal 10 (SIGUSR1) recibida\n");
    flag_send = 1;
}

static void leer_fifo(void) {
    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd < 0) {
        perror("[Programa 1] open FIFO");
        return;
    }

    char buf[MAXSIZE] = {0};
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (n > 0) {
        buf[n] = '\0';
        printf("[Programa 1] Dato leido del FIFO: \"%s\"\n", buf);
    } else {
        printf("[Programa 1] FIFO vacio o cerrado sin datos\n");
    }
}

static void enviar_mensaje(void) {
    int msqid = msgget(MSGQ1_KEY, IPC_CREAT | 0666);
    if (msqid < 0) die("msgget");

    struct msgbuf sbuf;
    sbuf.mtype = 3;
    snprintf(sbuf.mtext, MAXSIZE, "Mensaje de Programa 1 (PID %d)", getpid());

    if (msgsnd(msqid, &sbuf, strlen(sbuf.mtext) + 1, 0) < 0) {
        die("msgsnd");
    }

    printf("[Programa 1] Mensaje enviado -> type = 3, contenido: \"%s\"\n", sbuf.mtext);
}

int main(void) {
    /* Crea el FIFO si todavia no existe (ignora error si ya esta creado) */
    if (mkfifo(FIFO_PATH, 0666) < 0 && errno != EEXIST) {
        perror("mkfifo");
    }

    signal(SIGINT, handler_signal2);
    signal(SIGUSR1, handler_signal10);

    printf("=== PROGRAMA 1 INICIADO (PID %d) ===\n", getpid());
    printf("Esperando senales:\n");
    printf("  kill -SIGINT  %d   -> leer FIFO %s\n", getpid(), FIFO_PATH);
    printf("  kill -SIGUSR1 %d   -> enviar mensaje type 3 a Programa 2\n\n", getpid());

    while (1) {
        pause();

        if (flag_fifo) {
            flag_fifo = 0;
            leer_fifo();
        }

        if (flag_send) {
            flag_send = 0;
            enviar_mensaje();
        }
    }

    return 0;
}
