import struct

packet_header = struct.Struct('>L')
tag_header = struct.Struct('>4sHHL')
