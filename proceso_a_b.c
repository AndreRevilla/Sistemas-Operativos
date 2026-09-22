#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAXSIZE 1024
#define KEY_QUEUE 1234

/* Estructura para la Message Queue */
struct msgbuf {
    long mtype;
    char mtext[MAXSIZE];
};

/* Buffer para saltos no locales (Patrón de emisor.c) */
static sigjmp_buf env;

/* Manejadores de Señales */
void handler_sigint(int sig) {
    siglongjmp(env, 2); /* Retorna 2 para SIGINT */
}

void handler_sigtrap(int sig) {
    siglongjmp(env, 5); /* Retorna 5 para SIGTRAP */
}

int main(void)
{
    int pipefd[2];
    int msqid;

    /* Ignorar SIGPIPE para evitar que cierres de pipe tumben al Proceso A */
    signal(SIGPIPE, SIG_IGN);

    /* 1. Crear la Tubería (Pipe sin nombre) */
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }

    /* 2. Crear la Cola de Mensajes (Message Queue) */
    msqid = msgget(KEY_QUEUE, IPC_CREAT | 0666);
    if (msqid < 0) {
        perror("msgget");
        exit(1);
    }

    /* 3. Crear Proceso B */
    pid_t pid_b = fork();

    if (pid_b < 0) {
        perror("fork Proceso B");
        exit(1);
    }

    if (pid_b == 0) {
        /* ================= PROCESO B ================= */

        /* Crear el Clon de B */
        pid_t pid_clon = fork();

        if (pid_clon < 0) {
            perror("fork Clon de B");
            exit(1);
        }

        if (pid_clon == 0) {
            /* ================= CLON DE B ================= */
            /* Cierra el lado de escritura en el clon */
            close(pipefd[1]);

            /* Convertir descriptor fd[0] a texto (Patrón de codigo3.c) */
            char descriptor_str[20];
            snprintf(descriptor_str, sizeof(descriptor_str), "%d", pipefd[0]);

            char *args[] = {
                "./proceso_c",
                descriptor_str,
                NULL
            };

            /* Reemplazar imagen de memoria por Proceso C */
            execv("./proceso_c", args);

            perror("execv ./proceso_c falló (Asegúrate de compilar proceso_c primero)");
            exit(1);
        }

        /* --- Continuación del Proceso B Original --- */
        /* Cierra ambos extremos del Pipe ya que B no los utiliza */
        close(pipefd[0]);
        close(pipefd[1]);

        struct msgbuf rcv_msg;
        printf("=== PROCESO B INICIADO (PID: %d) ===\n", getpid());
        printf("[Proceso B] Esperando mensajes en la Cola IPC (msqid: %d)...\n\n", msqid);

        while (1) {
            /* Extracción bloqueante de la cola de mensajes */
            if (msgrcv(msqid, &rcv_msg, MAXSIZE, 0, 0) < 0) {
                perror("[Proceso B] msgrcv");
                exit(1);
            }
            printf("[Proceso B - PID %d] Mensaje recibido vía Message Queue: \"%s\"\n", getpid(), rcv_msg.mtext);
        }

        exit(0);
    }

    /* ================= PROCESO A (PADRE PRINCIPAL) ================= */
    /* Cierra el extremo de lectura del Pipe */
    close(pipefd[0]);

    /* Registrar capturadores para Señal 2 (SIGINT) y Señal 5 (SIGTRAP) */
    signal(SIGINT, handler_sigint);
    signal(SIGTRAP, handler_sigtrap);

    printf("==================================================\n");
    printf("=== PROCESO A INICIADO (PID: %d) ===\n", getpid());
    printf("Instrucciones para prueba desde otra terminal:\n");
    printf("  kill -2 %d   -> Enviar mensaje a Proceso B (Message Queue)\n", getpid());
    printf("  kill -5 %d   -> Enviar mensaje a Proceso C (Pipe)\n", getpid());
    printf("==================================================\n\n");

    struct msgbuf msg_envio;
    msg_envio.mtype = 1;
    char buffer_pipe[MAXSIZE];

    while (1) {
        /*
         * Guardar el punto de salto al inicio del ciclo
         * (Estructura tomada de emisor.c de tu segundo laboratorio)
         */
        int signal_code = sigsetjmp(env, 1);

        if (signal_code == 2) {
            /* Manejo de Señal 2 (SIGINT) */
            snprintf(msg_envio.mtext, MAXSIZE, "Mensaje desde Proceso A (PID: %d) por SIGINT (2)", getpid());
            
            if (msgsnd(msqid, &msg_envio, strlen(msg_envio.mtext) + 1, 0) < 0) {
                perror("[Proceso A] msgsnd");
            } else {
                printf("[Proceso A] Señal 2 recibida -> Mensaje enviado a la Cola (Proceso B)\n");
            }
        } 
        else if (signal_code == 5) {
            /* Manejo de Señal 5 (SIGTRAP) */
            snprintf(buffer_pipe, MAXSIZE, "Mensaje desde Proceso A (PID: %d) por SIGTRAP (5)\n", getpid());
            
            if (write(pipefd[1], buffer_pipe, strlen(buffer_pipe) + 1) < 0) {
                perror("[Proceso A] write pipe");
            } else {
                printf("[Proceso A] Señal 5 recibida -> Mensaje escrito en la Tubería (Proceso C)\n");
            }
        }

        /* Suspender proceso a la espera de la siguiente señal */
        pause();
    }

    return 0;
}
