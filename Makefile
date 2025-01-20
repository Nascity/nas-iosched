obj-m		:= nas-iosched.o
KVERSION	:= $(shell uname -r)

all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules

clean:
	rm trace.*
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean

apply: all
	sudo insmod nas-iosched.ko
	sudo echo nas-iosched > /sys/block/sda/queue/scheduler
	cat /sys/block/sda/queue/scheduler

remove:
	sudo echo mq-deadline > /sys/block/sda/queue/scheduler
	cat /sys/block/sda/queue/scheduler

show:
	cat /sys/block/sda/queue/scheduler

test:
	# https://greencloud33.tistory.com/54
	sudo blktrace -d /dev/sda -o trace -w 300 &
	sudo fio --filename=/dev/sda --direct=1 --sync=1 --rw=randread --stonewall  --bs=4k --numjobs=16 --iodepth=16 --runtime=60 --time_based --group_reporting --name=bigt --size=10G
