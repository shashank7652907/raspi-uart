cc = gcc
CFLAGS = -Wall

TARGET = uart_test.o
SRC = uart.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

