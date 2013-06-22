#ifndef FCF_NETADDRS
#define FCF_NETADDRS


/*
	FLIGHT COMPUTER
*/
#ifdef FCF_FC_NETWORK
#define FC_IP "10.0.0.10"
#else
#define FC_IP "127.0.0.1"
#endif
#define FC_LISTEN_PORT 36000
#define FC_LISTEN_PORT_S "36000"


/*
	WIFI
*/
#ifdef FCF_FC_NETWORK
#define WIFI_IP "10.0.20.20" //should this broadcast? -nathan
#else
#define WIFI_IP "127.0.0.1"
#endif
#define WIFI_PORT 35001
#define WIFI_PORT_S "35001"


/*
	ADIS
*/
#define ADIS_IP "10.0.0.20"
#define ADIS_PORT 35001
#define ADIS_PORT_S "35001"


/*
	ROLL
*/
#define ROLL_IP "10.0.0.30"
#define ROLL_PORT 35003
#define ROLL_PORT_S "35003"

#endif /* FCF_NETADDRS */
