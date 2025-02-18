obj-m		:= nas-iosched.o
KVERSION	:= $(shell uname -r)

all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
	rm trace.*

show:
	cat /sys/block/sda/queue/scheduler

blktrace:
	sudo blktrace -d /dev/sda -o trace -w 80

readtest:
	# https://greencloud33.tistory.com/54
	sudo fio --filename=/dev/sda --direct=1 --sync=1 --rw=randread --stonewall  --bs=4k --numjobs=16 --iodepth=16 --runtime=60 --time_based --group_reporting --name=bigt --size=10G
	
writetest:
	# https://greencloud33.tistory.com/54
	sudo fio --filename=/dev/sda --direct=1 --sync=1 --rw=randwrite --stonewall  --bs=4k --numjobs=16 --iodepth=16 --runtime=60 --time_based --group_reporting --name=bigt --size=10G

realblktrace:
	sudo blktrace -d /dev/sdb -o trace -w 80

realtest:
	# https://greencloud33.tistory.com/54
	sudo fio --filename=/dev/sdb --direct=1 --sync=1 --rw=randread --stonewall  --bs=4k --numjobs=16 --iodepth=16 --runtime=60 --time_based --group_reporting --name=bigt --size=10G
