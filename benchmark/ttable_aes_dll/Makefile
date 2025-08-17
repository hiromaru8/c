CC = gcc
CFLAGS = -O3 -Wall -Iinclude
LDFLAGS = -shared

TARGET = ttable_aes_t.dll
SRC = src/aes_256_ctr.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o
