#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>


/*
 * =========================================================
 * TP2 - Ejercicio 4
 * Menú de control de BluePill mediante UART
 *
 * Puerto:
 *      /dev/ttyUSB0
 *
 * Configuración:
 *      9600 8N1
 *
 * Comandos:
 *
 *      1 -> LED ON
 *      2 -> LED OFF
 *      3 -> ADC1 PA1
 *      4 -> Sensor temperatura
 *      0 -> Salir
 * =========================================================
 */


#define SERIAL_PORT "/dev/ttyUSB0"


/* Descriptor del puerto serie */
int fd;


/* Configuración original del puerto */
struct termios oldtty;


/* =========================================================
 * RESTAURAR PUERTO Y CERRAR PROGRAMA
 * ========================================================= */

void cerrar_programa(int sig)
{
    (void)sig;

    /*
     * Restaurar configuración original.
     */
    tcsetattr(fd, TCSANOW, &oldtty);


    /*
     * Cerrar puerto.
     */
    close(fd);


    printf("\nPuerto cerrado.\n");

    exit(0);
}


/* =========================================================
 * CONFIGURAR PUERTO SERIE
 * ========================================================= */

int configurar_puerto(void)
{
    struct termios tty;


    /*
     * Obtener configuración actual.
     */
    if (tcgetattr(fd, &oldtty) != 0)
    {
        perror("tcgetattr");
        return -1;
    }


    /*
     * Copiar configuración.
     */
    tty = oldtty;


    /*
     * Modo RAW:
     *
     * Sin procesamiento de caracteres.
     * Sin echo.
     */
    cfmakeraw(&tty);


    /*
     * Velocidad:
     *
     * 9600 baudios.
     */
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);


    /*
     * Habilitar recepción.
     */
    tty.c_cflag |= CLOCAL | CREAD;


    /*
     * 8 bits.
     */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;


    /*
     * Sin paridad.
     */
    tty.c_cflag &= ~PARENB;


    /*
     * 1 bit de stop.
     */
    tty.c_cflag &= ~CSTOPB;


    /*
     * Sin control de flujo por hardware.
     */
    tty.c_cflag &= ~CRTSCTS;


    /*
     * Aplicar configuración.
     */
    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        perror("tcsetattr");
        return -1;
    }


    /*
     * Limpiar datos que pudieran haber quedado
     * en el buffer de recepción.
     */
    tcflush(fd, TCIOFLUSH);


    return 0;
}


/* =========================================================
 * MOSTRAR MENÚ
 * ========================================================= */

void mostrar_menu(void)
{
    printf("\n");
    printf("================================\n");
    printf("       BLUEPILL - MENU UART\n");
    printf("================================\n");
    printf("\n");

    printf("1     LED ON\n");
    printf("2     LED OFF\n");
    printf("3     ADC1\n");
    printf("4     SENSOR DE TEMPERATURA\n");
    printf("0     SALIR\n");

    printf("\n");
    printf("Seleccione una opcion: ");

    fflush(stdout);
}


/* =========================================================
 * ENVIAR COMANDO
 * ========================================================= */

int enviar_comando(char comando)
{
    ssize_t enviados;


    /*
     * Importante:
     *
     * Enviamos solamente el carácter.
     *
     * NO enviamos '\n', porque la BluePill
     * interpreta cada carácter recibido como
     * un comando individual.
     */
    enviados = write(fd, &comando, 1);


    if (enviados != 1)
    {
        perror("write");
        return -1;
    }


    /*
     * Asegurar que los datos sean transmitidos.
     */
    tcdrain(fd);


    return 0;
}


/* =========================================================
 * RECIBIR RESPUESTA
 * ========================================================= */

void recibir_respuesta(void)
{
    char buffer[128];

    ssize_t cantidad;

    fd_set conjunto;

    struct timeval timeout;


    /*
     * Esperar hasta 1 segundo por la respuesta
     * de la BluePill.
     */
    FD_ZERO(&conjunto);
    FD_SET(fd, &conjunto);


    timeout.tv_sec = 1;
    timeout.tv_usec = 0;


    int resultado = select(fd + 1,
                           &conjunto,
                           NULL,
                           NULL,
                           &timeout);


    if (resultado < 0)
    {
        perror("select");
        return;
    }


    /*
     * Timeout.
     */
    if (resultado == 0)
    {
        printf("[Sin respuesta de la BluePill]\n");
        return;
    }


    /*
     * Hay datos disponibles.
     */
    cantidad = read(fd,
                    buffer,
                    sizeof(buffer) - 1);


    if (cantidad > 0)
    {
        buffer[cantidad] = '\0';

        printf("\nBluePill: %s", buffer);
    }
}


/* =========================================================
 * MAIN
 * ========================================================= */

int main(void)
{
    char opcion;


    /*
     * Ctrl+C:
     *
     * restaurar configuración y cerrar puerto.
     */
    signal(SIGINT, cerrar_programa);


    /* =====================================================
     * ABRIR PUERTO
     * ===================================================== */

    fd = open(SERIAL_PORT,
              O_RDWR | O_NOCTTY);


    if (fd < 0)
    {
        perror("No se pudo abrir el puerto serie");

        printf("\n");
        printf("Verifique que exista:\n");
        printf("    %s\n", SERIAL_PORT);

        printf("\n");
        printf("Puede comprobarlo con:\n");
        printf("    ls /dev/ttyUSB*\n");

        return 1;
    }


    /* =====================================================
     * CONFIGURAR UART
     * ===================================================== */

    if (configurar_puerto() != 0)
    {
        close(fd);
        return 1;
    }


    /*
     * Mensaje inicial.
     */
    printf("\n");
    printf("Puerto serie: %s\n", SERIAL_PORT);
    printf("Configuracion: 9600 8N1\n");
    printf("Puerto abierto correctamente.\n");


    /*
     * Esperar un poco por el mensaje inicial
     * de la BluePill.
     */
    recibir_respuesta();


    /* =====================================================
     * BUCLE DEL MENÚ
     * ===================================================== */

    while (1)
    {
        mostrar_menu();


        /*
         * Leer opción.
         *
         * Se utiliza getchar() para que el menú
         * sea simple y compatible con terminal.
         */
        opcion = getchar();


        /*
         * Limpiar el resto de la línea de entrada.
         */
        int c;

        while ((c = getchar()) != '\n' &&
               c != EOF);


        /* =================================================
         * SALIR
         * ================================================= */

        if (opcion == '0')
        {
            break;
        }


        /* =================================================
         * VALIDAR OPCIÓN
         * ================================================= */

        if (opcion != '1' &&
            opcion != '2' &&
            opcion != '3' &&
            opcion != '4')
        {
            printf("\nOpcion invalida.\n");
            continue;
        }


        /* =================================================
         * ENVIAR COMANDO
         * ================================================= */

        if (enviar_comando(opcion) != 0)
        {
            break;
        }


        /* =================================================
         * ESPERAR RESPUESTA
         * ================================================= */

        recibir_respuesta();
    }


    /* =====================================================
     * RESTAURAR CONFIGURACIÓN
     * ===================================================== */

    tcsetattr(fd, TCSANOW, &oldtty);


    /*
     * Cerrar puerto.
     */
    close(fd);


    printf("\nPuerto cerrado.\n");
    printf("Programa finalizado.\n");


    return 0;
}
