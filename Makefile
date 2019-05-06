obj-m += module/fiber.o

all:
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	sudo insmod module/fiber.ko
	gcc -pthread -o usertest_1 user/usertest_1.c

clean:
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	sudo rmmod fiber

remove:
	sudo rmmod fiber
	
insert:
	sudo insmod module/fiber.ko

test:
	./usertest_1
