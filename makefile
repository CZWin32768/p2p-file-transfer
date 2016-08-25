src = p2p-file-transfer.c
client : $(src)
	gcc -o test $(src) -g -lpthread -lm -ldl -w `pkg-config gtk+-2.0 --cflags --libs gtk+-2.0 gthread-2.0`
