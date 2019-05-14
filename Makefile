EXTRA_CFLAGS := -I$(src)/module

obj-m += fiber.o
fiber-objs := module/fiber.o module/fiber_struct.o module/fiber_methods.o module/fls.o 

all:
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -O2 -pthread -o usertest_1 user/usertest_1.c
	gcc -pthread -o usertest_2 user/usertest_2.c

clean:
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

remove:
	sudo rmmod fiber
	
insert:
	sudo insmod fiber.ko

test1:
	./usertest_1
test2:	
	./usertest_2
