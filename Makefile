obj-m += module/fiber.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -pthread -o usertest_1 user/usertest_1.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	sudo rmmod fiber
	sudo dmesg -C

ins:
	sudo insmod module/fiber.ko

test:
	./usertest_1
