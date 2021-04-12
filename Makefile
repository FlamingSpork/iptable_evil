# generic IP tables
obj-m += ip_tables.o

# the individual tables
obj-m += iptable_filter.o
obj-m += iptable_mangle.o
obj-m += iptable_nat.o
obj-m += iptable_raw.o
obj-m += iptable_security.o
obj-m += iptable_evil.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
