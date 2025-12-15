#ifndef PICO_BTSTACK_CONFIG_H
#define PICO_BTSTACK_CONFIG_H

// ----------------------
// BTstack features
// ----------------------
#define ENABLE_HID_HOST          // Classic HID Host (Wiimote, mice, keyboards, etc.)
#define ENABLE_LOG_INFO
#define ENABLE_LOG_ERROR
#define ENABLE_PRINTF_HEXDUMP

// ----------------------
// HCI / controller buffers
// ----------------------

// Required by pico-sdk's CYW43 HCI transport:
#define HCI_OUTGOING_PRE_BUFFER_SIZE 4

// ACL payload size
// 69 = 1 (HIDP header) + 4 (L2CAP) + 64 (report)
#define HCI_ACL_PAYLOAD_SIZE        (169)
#define HCI_ACL_CHUNK_SIZE_ALIGNMENT 4

// Max connections
#define MAX_NR_HCI_CONNECTIONS  1

// Limit number of ACL/SCO buffers to avoid CYW43 bus overrun
#define MAX_NR_CONTROLLER_ACL_BUFFERS  3
#define MAX_NR_CONTROLLER_SCO_PACKETS  0

// Enable and configure HCI Controller-to-Host flow control
#define ENABLE_HCI_CONTROLLER_TO_HOST_FLOW_CONTROL
#define HCI_HOST_ACL_PACKET_LEN  HCI_ACL_PAYLOAD_SIZE
#define HCI_HOST_ACL_PACKET_NUM  3
#define HCI_HOST_SCO_PACKET_LEN  0
#define HCI_HOST_SCO_PACKET_NUM  0

// ----------------------
// Classic stack objects
// ----------------------

// L2CAP: HID needs 2 L2CAP channels (control + interrupt) per device
#define MAX_NR_L2CAP_CHANNELS   4
#define MAX_NR_L2CAP_SERVICES   3

// HID Host connections
#define MAX_NR_HID_HOST_CONNECTIONS  1
#define MAX_NR_HIDS_CLIENTS          0  

// SDP / RFCOMM / other profiles
#define MAX_NR_AVDTP_CONNECTIONS     0
#define MAX_NR_AVDTP_STREAM_ENDPOINTS 0
#define MAX_NR_AVRCP_CONNECTIONS     0
#define MAX_NR_BNEP_CHANNELS         0
#define MAX_NR_BNEP_SERVICES         0
#define MAX_NR_RFCOMM_CHANNELS       0
#define MAX_NR_RFCOMM_MULTIPLEXERS   0
#define MAX_NR_RFCOMM_SERVICES       0

// SDP service records
#define MAX_NR_SERVICE_RECORD_ITEMS  4

// Security manager / LE DB
#define MAX_NR_SM_LOOKUP_ENTRIES     3
#define MAX_NR_WHITELIST_ENTRIES     16
#define MAX_NR_LE_DEVICE_DB_ENTRIES  16
#define MAX_NR_BTSTACK_LINK_KEY_DB_MEMORY_ENTRIES  3

// NVM: link keys + LE device DB entries stored in flash via TLV
#define NVM_NUM_DEVICE_DB_ENTRIES    16
#define NVM_NUM_LINK_KEYS            16

// ----------------------
// BTstack HAL / Pico SDK integration
// ----------------------
#define HAVE_EMBEDDED_TIME_MS        // use Pico time functions
#define HAVE_ASSERT                  // map btstack_assert to Pico assert()

// Some chipsets need this, safe to keep for CYW43
#define HCI_RESET_RESEND_TIMEOUT_MS  1000

#define ENABLE_SOFTWARE_AES128
#define ENABLE_MICRO_ECC_FOR_LE_SECURE_CONNECTIONS

#ifdef ENABLE_CLASSIC
// #define ENABLE_L2CAP_ENHANCED_RETRANSMISSION_MODE
#endif

#endif // PICO_BTSTACK_CONFIG_H
