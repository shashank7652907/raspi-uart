#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>

static int uart_init(const char *dev)
{
    int fd = open(dev, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        close(fd);
        return -1;
    }

    // 115200 8N1, no flow control
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;   // 8-bit chars
    tty.c_cflag |= (CLOCAL | CREAD);              // ignore modem controls, enable read
    tty.c_cflag &= ~(PARENB | PARODD);            // no parity
    tty.c_cflag &= ~CSTOPB;                       // 1 stop bit
    tty.c_cflag &= ~CRTSCTS;                      // no HW flow control

    tty.c_iflag = 0;
    tty.c_oflag = 0;
    tty.c_lflag = 0; // raw mode

    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 5; // 0.5s timeout

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        close(fd);
        return -1;
    }

    return fd;
}

int main(void)
{
    const char *dev = "/dev/serial0";       // symlink to the active UART
    const char *msg = "Hello over UART from C\r\n";

    int fd = uart_init(dev);
    if (fd < 0) {
        return 1;
    }

    // Send one char at a time with small delay
    for (size_t i = 0; i < strlen(msg); i++) {
        ssize_t n = write(fd, &msg[i], 1);
        if (n != 1) {
            perror("write");
            break;
        }
        tcdrain(fd);         // wait until byte actually sent
        usleep(100 * 1000);  // 100 ms between chars
    }

    close(fd);
    return 0;
}

