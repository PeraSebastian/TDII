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
     * Abrir puerto serie.
     *
     * O_RDWR  -> lectura y escritura
     * O_NOCTTY -> no usarlo como terminal de control
     * O_SYNC -> escritura sincronizada
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
     * Formato:
     *
     * 8 bits
     * Sin paridad
     * 1 bit de stop
     *
     * => 8N1
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
     * No utilizar control de módem.
     */
    tty.c_cflag |= CLOCAL;


    /*
     * Sin control de flujo por hardware.
     */
    tty.c_cflag &= ~CRTSCTS;


    /*
     * Modo RAW.
     *
     * Los datos recibidos se entregan directamente
     * al programa.
     */
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);


    /*
     * Deshabilitar control de flujo por software.
     */
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);


    /*
     * Deshabilitar procesamiento de salida.
     */
    tty.c_oflag &= ~OPOST;


    /*
     * Lectura bloqueante.
     *
     * VMIN = 1
     * El read() espera al menos un carácter.
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
     * Limpiar buffers de entrada y salida.
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

    int posicion = 0;

    char c;


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
     * Configurar UART.
     */
    fd = configurar_uart(argv[1]);

    if (fd < 0)
    {
        return 1;
    }


    /*
     * Cabecera del programa.
     */
    printf("\n");
    printf("========================================\n");
    printf("       ADC1 - STM32 + FreeRTOS\n");
    printf("========================================\n");

    printf("Puerto  : %s\n", argv[1]);

    printf("Baudrate: 115200\n");

    printf("Formato : 8N1\n");

    printf("----------------------------------------\n");

    printf("Esperando valores del ADC...\n");

    printf("Presione Ctrl+C para salir.\n");

    printf("----------------------------------------\n\n");


    /*
     * Bucle principal.
     *
     * Leer continuamente los datos enviados
     * por la STM32.
     */
    while (1)
    {
        /*
         * Leer un carácter.
         */
        int n = read(fd, &c, 1);


        if (n == 1)
        {
            /*
             * Si recibimos ENTER:
             *
             * Procesamos la línea completa.
             */
            if (c == '\n')
            {
                /*
                 * Terminar string.
                 */
                buffer[posicion] = '\0';


                /*
                 * Mostrar línea recibida.
                 */
                if (posicion > 0)
                {
                    printf("%s\n", buffer);

                    fflush(stdout);
                }


                /*
                 * Reiniciar buffer.
                 */
                posicion = 0;
            }

            /*
             * Ignorar '\r'.
             */
            else if (c == '\r')
            {
                continue;
            }

            /*
             * Guardar carácter.
             */
            else
            {
                /*
                 * Evitar desbordamiento.
                 */
                if (posicion < (int)(sizeof(buffer) - 1))
                {
                    buffer[posicion] = c;

                    posicion++;
                }
                else
                {
                    /*
                     * Buffer lleno.
                     *
                     * Reiniciar.
                     */
                    posicion = 0;
                }
            }
        }


        /*
         * Error de lectura.
         */
        else if (n < 0)
        {
            /*
             * EINTR significa que la lectura
             * fue interrumpida.
             */
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
