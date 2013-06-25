import struct

# Port and Addresses:
FC_IP    = '127.0.0.1'
FC_LISTEN_PORT  = 36000

SENSOR_IP       = '127.0.0.1'
ADIS_TX_PORT    = 35020
MPU_TX_PORT     = 35002
MPL_TX_PORT     = 35010

ROLL_IP         = '127.0.0.1'
ROLL_TX_PORT    = 35004
ROLL_RX_PORT    = 35003


# Messages:
RC_Servo_Message = struct.Struct('>HB')
ADIS_Message = struct.Struct('<12H')
