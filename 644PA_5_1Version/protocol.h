/*
 * Protocol and sensor definitions for weather station system
 * Alignment rules:
 *
 * Data type   Alignment (in bytes)
 * uint8_t     1
 * uint16_t    2
 * uint32_t    4
 * struct      4
 * 
 * $Id: protocol.h 601 2011-02-26 15:04:39Z nuumio $
 */

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <stdint.h>

/* USART SYNC BYTES. NOTE: Will be transferred as 0x78 0x56 0x34 0x12 because we use little endian systems. */
/*                         This needs to be considered if ported to bid endian system.                      */
#define USART_SYNC_BYTES 0x12345678

/* DATAGRAM DATA TYPES */
#define WS_DG_TYPE_ACK             0x00 /* General ack for notifications */
#define WS_DG_TYPE_SENSOR_DATA_NTF 0x01 /* Sensor data notification. Contains n sensor data blocks. */
#define WS_DG_TYPE_TIME_DATE_REQ   0x02 /* Time and date request */
#define WS_DG_TYPE_TIME_DATE_RSP   0x03 /* Time and date response */
#define WS_DG_TYPE_MIN_MAX_REQ     0x04 /* Time and date request */
#define WS_DG_TYPE_MIN_MAX_RSP     0x05 /* Time and date response */

/* DATAGRAM HEADER */
typedef struct {
	uint8_t datagram_type; /* See datagram types above */
	uint8_t msg_id;        /* Message id. ACK/RSP must have same msg_id as corresponding REQ/NTF */
	uint16_t data_len;     /* Data length including ending checksum */
} ws_datagram_header_t;

/* SENSOR DATA NOTIFICATION DATAGRAM DESCRIPTION:
 * Byte number  Content
 * [0..3]       Datagram header: ws_datagram_header_t
 * [4..7]       Sensor notify subpacket 1 header = always NODE_ID
 * [8+n*4]        Subpacket 1 data
 * [...]        Sensor notify subpacket 2 header
 * [...]          Subpacket 2 data
 * ...
 * 
 * Datagram ends with WS_SENSOR_NFT_NULL header (no data after header)
 */

/* Datagram data: After the header data is formed like this:
 * 2 bytes: sensor type (see Sensor types below, 0 means end of datagram)
 * n bytes: sensor data (length depends on sensor type)
 */

/* ACK datagram (Just header and checksum) */
typedef struct {
	ws_datagram_header_t header;
	uint8_t pad1;     /* Padding (to get 4 byte aligment) */
	uint8_t pad2;     /* Padding (to get 4 byte aligment) */
	uint16_t crc;
} ws_datagram_ack_t;

/* Time and date request datagram (Just header and checksum) */
typedef struct {
	ws_datagram_header_t header;
	uint8_t pad1;     /* Padding (to get 4 byte aligment) */
	uint8_t pad2;     /* Padding (to get 4 byte aligment) */
	uint16_t crc;
} ws_datagram_time_date_req_t;

/* Time and date response datagram */
typedef struct {
	ws_datagram_header_t header;
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t weekday;
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
	uint16_t milliseconds;
	uint16_t crc;
} ws_datagram_time_date_resp_t;

/* Time and date request datagram (Just header and checksum) */
typedef struct {
	ws_datagram_header_t header;
	uint8_t pad1;     /* Padding (to get 4 byte aligment) */
	uint8_t pad2;     /* Padding (to get 4 byte aligment) */
	uint16_t crc;
} ws_datagram_min_max_req_t;

typedef struct {
	uint16_t temp;
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t weekday;
	uint8_t hours;
	uint8_t minutes;
	uint8_t valid; /* true if values are valid; otherwise false */
	uint8_t pad2;
	uint8_t pad3;
} min_max_temp_t;

/* Time and date response datagram */
typedef struct {
	ws_datagram_header_t header;
	min_max_temp_t minmax_temp[8];
#if 0
	min_max_temp_t max_temp_out_24;
	min_max_temp_t min_temp_out_24;
	min_max_temp_t max_temp_in_24;
	min_max_temp_t min_temp_in_24;
	min_max_temp_t max_temp_out_12;
	min_max_temp_t min_temp_out_12;
	min_max_temp_t max_temp_in_12;
	min_max_temp_t min_temp_in_12;
#endif
	uint16_t pad1;
	uint16_t crc;
} ws_datagram_min_max_resp_t;

/* Node ids. NOTE: These are fixed until registration support is completed. */
#define WS_NODE_ID_MAIN_UNIT   0x0001
#define WS_NODE_ID_REMOTE_UNIT 0x0002

/* Sensor data notification header. */
typedef struct {
	uint16_t packet_id; /* Packet id */
	uint16_t data;      /* crc in case of ending WS_SENSOR_NFT_NULL header, node_id if WS_SENSOR_NTF_MODE_ID (otherwise it's padding) */
} ws_ntf_subheader_t;


/* SENSOR NOTIFY SUBPACKET IDS */
#define WS_SENSOR_NFT_NULL           0x0000 /* Means end of data */
#define WS_SENSOR_NTF_SHT1X          0x0001 /* Sensirion SHT1X temperature and humidity sensor */
#define WS_SENSOR_NTF_BMP085         0x0002 /* Bosch BMP085 digital barometric pressure and temperature sensor */
#define WS_SENSOR_NTF_MODE_ID        0xFFFF /* Node id */

/* SENSOR DATA STRUCTS */
/* Sensor data header. This is mandatory in the beginning of every sensor data struct. */
typedef struct {
	uint16_t sensor_id;  /* Sensor id within the node */
	uint8_t pad1;        /* Padding (to get 4 byte aligment) */
	uint8_t pad2;        /* Padding (to get 4 byte aligment) */
} ws_sensor_header_t;

/* Sensirion SHT1X temperature and humidity sensor                                                    */
/* See: http://www.sensirion.com/en/01_humidity_sensors/00_humidity_sensors.htm                       */
/* datasheet: http://www.sensirion.com/en/pdf/product_information/Datasheet-humidity-sensor-SHT1x.pdf */
typedef struct {
	ws_sensor_header_t header;
	uint16_t temperature;
	uint16_t humidity;
	uint8_t resolution_setting; /* Needed to determine correct coefficients for conversion */
	                            /* 0 = 14/12 bits, 1 = 12/8 bits (temperature/humidity)    */
	uint8_t pad1;               /* Padding (to get 4 byte aligment) */
	uint8_t pad2;               /* Padding (to get 4 byte aligment) */
	uint8_t pad3;               /* Padding (to get 4 byte aligment) */
} ws_sensor_sht1x_t;

/* Bosch BMP085 digital barometric pressure and temperature sensor                                      */
/* See: http://www.bosch-sensortec.com/content/language1/html/3477.htm                                  */
/* API: http://www.bosch-sensortec.com/content/language1/downloads/API_BMP085.zip                       */
/* Datasheet: http://www.bosch-sensortec.com/content/language1/downloads/BST-BMP085-DS000-05.pdf        */
/* Note: Calibration data is per sensor. Protocol could be optimized to send it only once (per sensor). */
typedef struct {
	ws_sensor_header_t header;
	int16_t ac1;                /* Calibration data */
	int16_t ac2;                /* Calibration data */
	int16_t ac3;                /* Calibration data */
	uint16_t ac4;               /* Calibration data */
	uint16_t ac5;               /* Calibration data */
	uint16_t ac6;               /* Calibration data */
	int16_t b1;                 /* Calibration data */
	int16_t b2;                 /* Calibration data */
	int16_t mb;                 /* Calibration data */
	int16_t mc;                 /* Calibration data */
	int16_t md;                 /* Calibration data */
	uint16_t temperature;       /* 16 bits */
	uint32_t pressure;          /* 16 to 19 bits */
	uint8_t oversampling;       /* Needed for correct conversion (See BMP085 datasheet) */
	                            /* Valid values are [0...3]                             */
	uint8_t pad1;               /* Padding to round size to 4 bytes */
	uint8_t pad2;               /* Padding to round size to 4 bytes */
	uint8_t pad3;               /* Padding to round size to 4 bytes */
} ws_sensor_bmp085_t;


#endif
