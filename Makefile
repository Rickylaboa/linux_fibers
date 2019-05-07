EXTRA_CFLAGS := -I$(src)/module

obj-m += fiber.o
fiber-objs := module/fiber.o module/fiber_struct.o module/fiber_methods.o

all:
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -pthread -o usertest_1 user/usertest_1.c

clean:
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

remove:
	sudo rmmod fiber
	
insert:
	sudo insmod fiber.ko

test:
	./usertest_1
