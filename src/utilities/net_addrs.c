#include "net_addrs.h"

 /* Redefinition of htons() and htonl() because using the ones in endian.h result in
  * error: braced-group within expression allowed only inside a function
  */

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define HTONS(n) __builtin_bswap16(n)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define htons(n) (n)
#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define HTONL(n) __builtin_bswap32(n)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define htonl(n) (n)
#endif

/** Set an IP address given by the four byte-parts */
#define IPv4(a,b,c,d) \
	HTONL(((uint32_t)((a) & 0xff) << 24) | \
	      ((uint32_t)((b) & 0xff) << 16) | \
	      ((uint32_t)((c) & 0xff) << 8)  | \
	       (uint32_t)((d) & 0xff) \
	     )
/* Returns a pointer to a filled anonymous struct sockaddr_in
 * suitable for direct use in bind() and connect()
 */
#define make_addr(addr, port) \
(const struct sockaddr *) &(const struct sockaddr_in){ \
	.sin_family = AF_INET, \
	.sin_port = HTONS((port)), \
	.sin_addr = { (addr) } \
}


#ifdef FCF_FC_NETWORK
#define RNH_RCI_PORT RCI_PORT
#define SENSOR_RCI_PORT RCI_PORT
#define GPS_RCI_PORT RCI_PORT
#define ROLL_RCI_PORT RCI_PORT

#else
#define RNH_RCI_PORT 37001
#define SENSOR_RCI_PORT 37002
#define GPS_RCI_PORT 37003
#define ROLL_RCI_PORT 37004
#endif

const struct sockaddr * RCI_ADDR = make_addr(FC_IP, RCI_PORT);
const struct sockaddr * RNH_RCI_ADDR = make_addr(RNH_IP, RNH_RCI_PORT);
const struct sockaddr * SENSOR_RCI_ADDR = make_addr(SENSOR_IP, SENSOR_RCI_PORT);
const struct sockaddr * GPS_RCI_ADDR = make_addr(GPS_IP, GPS_RCI_PORT);
const struct sockaddr * ROLL_RCI_ADDR = make_addr(ROLL_IP, ROLL_RCI_PORT);

const struct sockaddr * RNH_BATTERY_ADDR = make_addr(RNH_IP, RNH_BATTERY);
const struct sockaddr * RNH_PORT_ADDR = make_addr(RNH_IP, RNH_PORT);
const struct sockaddr * RNH_ALARM_ADDR = make_addr(RNH_IP, RNH_ALARM);
const struct sockaddr * RNH_UMBDET_ADDR = make_addr(RNH_IP, RNH_UMBDET);

const struct sockaddr * FC_ADDR = make_addr(FC_IP, FC_LISTEN_PORT);

const struct sockaddr * ADIS_ADDR = make_addr(SENSOR_IP, ADIS_PORT);
const struct sockaddr * MPU_ADDR = make_addr(SENSOR_IP, MPU_PORT);
const struct sockaddr * MPL_ADDR = make_addr(SENSOR_IP, MPL_PORT);

const struct sockaddr * ROLL_ADDR = make_addr(ROLL_IP, ROLL_PORT);
const struct sockaddr * TEATHER_ADDR = make_addr(ROLL_IP, TEATHER_PORT);

const struct sockaddr * WIFI_ADDR = make_addr(WIFI_IP, WIFI_PORT);

const struct sockaddr * ARM_ADDR = make_addr(ARM_IP, ARM_PORT);
const struct sockaddr * RC_SERVO_ENABLE_ADDR = make_addr(ARM_IP, RC_SERVO_ENABLE_PORT);

const struct sockaddr * GPS_COTS_ADDR = make_addr(GPS_IP, GPS_COTS);

