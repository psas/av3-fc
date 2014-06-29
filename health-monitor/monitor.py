#!/usr/bin/env python
import psutil
import time
from psas_packet import messages
from psas_packet import network

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

    pack['CPU_User'] = float(cpu.user)
    pack['CPU_System'] = float(cpu.system)
    pack['CPU_Nice'] = float(cpu.nice)
    pack['CPU_IOWait'] = float(cpu.iowait)
    pack['CPU_IRQ'] = float(cpu.irq)
    pack['CPU_SoftIRQ'] = float(cpu.softirq)

    pack['RAM_Used'] = int(ram.used)
    pack['RAM_Buffer'] = int(ram.buffers)
    pack['RAM_Cached'] = int(ram.cached)

    pack['PID'] = pids

    pack['Disk_Used'] = int(disk_used)

    # Since last interval
    pack['Disk_Read'] = int(disk.read_bytes - last_disk.read_bytes)
    pack['Disk_Write'] = int(disk.write_bytes - last_disk.write_bytes)

    lo = nics.get('lo')
    last_lo = last_nics.get('lo')
    if lo is not None and last_lo is not None:
        pack['IO_lo_Bytes_Sent'] = int(lo.bytes_sent - last_lo.bytes_sent)
        pack['IO_lo_Bytes_Recv'] = int(lo.bytes_recv - last_lo.bytes_recv)
        pack['IO_lo_Packets_Sent'] = int(lo.packets_sent - last_lo.packets_sent)
        pack['IO_lo_Packets_Recv'] = int(lo.packets_recv - last_lo.packets_recv)

    eth0 = nics.get('eth0')
    last_eth0 = last_nics.get('eth0')
    if eth0 is not None and last_eth0 is not None:
        pack['IO_eth0_Bytes_Sent'] = int(eth0.bytes_sent - last_eth0.bytes_sent)
        pack['IO_eth0_Bytes_Recv'] = int(eth0.bytes_recv - last_eth0.bytes_recv)
        pack['IO_eth0_Packets_Sent'] = int(eth0.packets_sent - last_eth0.packets_sent)
        pack['IO_eth0_Packets_Recv'] = int(eth0.packets_recv - last_eth0.packets_recv)

    wlan0 = nics.get('wlan0')
    last_wlan0 = last_nics.get('wlan0')
    if wlan0 is not None and last_wlan0 is not None:
        pack['IO_wlan0_Bytes_Sent'] = int(wlan0.bytes_sent - last_wlan0.bytes_sent)
        pack['IO_wlan0_Bytes_Recv'] = int(wlan0.bytes_recv - last_wlan0.bytes_recv)
        pack['IO_wlan0_Packets_Sent'] = int(wlan0.packets_sent - last_wlan0.packets_sent)
        pack['IO_wlan0_Packets_Recv'] = int(wlan0.packets_recv - last_wlan0.packets_recv)

    # temp
    try:
        with open('/sys/bus/platform/drivers/coretemp/coretemp.0/temp2_input', 'r') as tempfile:
            temp = int(tempfile.read().strip())
            pack['Core_Temp'] = temp
    except:
        pass

    last_disk = disk
    last_nics = nics

    return pack


if __name__ == '__main__':
    time.sleep(rate)

    seq = 0
    with network.SendUDP('127.0.0.1', 36000, from_port=36201) as udp:
        while True:
            # Includes a blocking read so it should run at globally defined rate
            udp.send_seq_message(messages.FCFH, seq, read())
            seq = seq + 1
