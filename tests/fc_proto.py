import struct

packet_header = struct.Struct('>L')
tag_header = struct.Struct('>4sHHL')

message_types = {
    # Crescent GPS
    'GPS\x01': struct.Struct("<BBHdddfffffHH H"),
    'GPS\x02': struct.Struct("<LLHHHH H"),
    'GPS\x4C': struct.Struct("<dH8x 81L 15L 12L H"),
    'GPS\x5E': struct.Struct("<10d LHHHhhh H"),
    'GPS\x5F': struct.Struct("<HxxL 10L 10L 10L H"),
    'GPS\x60': struct.Struct("<xxHd " + ''.join(("BBBB" for i in range(12))) + "12L 12d 12d H"),
    'GPS\x61': struct.Struct("<L 4H 3L 2H H"),
    'GPS\x62': struct.Struct("<" + ''.join(("HBBBBbB" for i in range(8))) + "BBxx H"),
    'GPS\x63': struct.Struct("<BBHd" + ''.join(("8B b 3B H 5h" for i in range(12))) + "hH H"),

    # Theo-IMU
    'ACCL': struct.Struct(">BLBHHH"),
    'GYRO': struct.Struct(">BLBHHHB"),
    'MAGN': struct.Struct(">BLBHHH"),
    'CACC': struct.Struct(">BLBHHH"),
}
