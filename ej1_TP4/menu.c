#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>


/* ============================================================
 * CONFIGURACION
 * ============================================================ */

#define BAUDRATE B115200


/* ============================================================
 * CONFIGURAR PUERTO SERIE
 * ============================================================ */

int configurar_uart(const char *puerto)
{
    int fd;

    struct termios tty;


    /*
     * Abrir puerto serie
     *
     * O_RDWR  -> lectura y escritura
     * O_NOCTTY -> no convertirlo en terminal de control
     * O_SYNC -> escritura sin buffering excesivo
     */
    fd = open(puerto, O_RDWR | O_NOCTTY | O_SYNC);

    if (fd < 0)
    {
        perror("Error al abrir el puerto serie");
        return -1;
    }


    /*
     * Obtener configuración actual.
     */
    if (tcgetattr(fd, &tty) != 0)
    {
        perror("Error en tcgetattr");

        close(fd);

        return -1;
    }


    /*
     * Velocidad:
     *
     * 115200 baudios
     */
    cfsetospeed(&tty, BAUDRATE);
    cfsetispeed(&tty, BAUDRATE);


    /*
     * Configuración 8N1:
     *
     * 8 bits
     * Sin paridad
     * 1 bit de stop
     */
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;


    /*
     * Habilitar receptor.
     */
    tty.c_cflag |= CREAD;


    /*
     * Ignorar control de módem.
     */
    tty.c_cflag |= CLOCAL;


    /*
     * Sin control de flujo por hardware.
     */
    tty.c_cflag &= ~CRTSCTS;


    /*
     * Modo raw:
     *
     * No interpretar caracteres especiales.
     */
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);


    /*
     * Deshabilitar procesamiento de caracteres.
     */
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);


    /*
     * Salida sin procesamiento.
     */
    tty.c_oflag &= ~OPOST;


    /*
     * Tiempo de lectura.
     *
     * VMIN = 1
     * Esperar al menos un carácter.
     */
    tty.c_cc[VMIN] = 1;

    tty.c_cc[VTIME] = 0;


    /*
     * Aplicar configuración.
     */
    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        perror("Error en tcsetattr");

        close(fd);

        return -1;
    }


    /*
     * Limpiar buffers.
     */
    tcflush(fd, TCIOFLUSH);


    return fd;
}


/* ============================================================
 * MAIN
 * ============================================================ */

int main(int argc, char *argv[])
{
    int fd;

    char buffer[128];

    int n;


    /*
     * Verificar argumento.
     *
     * Ejemplo:
     *
     * ./menu /dev/ttyUSB0
     */
    if (argc != 2)
    {
        printf("Uso: %s <puerto-serie>\n", argv[0]);

        printf("\nEjemplos:\n");
        printf("  %s /dev/ttyUSB0\n", argv[0]);
        printf("  %s /dev/ttyACM0\n", argv[0]);

        return 1;
    }


    /*
     * Abrir y configurar UART.
     */
    fd = configurar_uart(argv[1]);

    if (fd < 0)
    {
        return 1;
    }


    printf("========================================\n");
    printf("   TELEMETRIA STM32 - FreeRTOS\n");
    printf("========================================\n");

    printf("Puerto : %s\n", argv[1]);
    printf("Baudrate: 115200\n");
    printf("Formato: 8N1\n");

    printf("----------------------------------------\n");
    printf("Esperando mensajes de la BluePill...\n");
    printf("Presione Ctrl+C para salir.\n");
    printf("----------------------------------------\n\n");


    /*
     * Bucle principal.
     *
     * Esperar datos enviados por la STM32.
     */
    while (1)
    {
        /*
         * Leer datos del puerto.
         */
        n = read(fd, buffer, sizeof(buffer) - 1);


        if (n > 0)
        {
            /*
             * Agregar terminador de string.
             */
            buffer[n] = '\0';


            /*
             * Mostrar mensaje recibido.
             */
            printf("%s", buffer);

            fflush(stdout);
        }


        /*
         * Error de lectura.
         */
        else if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            perror("Error leyendo puerto serie");

            break;
        }
    }


    /*
     * Cerrar puerto.
     */
    close(fd);


    return 0;
}
