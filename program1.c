#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define MAX_BUF 1024
#define MAXSIZE 1024

#define FIFO_PATH "/tmp/fifoLab"
#define KEY_QUEUE1 1234

/* Estructura para la Message Queue */
struct msgbuf
{
    long mtype;
    char mtext[MAXSIZE];
};

/* Descriptor del FIFO */
int fd;

/* Buffer para almacenar el mensaje leído del FIFO */
char buf[MAX_BUF];

/* Puntos de salto para las señales */
static sigjmp_buf punto_salto2;
static sigjmp_buf punto_salto10;

/* Manejador de Signal 2 */
void manejador2(int signo)
{
    printf("\n[Program 1] Se recibió la señal %d (Signal 2)\n", signo);
    siglongjmp(punto_salto2, 1);
}

/* Manejador de Signal 10 */
void manejador10(int signo)
{
    printf("\n[Program 1] Se recibió la señal %d (Signal 10)\n", signo);
    siglongjmp(punto_salto10, 1);
}

int main(void)
{
    int msqid;
    struct msgbuf mensaje;

    /* Registrar los manejadores */
    signal(SIGINT, manejador2);
    signal(SIGUSR1, manejador10);

    /* Crear el FIFO si no existe */
    mkfifo(FIFO_PATH, 0666);

    /* Crear/conectarse a la primera Message Queue */
    msqid = msgget(KEY_QUEUE1, IPC_CREAT | 0666);

    if (msqid < 0)
    {
        perror("msgget");
        exit(1);
    }

    printf("=== PROGRAM 1 INICIADO ===\n");
    printf("PID del proceso: %d\n", getpid());
    printf("Signal 2  -> Leer FIFO\n");
    printf("Signal 10 -> Enviar mensaje a Program 2\n");
    printf("FIFO: %s\n", FIFO_PATH);
    printf("Message Queue 1: %d\n\n", msqid);

    while (1)
    {
        /*
         * SIGNAL 2
         * Al recibir Signal 2, se ejecuta la lectura del FIFO.
         */
        if (sigsetjmp(punto_salto2, 1) == 0)
        {
            printf("Esperando Signal 2...\n");
        }
        else
        {
            fd = open(FIFO_PATH, O_RDONLY);

            if (fd < 0)
            {
                perror("open FIFO");
                continue;
            }

            int bytes = read(fd, buf, MAX_BUF - 1);

            if (bytes < 0)
            {
                perror("read FIFO");
                close(fd);
                continue;
            }

            buf[bytes] = '\0';

            printf("[Program 1] Datos leídos del FIFO: \"%s\"\n", buf);

            close(fd);
        }

        /*
         * SIGNAL 10
         * Al recibir Signal 10, se envía el mensaje leído
         * anteriormente a Program 2 usando tipo 3.
         */
        if (sigsetjmp(punto_salto10, 1) == 0)
        {
            printf("Esperando Signal 10...\n\n");
        }
        else
        {
            mensaje.mtype = 3;

            strcpy(mensaje.mtext, buf);

            if (msgsnd(msqid, &mensaje,
                       strlen(mensaje.mtext) + 1, 0) < 0)
            {
                perror("msgsnd");
                continue;
            }

            printf("[Program 1] Mensaje enviado a Program 2\n");
            printf("[Program 1] Tipo de mensaje: %ld\n", mensaje.mtype);
            printf("[Program 1] Contenido: \"%s\"\n\n",
                   mensaje.mtext);
        }
    }

    return 0;
}
