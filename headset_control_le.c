/*
 * Copyright 2016-2021, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

/** @file
 *
 * This file implement BTLE controls.
 * The GATT database is defined in this file.
 *
 */
#include "wiced.h"
#include "wiced_bt_dev.h"
#include "wiced_app.h"
#include "wiced_bt_uuid.h"
#include "wiced_bt_gatt.h"
#include "wiced_bt_gatt.h"
#include "wiced_bt_ble.h"
#include <wiced_bt_ota_firmware_upgrade.h>
#include "wiced_bt_trace.h"
#include "wiced_bt_cfg.h"
#include "wiced_timer.h"
#include "bt_hs_spk_control.h"
#include "wiced_memory.h"
#include "wiced_app_cfg.h"
#include "headset_nvram.h"
#include "headset_control_le.h"
#ifdef FASTPAIR_ENABLE
#include "wiced_bt_gfps.h"
#endif

/******************************************************
 *                     Constants
 ******************************************************/
#define LE_CONTROL_MAX_CONNECTIONS          20
#define LE_CONTROL_CONNECT_TIMEOUT          10

/* UUID value of the Hello Sensor Service */
#define UUID_HELLO_SERVICE                    0x23, 0x20, 0x56, 0x7c, 0x05, 0xcf, 0x6e, 0xb4, 0xc3, 0x41, 0x77, 0x28, 0x51, 0x82, 0x7e, 0x1b
/* UUID value of the Hello Sensor Characteristic, Value Notification */
#define UUID_HELLO_CHARACTERISTIC_NOTIFY      0x26, 0xf6, 0x69, 0x91, 0x68, 0xee, 0xc2, 0xbe, 0x44, 0x4d, 0xb9, 0x5c, 0x3f, 0x2d, 0xc3, 0x8a
/* UUID value of the Hello Sensor Characteristic, Configuration */
#define UUID_HELLO_CHARACTERISTIC_CONFIG      0x1a, 0x89, 0x07, 0x4a, 0x2f, 0x3b, 0x7e, 0xa6, 0x81, 0x44, 0x3f, 0xf9, 0xa8, 0xf2, 0x9b, 0x5e
/* UUID value of the Hello Sensor Characteristic, Configuration */
#define UUID_HELLO_CHARACTERISTIC_LONG_MSG    0x2a, 0x99, 0x17, 0x5a, 0x3f, 0x4b, 0x8e, 0xb6, 0x91, 0x54, 0x2f, 0x09, 0xb8, 0x02, 0xab, 0x6e

#ifdef FASTPAIR_ENABLE
/* MODEL-specific definitions */
#if defined(CYW20721B2) || defined(CYW43012C0)
#define FASTPAIR_MODEL_ID                   0x82DA6E
#else
#define FASTPAIR_MODEL_ID                   0xCE948F //0xB49236 //0x000107 //0x140A02 // 0xCE948F
#endif

#if (FASTPAIR_MODEL_ID == 0x82DA6E)
const uint8_t anti_spoofing_public_key[] =  { 0x95, 0xcf, 0xdb, 0xae, 0xc0, 0xef, 0xc5, 0x1f, 0x39, 0x0f, 0x2a, 0xe0, 0x16, 0x5a, 0x2b, 0x59,\
		                                      0x62, 0xb2, 0xfe, 0x82, 0xfa, 0xf0, 0xd4, 0x1e, 0xa3, 0x4f, 0x07, 0x7e, 0xf7, 0x3d, 0xc0, 0x44,\
		                                      0x3d, 0xd0, 0x38, 0xb2, 0x31, 0x5d, 0xc6, 0x45, 0x72, 0x8a, 0x08, 0x0e, 0xc7, 0x4f, 0xc7, 0x76,\
		                                      0xd1, 0x19, 0xed, 0x8b, 0x17, 0x50, 0xb3, 0xa6, 0x94, 0x2e, 0xc8, 0x6b, 0xbb, 0x02, 0xc7, 0x4d };

const uint8_t anti_spoofing_private_key[] = { 0x84, 0xee, 0x67, 0xc3, 0x67, 0xea, 0x57, 0x38, 0xa7, 0x7e, 0xe2, 0x4d, 0x68, 0xaa, 0x9c, 0xf0,\
                                              0xc7, 0x9f, 0xc8, 0x07, 0x7e, 0x4e, 0x20, 0x35, 0x4c, 0x15, 0x43, 0x4d, 0xb5, 0xd2, 0xd1, 0xc3 };

#elif (FASTPAIR_MODEL_ID == 0xCE948F)
const uint8_t anti_spoofing_public_key[] =  { 0x0e, 0xe2, 0xbf, 0xe7, 0x96, 0xc6, 0xe1, 0x13, 0xf6, 0x57, 0x4a, 0xa8, 0x8c, 0x3a, 0x1b, 0x9c,\
                                              0x67, 0x1e, 0x36, 0xdf, 0x62, 0x69, 0xd8, 0xe5, 0x07, 0xe6, 0x8a, 0x72, 0x66, 0x4c, 0x9c, 0x90,\
                                              0xfc, 0xff, 0x00, 0x4f, 0x0f, 0x95, 0xde, 0x63, 0xe1, 0xc0, 0xbb, 0xa0, 0x75, 0xb1, 0xd2, 0x76,\
                                              0xfd, 0xe9, 0x66, 0x25, 0x0d, 0x45, 0x43, 0x7d, 0x5b, 0xf9, 0xce, 0xc0, 0xeb, 0x11, 0x03, 0xbe };

const uint8_t anti_spoofing_private_key[] = { 0x71, 0x11, 0x42, 0xb5, 0xe4, 0xa0, 0x6c, 0xa2, 0x8b, 0x74, 0xd4, 0x87, 0x7d, 0xac, 0x15, 0xc5,\
                                              0x42, 0x38, 0x1d, 0xb7, 0xba, 0x21, 0x19, 0x60, 0x17, 0x67, 0xfc, 0xba, 0x67, 0x47, 0x44, 0xc6 };

#else
const uint8_t anti_spoofing_public_key[] =  "";
const uint8_t anti_spoofing_private_key[] = "";
#warning "No Anti-Spooging key"

#endif
#endif //FASTPAIR_ENABLE

/******************************************************
 *                     Structures
 ******************************************************/

/******************************************************
 *               Variables Definitions
 ******************************************************/

/******************************************************************************
 *                                GATT DATABASE
 ******************************************************************************/
/*
 * This is the GATT database for the Hello Sensor application.  It defines
 * services, characteristics and descriptors supported by the sensor.  Each
 * attribute in the database has a handle, (characteristic has two, one for
 * characteristic itself, another for the value).  The handles are used by
 * the peer to access attributes, and can be used locally by application for
 * example to retrieve data written by the peer.  Definition of characteristics
 * and descriptors has GATT Properties (read, write, notify...) but also has
 * permissions which identify if and how peer is allowed to read or write
 * into it.  All handles do not need to be sequential, but need to be in
 * ascending order.
 */
const uint8_t gatt_server_db[]=
{
    /* Declare mandatory GATT service */
    PRIMARY_SERVICE_UUID16( HANDLE_HSENS_GATT_SERVICE, UUID_SERVICE_GATT ),

    /* Declare mandatory GAP service. Device Name and Appearance are mandatory
     * characteristics of GAP service                                        */
    PRIMARY_SERVICE_UUID16( HANDLE_HSENS_GAP_SERVICE, UUID_SERVICE_GAP ),

        /* Declare mandatory GAP service characteristic: Dev Name */
        CHARACTERISTIC_UUID16( HANDLE_HSENS_GAP_SERVICE_CHAR_DEV_NAME, HANDLE_HSENS_GAP_SERVICE_CHAR_DEV_NAME_VAL,
                GATT_UUID_GAP_DEVICE_NAME, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE ),

        /* Declare mandatory GAP service characteristic: Appearance */
        CHARACTERISTIC_UUID16( HANDLE_HSENS_GAP_SERVICE_CHAR_DEV_APPEARANCE, HANDLE_HSENS_GAP_SERVICE_CHAR_DEV_APPEARANCE_VAL,
                GATT_UUID_GAP_ICON, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE ),

    /* Declare Device info service */
    PRIMARY_SERVICE_UUID16( HANDLE_HSENS_DEV_INFO_SERVICE, UUID_SERVCLASS_DEVICE_INFO ),

        /* Handle 0x4e: characteristic Manufacturer Name, handle 0x4f characteristic value */
        CHARACTERISTIC_UUID16( HANDLE_HSENS_DEV_INFO_SERVICE_CHAR_MFR_NAME, HANDLE_HSENS_DEV_INFO_SERVICE_CHAR_MFR_NAME_VAL,
                GATT_UUID_MANU_NAME, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE ),

        /* Handle 0x50: characteristic Model Number, handle 0x51 characteristic value */
        CHARACTERISTIC_UUID16( HANDLE_HSENS_DEV_INFO_SERVICE_CHAR_MODEL_NUM, HANDLE_HSENS_DEV_INFO_SERVICE_CHAR_MODEL_NUM_VAL,
                GATT_UUID_MODEL_NUMBER_STR, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE ),

        /* Handle 0x52: characteristic System ID, handle 0x53 characteristic value */
        CHARACTERISTIC_UUID16( HANDLE_HSENS_DEV_INFO_SERVICE_CHAR_SYSTEM_ID, HANDLE_HSENS_DEV_INFO_SERVICE_CHAR_SYSTEM_ID_VAL,
                GATT_UUID_SYSTEM_ID, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE ),

    /* Declare Battery service */
    PRIMARY_SERVICE_UUID16( HANDLE_HSENS_BATTERY_SERVICE, UUID_SERVCLASS_BATTERY ),

        /* Handle 0x62: characteristic Battery Level, handle 0x63 characteristic value */
        CHARACTERISTIC_UUID16( HANDLE_HSENS_BATTERY_SERVICE_CHAR_LEVEL, HANDLE_HSENS_BATTERY_SERVICE_CHAR_LEVEL_VAL,
                GATT_UUID_BATTERY_LEVEL, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE),

#ifdef FASTPAIR_ENABLE
    // Declare Fast Pair service
    PRIMARY_SERVICE_UUID16 (HANDLE_FASTPAIR_SERVICE, WICED_BT_GFPS_UUID16),

    CHARACTERISTIC_UUID16_WRITABLE (HANDLE_FASTPAIR_SERVICE_CHAR_KEY_PAIRING,
                                    HANDLE_FASTPAIR_SERVICE_CHAR_KEY_PAIRING_VAL,
                                    WICED_BT_GFPS_UUID_CHARACTERISTIC_KEY_PAIRING,
                                    LEGATTDB_CHAR_PROP_WRITE | LEGATTDB_CHAR_PROP_NOTIFY,
                                    LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ),

    CHAR_DESCRIPTOR_UUID16_WRITABLE(HANDLE_FASTPAIR_SERVICE_CHAR_KEY_PAIRING_CFG_DESC,
                                    UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                    LEGATTDB_PERM_AUTH_READABLE | LEGATTDB_PERM_WRITE_REQ),

    CHARACTERISTIC_UUID16_WRITABLE (HANDLE_FASTPAIR_SERVICE_CHAR_PASSKEY,
                                    HANDLE_FASTPAIR_SERVICE_CHAR_PASSKEY_VAL,
                                    WICED_BT_GFPS_UUID_CHARACTERISTIC_PASSKEY,
                                    LEGATTDB_CHAR_PROP_WRITE | LEGATTDB_CHAR_PROP_NOTIFY,
                                    LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ),

    CHAR_DESCRIPTOR_UUID16_WRITABLE(HANDLE_FASTPAIR_SERVICE_CHAR_PASSKEY_CFG_DESC,
                                    UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                    LEGATTDB_PERM_AUTH_READABLE | LEGATTDB_PERM_WRITE_REQ),

    CHARACTERISTIC_UUID16_WRITABLE (HANDLE_FASTPAIR_SERVICE_CHAR_ACCOUNT_KEY,
                                    HANDLE_FASTPAIR_SERVICE_CHAR_ACCOUNT_KEY_VAL,
                                    WICED_BT_GFPS_UUID_CHARACTERISTIC_ACCOUNT_KEY,
                                    LEGATTDB_CHAR_PROP_WRITE,
                                    LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ),

    CHAR_DESCRIPTOR_UUID16_WRITABLE(HANDLE_FASTPAIR_SERVICE_CHAR_ACCOUNT_KEY_CFG_DESC,
                                    UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                    LEGATTDB_PERM_AUTH_READABLE | LEGATTDB_PERM_WRITE_REQ),
#endif

    /* WICED Upgrade Service. */
#ifdef OTA_SECURE_FIRMWARE_UPGRADE
    PRIMARY_SERVICE_UUID128(HANDLE_OTA_FW_UPGRADE_SERVICE, UUID_OTA_SEC_FW_UPGRADE_SERVICE),
#else
    PRIMARY_SERVICE_UUID128(HANDLE_OTA_FW_UPGRADE_SERVICE, UUID_OTA_FW_UPGRADE_SERVICE),
#endif
        /* characteristic Control Point */
        CHARACTERISTIC_UUID128_WRITABLE(HANDLE_OTA_FW_UPGRADE_CHARACTERISTIC_CONTROL_POINT, HANDLE_OTA_FW_UPGRADE_CONTROL_POINT,
            UUID_OTA_FW_UPGRADE_CHARACTERISTIC_CONTROL_POINT, LEGATTDB_CHAR_PROP_WRITE | LEGATTDB_CHAR_PROP_NOTIFY | LEGATTDB_CHAR_PROP_INDICATE,
            LEGATTDB_PERM_VARIABLE_LENGTH | LEGATTDB_PERM_WRITE_REQ),

        /* client characteristic configuration descriptor */
        CHAR_DESCRIPTOR_UUID16_WRITABLE(HANDLE_OTA_FW_UPGRADE_CLIENT_CONFIGURATION_DESCRIPTOR, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
            LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ),

        /* characteristic Data. */
        CHARACTERISTIC_UUID128_WRITABLE(HANDLE_OTA_FW_UPGRADE_CHARACTERISTIC_DATA, HANDLE_OTA_FW_UPGRADE_DATA,
            UUID_OTA_FW_UPGRADE_CHARACTERISTIC_DATA, LEGATTDB_CHAR_PROP_WRITE,
            LEGATTDB_PERM_VARIABLE_LENGTH | LEGATTDB_PERM_WRITE_REQ | LEGATTDB_PERM_RELIABLE_WRITE),



};

typedef struct
{
    uint16_t handle;
    uint16_t attr_len;
    void     *p_attr;
} attribute_t;

uint8_t btheadset_sensor_device_name[]          = "HeadsetPro";
uint8_t btheadset_sensor_appearance_name[2]     = { BIT16_TO_8(APPEARANCE_GENERIC_TAG) };
char    btheadset_sensor_char_notify_value[]    = { 'H', 'e', 'l', 'l', 'o', ' ', '0', };
char    btheadset_sensor_char_mfr_name_value[]  = { 'C', 'y', 'p', 'r', 'e', 's', 's', 0, };
char    btheadset_sensor_char_model_num_value[] = { '1', '2', '3', '4',   0,   0,   0,   0 };
uint8_t btheadset_sensor_char_system_id_value[] = { 0xbb, 0xb8, 0xa1, 0x80, 0x5f, 0x9f, 0x91, 0x71};

static uint8_t btheadset_battery_level;

static char *p_headset_control_le_dev_name = NULL;
static wiced_bt_ble_advert_elem_t headset_control_le_adv_elem = {0};
attribute_t gauAttributes[] =
{
    { HANDLE_HSENS_GAP_SERVICE_CHAR_DEV_NAME_VAL,       sizeof( btheadset_sensor_device_name ),         btheadset_sensor_device_name },
    { HANDLE_HSENS_GAP_SERVICE_CHAR_DEV_APPEARANCE_VAL, sizeof(btheadset_sensor_appearance_name),       btheadset_sensor_appearance_name },
    { HANDLE_HSENS_DEV_INFO_SERVICE_CHAR_MFR_NAME_VAL,  sizeof(btheadset_sensor_char_mfr_name_value),   btheadset_sensor_char_mfr_name_value },
    { HANDLE_HSENS_DEV_INFO_SERVICE_CHAR_MODEL_NUM_VAL, sizeof(btheadset_sensor_char_model_num_value),  btheadset_sensor_char_model_num_value },
    { HANDLE_HSENS_DEV_INFO_SERVICE_CHAR_SYSTEM_ID_VAL, sizeof(btheadset_sensor_char_system_id_value),  btheadset_sensor_char_system_id_value },
    { HANDLE_HSENS_BATTERY_SERVICE_CHAR_LEVEL_VAL,      1,                                            &btheadset_battery_level },
};

#if BTSTACK_VER >= 0x01020000
wiced_bt_db_hash_t headset_db_hash;
#endif

/******************************************************
 *               Function Definitions
 ******************************************************/
static wiced_bt_gatt_status_t hci_control_le_gatt_callback( wiced_bt_gatt_evt_t event, wiced_bt_gatt_event_data_t *p_data );
static wiced_result_t         hci_control_le_connection_up( wiced_bt_gatt_connection_status_t *p_status );
static wiced_result_t         hci_control_le_connection_down( wiced_bt_gatt_connection_status_t *p_status );
static wiced_result_t         hci_control_le_write_handler( uint16_t conn_id, wiced_bt_gatt_write_t * p_data );


static void headset_control_le_discoverabilty_change_callback(wiced_bool_t discoverable)
{
#ifdef FASTPAIR_ENABLE
    wiced_bt_gfps_provider_discoverablility_set(discoverable);
#endif
}

/*
 * Enable LE Control
 */
void hci_control_le_enable( void )
{
    wiced_bt_gatt_status_t     gatt_status;
#ifdef FASTPAIR_ENABLE
    wiced_bt_gfps_provider_conf_t fastpair_conf = {0};
#endif
    char appended_ble_dev_name[] = " LE";
    uint8_t *p_index;
    uint16_t dev_name_len;

    WICED_BT_TRACE( "hci_control_le_enable\n" );

    /*  GATT DB Initialization */
#if BTSTACK_VER >= 0x01020000
    gatt_status = wiced_bt_gatt_db_init(gatt_server_db, sizeof(gatt_server_db),
            headset_db_hash);
#else
    gatt_status = wiced_bt_gatt_db_init(gatt_server_db, sizeof(gatt_server_db));
#endif

    WICED_BT_TRACE("wiced_bt_gatt_db_init %d\n", gatt_status);

#ifdef FASTPAIR_ENABLE
    // set Tx power level data type in ble advertisement
#if defined(CYW20719B2) || defined(CYW20721B2) || defined(CYW20819A1) || defined (CYW20820A1)
    fastpair_conf.ble_tx_pwr_level = wiced_bt_cfg_settings.default_ble_power_level;
#else
    fastpair_conf.ble_tx_pwr_level = 0;
#endif

    // set GATT event callback
    fastpair_conf.p_gatt_cb = hci_control_le_gatt_callback;

    // set assigned handles for GATT attributes
    fastpair_conf.gatt_db_handle.key_pairing_val        = HANDLE_FASTPAIR_SERVICE_CHAR_KEY_PAIRING_VAL;
    fastpair_conf.gatt_db_handle.key_pairing_cfg_desc   = HANDLE_FASTPAIR_SERVICE_CHAR_KEY_PAIRING_CFG_DESC;
    fastpair_conf.gatt_db_handle.passkey_val            = HANDLE_FASTPAIR_SERVICE_CHAR_PASSKEY_VAL;
    fastpair_conf.gatt_db_handle.passkey_cfg_desc       = HANDLE_FASTPAIR_SERVICE_CHAR_PASSKEY_CFG_DESC;
    fastpair_conf.gatt_db_handle.account_key_val        = HANDLE_FASTPAIR_SERVICE_CHAR_ACCOUNT_KEY_VAL;

    // model id
    fastpair_conf.model_id = FASTPAIR_MODEL_ID;

    // anti-spoofing public key
    memcpy((void *) &fastpair_conf.anti_spoofing_key.public[0],
           (void *) &anti_spoofing_public_key[0],
           WICED_BT_GFPS_ANTI_SPOOFING_KEY_LEN_PUBLIC);

    // anti-spoofing private key
    memcpy((void *) &fastpair_conf.anti_spoofing_key.private[0],
           (void *) &anti_spoofing_private_key[0],
           WICED_BT_GFPS_ANTI_SPOOFING_KEY_LEN_PRIVATE);

    // Account Key Filter generate format
    fastpair_conf.account_key_filter_generate_random = WICED_TRUE;;

    // Account Key list size
    fastpair_conf.account_key_list_size = FASTPAIR_ACCOUNT_KEY_NUM;

    // NVRAM id for Account Key list
    fastpair_conf.account_key_list_nvram_id = HEADSET_NVRAM_ID_GFPS_ACCOUNT_KEY;

    // BLE advertisement appended to fast pair advertisement data
    dev_name_len = strlen((char *) wiced_bt_cfg_settings.device_name) +
                   strlen(appended_ble_dev_name);

    p_headset_control_le_dev_name = (char *) wiced_memory_allocate(dev_name_len);

    if (p_headset_control_le_dev_name)
    {
        p_index = (uint8_t *) p_headset_control_le_dev_name;

        memcpy((void *) p_index,
               (void *) wiced_bt_cfg_settings.device_name,
               strlen((char *) wiced_bt_cfg_settings.device_name));

        p_index += strlen((char *) wiced_bt_cfg_settings.device_name);

        memcpy((void *) p_index,
               (void *) appended_ble_dev_name,
               strlen(appended_ble_dev_name));
    }
    else
    {
        dev_name_len = 0;
    }

    headset_control_le_adv_elem.advert_type    = BTM_BLE_ADVERT_TYPE_NAME_COMPLETE;
    headset_control_le_adv_elem.len            = dev_name_len;
    headset_control_le_adv_elem.p_data         = (uint8_t *) p_headset_control_le_dev_name;

    fastpair_conf.appended_adv_data.p_elem      = &headset_control_le_adv_elem;
    fastpair_conf.appended_adv_data.elem_num    = 1;

    /* Initialize Google Fast Pair Service. */
    if (wiced_bt_gfps_provider_init(&fastpair_conf) == WICED_FALSE)
    {
        WICED_BT_TRACE("wiced_bt_gfps_provider_init fail\n");
    }

#else
    /* GATT registration */
    gatt_status = wiced_bt_gatt_register( hci_control_le_gatt_callback );
    WICED_BT_TRACE( "wiced_bt_gatt_register status %d\n", gatt_status );

#endif

    /* Register the BLE discoverability change callback. */
    bt_hs_spk_ble_discoverability_change_callback_register(&headset_control_le_discoverabilty_change_callback);

}


/*
 * Process connection up event
 */
wiced_result_t hci_control_le_connection_up( wiced_bt_gatt_connection_status_t *p_status )
{
    WICED_BT_TRACE( "le_connection_up, id:%d bd (%B) role:%d\n:", p_status->conn_id, p_status->bd_addr);

    return ( WICED_SUCCESS );
}

/*
* Process connection down event
*/
wiced_result_t hci_control_le_connection_down( wiced_bt_gatt_connection_status_t *p_status )
{
    WICED_BT_TRACE( "le_connection_down id:%x Disc_Reason: %02x\n", p_status->conn_id, p_status->reason );

    return ( WICED_SUCCESS );
}


/*
* Process connection status callback
*/
wiced_result_t hci_control_le_conn_status_callback( wiced_bt_gatt_connection_status_t *p_status )
{
#ifdef OTA_FW_UPGRADE
    wiced_ota_fw_upgrade_connection_status_event(p_status);
#endif

    if ( p_status->connected )
    {
        return hci_control_le_connection_up( p_status );
    }
    else
    {
        return hci_control_le_connection_down( p_status );
    }
}


/*
 * Find attribute description by handle
 */
attribute_t * hci_control_get_attribute( uint16_t handle )
            {
    int i;
    for ( i = 0; i <  sizeof( gauAttributes ) / sizeof( gauAttributes[0] ); i++ )
    {
        if ( gauAttributes[i].handle == handle )
        {
            return ( &gauAttributes[i] );
            }
    }
    WICED_BT_TRACE( "attr not found:%x\n", handle );
    return NULL;
}

/*
 * Process Read request or command from peer device
 */
wiced_bt_gatt_status_t hci_control_le_get_value( uint16_t conn_id, wiced_bt_gatt_read_t * p_read_data )
{
    attribute_t *puAttribute;
    int          attr_len_to_copy;

    if ( ( puAttribute = hci_control_get_attribute(p_read_data->handle) ) == NULL)
    {
        WICED_BT_TRACE("read_hndlr attr not found hdl:%x\n", p_read_data->handle );
        return WICED_BT_GATT_INVALID_HANDLE;
    }

    /* Dummy battery value read increment */
    if( p_read_data->handle == HANDLE_HSENS_BATTERY_SERVICE_CHAR_LEVEL_VAL)
    {
        if ( btheadset_battery_level++ > 5)
        {
            btheadset_battery_level = 0;
        }
    }

    attr_len_to_copy = puAttribute->attr_len;

    WICED_BT_TRACE("read_hndlr conn_id:%d hdl:%x offset:%d len:%d\n", conn_id, p_read_data->handle, p_read_data->offset, attr_len_to_copy );

    if ( p_read_data->offset >= puAttribute->attr_len )
    {
        attr_len_to_copy = 0;
    }

    if ( attr_len_to_copy != 0 )
    {
        uint8_t *from;
        int      to_copy = attr_len_to_copy - p_read_data->offset;


        if ( to_copy > *p_read_data->p_val_len )
        {
            to_copy = *p_read_data->p_val_len;
        }

        from = ((uint8_t *)puAttribute->p_attr) + p_read_data->offset;
        *p_read_data->p_val_len = to_copy;

        memcpy( p_read_data->p_val, from, to_copy);
    }

    return WICED_BT_GATT_SUCCESS;
}

/*
 * This function is called when peer issues a Read Request to access characteristics values
 * in the GATT database.  Application can fill the provided buffer and return SUCCESS,
 * return error if something not appropriate, or return PENDING and send Read Response
 * when data is ready.
 */

wiced_bt_gatt_status_t hci_control_le_read_handler( uint16_t conn_id, wiced_bt_gatt_read_t *p_req )
{
#ifdef OTA_FW_UPGRADE
    if (wiced_ota_fw_upgrade_is_gatt_handle(p_req->handle))
    {
        return wiced_ota_fw_upgrade_read_handler(conn_id, p_req);
    }
#endif

    WICED_BT_TRACE("handle:%d, conn_id:%d val:%d length:%d\n"
            , p_req->handle, conn_id, p_req->p_val,  p_req->p_val_len);

    return  hci_control_le_get_value(conn_id, p_req);
}


/*
 * This function is called when peer issues a Write request to access characteristics values
 * in the GATT database
 */
wiced_result_t hci_control_le_write_handler( uint16_t conn_id, wiced_bt_gatt_write_t *p_req )
{
#ifdef OTA_FW_UPGRADE
    if (wiced_ota_fw_upgrade_is_gatt_handle(p_req->handle))
    {
        return wiced_ota_fw_upgrade_write_handler(conn_id, p_req);
    }
#endif

    WICED_BT_TRACE( "hci_control_le_write_handler: conn_id:%d handle:%04x\n", conn_id, p_req->handle);

    return ( WICED_BT_GATT_SUCCESS );
}


/*
 * Process indication confirm.
 */
wiced_result_t  hci_control_le_conf_handler( uint16_t conn_id, uint16_t handle )
{
#ifdef OTA_FW_UPGRADE
    if (wiced_ota_fw_upgrade_is_gatt_handle(handle))
    {
        return wiced_ota_fw_upgrade_indication_cfm_handler(conn_id, handle);
    }
#endif

    WICED_BT_TRACE( "hci_control_le_conf_handler conn_id:%d handle:%x\n", conn_id, handle );

    return WICED_SUCCESS;
}

/*
 * This is a GATT request callback
 */
wiced_bt_gatt_status_t hci_control_le_gatt_req_cb( wiced_bt_gatt_attribute_request_t *p_req )
{
    wiced_bt_gatt_status_t result  = WICED_BT_GATT_SUCCESS;

    switch ( p_req->request_type )
    {
        case GATTS_REQ_TYPE_READ:
            result = hci_control_le_read_handler( p_req->conn_id, &p_req->data.read_req );
            break;

        case GATTS_REQ_TYPE_WRITE:
        case GATTS_REQ_TYPE_PREP_WRITE:
            result = hci_control_le_write_handler( p_req->conn_id, &p_req->data.write_req );
             break;

        case GATTS_REQ_TYPE_MTU:
		WICED_BT_TRACE( "conn_id:%d mtu:%x\n", p_req->conn_id, p_req->data.mtu);
            break;

        case GATTS_REQ_TYPE_CONF:
            result = hci_control_le_conf_handler( p_req->conn_id, p_req->data.handle );
            break;

       default:
            WICED_BT_TRACE("Invalid GATT request conn_id:%d type:%d\n", p_req->conn_id, p_req->request_type);
            break;
    }

    return result;
}

wiced_bt_gatt_status_t hci_control_le_gatt_callback( wiced_bt_gatt_evt_t event, wiced_bt_gatt_event_data_t *p_data )
{
    wiced_bt_gatt_status_t result = WICED_SUCCESS;

    switch( event )
    {
    case GATT_CONNECTION_STATUS_EVT:
        result = hci_control_le_conn_status_callback( &p_data->connection_status );
        break;

    case GATT_OPERATION_CPLT_EVT:
	WICED_BT_TRACE( "ERROR...GATT_OPERATION_CPLT_EVT\n");
        break;

    case GATT_DISCOVERY_RESULT_EVT:
	WICED_BT_TRACE( "ERROR...GATT_DISCOVERY_RESULT_EVT%d\n");
        break;

    case GATT_DISCOVERY_CPLT_EVT:
	WICED_BT_TRACE( "ERROR...GATT_DISCOVERY_CPLT_EVT\n");
        break;

    case GATT_ATTRIBUTE_REQUEST_EVT:
        result = hci_control_le_gatt_req_cb( &p_data->attribute_request );
        break;

    default:
        break;
    }

    return result;
}
