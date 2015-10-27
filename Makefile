
all:
	gcc -o sntp_server_x86 sntp_server.c

CC = arm-linux-androideabi-gcc --sysroot=${CROSS_SYSROOT}

arm:
	$(CC) -static -o sntp_server_arm sntp_server.c

