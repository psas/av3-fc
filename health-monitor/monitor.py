#!/usr/bin/env python
import psutil
import time

# setup
rate = 0.5  # 2 Hz

last_disk = psutil.disk_io_counters()
last_nics = psutil.net_io_counters(pernic=True)

def read():
    global last_disk, last_nics
    pack = {}
    cpu = psutil.cpu_times_percent(interval=rate)
    ram = psutil.virtual_memory()
    pids = len(psutil.pids())
    disk_used = psutil.disk_usage('/').used
    disk = psutil.disk_io_counters()
    nics = psutil.net_io_counters(pernic=True)

    pack['CPU_User'] = cpu.user
    pack['CPU_System'] = cpu.system
    pack['CPU_Nice'] = cpu.nice
    pack['CPU_IOWait'] = cpu.iowait
    pack['CPU_IRQ'] = cpu.irq
    pack['CPU_SoftIRQ'] = cpu.softirq

    pack['RAM_Used'] = ram.used
    pack['RAM_Buffer'] = ram.buffers
    pack['RAM_Cached'] = ram.cached

    pack['PID'] = pids

    pack['Disk_Used'] = disk_used

    # Since last interval
    pack['Disk_Read'] = disk.read_bytes - last_disk.read_bytes
    pack['Disk_Write'] = disk.write_bytes - last_disk.write_bytes

    lo = nics.get('lo')
    last_lo = last_nics.get('lo')
    if lo is not None and last_lo is not None:
        pack['IO_lo_Bytes_Sent'] = lo.bytes_sent - last_lo.bytes_sent
        pack['IO_lo_Bytes_Recv'] = lo.bytes_recv - last_lo.bytes_recv
        pack['IO_lo_Packets_Sent'] = lo.packets_sent - last_lo.packets_sent
        pack['IO_lo_Packets_Recv'] = lo.packets_recv - last_lo.packets_recv

    eth0 = nics.get('eth0')
    last_eth0 = last_nics.get('eth0')
    if eth0 is not None and last_eth0 is not None:
        pack['IO_eth0_Bytes_Sent'] = eth0.bytes_sent - last_eth0.bytes_sent
        pack['IO_eth0_Bytes_Recv'] = eth0.bytes_recv - last_eth0.bytes_recv
        pack['IO_eth0_Packets_Sent'] = eth0.packets_sent - last_eth0.packets_sent
        pack['IO_eth0_Packets_Recv'] = eth0.packets_recv - last_eth0.packets_recv

    wlan0 = nics.get('wlan0')
    last_wlan0 = last_nics.get('wlan0')
    if wlan0 is not None and last_wlan0 is not None:
        pack['IO_wlan0_Bytes_Sent'] = wlan0.bytes_sent - last_wlan0.bytes_sent
        pack['IO_wlan0_Bytes_Recv'] = wlan0.bytes_recv - last_wlan0.bytes_recv
        pack['IO_wlan0_Packets_Sent'] = wlan0.packets_sent - last_wlan0.packets_sent
        pack['IO_wlan0_Packets_Recv'] = wlan0.packets_recv - last_wlan0.packets_recv

    # temp
    try:
        with open('/sys/bus/platform/drivers/coretemp/coretemp.0/temp2_input', 'r') as tempfile:
            temp = int(tempfile.read().strip())
            pack['Core_Temp'] = temp
    except:
        pass

    print pack
    last_disk = disk
    last_nics = nics


if __name__ == '__main__':
    time.sleep(rate)
    while True:
        read()
