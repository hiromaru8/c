CC = D:/tools/w64devkit/bin/gcc
CFLAGS = -O3 -Wall -Iinclude
LDFLAGS = -shared

TARGET = tiny_aes.dll
SRC = src/aes.c src/aes_256_ctr.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o
