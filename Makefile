EXTRA_CFLAGS := -I$(src)/module

obj-m += fiber.o
fiber-objs := module/fiber.o module/fiber_struct.o module/fiber_methods.o module/fls.o module/debug.o module/proc.o
i=1

all:
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -O2 -Wno-attributes -pthread -o usertest_1 user/usertest_1.c
	gcc -pthread -Wno-attributes  -o usertest_2 user/usertest_2.c
	make -C benchmark/ all

clean:
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	make -C benchmark/ clean
	sudo dmesg -C

remove:
	sudo rmmod fiber
	
insert:
	sudo insmod fiber.ko

testpell5:	
	while true; do \
    	./benchmark/test 5  ; \
	done

testpell50:	
	while true; do \
    	./benchmark/test 50  ; \
	done

testpell100:	
	while true; do \
    	./benchmark/test 100  ; \
	done

testpell600:	
	while true; do \
    	./benchmark/test 600  ; \
	done

testpell:
	./benchmark/test 5
	./benchmark/test 10
	./benchmark/test 100
	./benchmark/test 1000

test1:
	./usertest_1
test2:	
	./usertest_2
