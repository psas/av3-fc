#!/usr/bin/env python
import psutil

# basic example
#CPU
cpu = psutil.cpu_times()
print "CPU: user:", cpu.user, ' sys:', cpu.system, ' nice:', cpu.nice, ' iowait:', cpu.iowait, ' irq:', cpu.irq, ' softirq:', cpu.softirq

# RAM
ram = psutil.virtual_memory()
print "RAM: used:", ram.used, ' buffer:', ram.buffers, ' chached:', ram.cached

# PIDs
print 'PIDs:', len(psutil.pids())

# Disk
print 'Disk used:', psutil.disk_usage('/').used

disk = psutil.disk_io_counters()
print "Total disk read:", disk.read_bytes, ' disk write:', disk.write_bytes

# Network
for nic, numbers in psutil.net_io_counters(pernic=True).items():
    print nic, 'sent:', numbers.bytes_sent, ' recv:', numbers.bytes_recv, ' packets sent:', numbers.packets_sent, ' packets recv:', numbers.packets_recv

# temp
try:
    with open('/sys/bus/platform/drivers/coretemp/coretemp.0/temp2_input', 'r') as tempfile:
        temp = int(tempfile.read().strip())
        print "Core temp:", temp
except:
    pass
