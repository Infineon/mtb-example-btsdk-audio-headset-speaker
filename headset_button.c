/*
 * Copyright 2016-2020, Cypress Semiconductor Corporation or a subsidiary of
 * Cypress Semiconductor Corporation. All Rights Reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software"), is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
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

/*
 * @btheadset_button.c : Application's Button-list entries
 */

#include "wiced.h"
#include "bt_hs_spk_control.h"
#include "bt_hs_spk_button.h"
#include "wiced_button_manager.h"
#include "wiced_platform.h"

/******************************************************
 *                      Macros
 ******************************************************/
#ifndef APP_BUTTON_MAX
#define APP_BUTTON_MAX WICED_PLATFORM_BUTTON_MAX_DEF
#endif


static wiced_button_manager_configuration_t app_button_manager_configuration =
{
    .short_hold_duration     = 500, /*msec*/
    .medium_hold_duration    = 700,
    .long_hold_duration      = 1500,
    .very_long_hold_duration = 2500,
    .debounce_duration       = 150, /* typically a click takes around ~150-200 ms */
    .continuous_hold_detect  = WICED_TRUE,
    /*if NULL button events are handled by bt_hs_spk library*/
    .event_handler = NULL,
};

/* Static button configuration */
static wiced_button_configuration_t app_button_configurations[] =
{
#if defined(CYW43012C0)
    [ PLAY_PAUSE_BUTTON ]                   = { PLATFORM_BUTTON_1, BUTTON_CLICK_EVENT | BUTTON_LONG_DURATION_EVENT | BUTTON_VERY_LONG_DURATION_EVENT , 0 },
#else
    [ PLAY_PAUSE_BUTTON ]                   = { PLATFORM_BUTTON_1, BUTTON_CLICK_EVENT | BUTTON_LONG_DURATION_EVENT | BUTTON_VERY_LONG_DURATION_EVENT , 0 },
    [ VOLUME_UP_NEXT_TRACK_BUTTON ]         = { PLATFORM_BUTTON_2, BUTTON_CLICK_EVENT | BUTTON_LONG_DURATION_EVENT | BUTTON_VERY_LONG_DURATION_EVENT | BUTTON_HOLDING_EVENT , 0 },
    [ VOLUME_DOWN_PREVIOUS_TRACK_BUTTON ]   = { PLATFORM_BUTTON_3, BUTTON_CLICK_EVENT | BUTTON_LONG_DURATION_EVENT | BUTTON_VERY_LONG_DURATION_EVENT | BUTTON_HOLDING_EVENT , 0 },
#if (APP_BUTTON_MAX >= 4)
    [ VOICE_REC_BUTTON ]                    = { PLATFORM_BUTTON_4, BUTTON_CLICK_EVENT | BUTTON_LONG_DURATION_EVENT | BUTTON_VERY_LONG_DURATION_EVENT, 0 },
#endif
#endif
};

/* Button objects for the button manager */
button_manager_button_t app_buttons[] =
{
#if defined(CYW43012C0)
    [ PLAY_PAUSE_BUTTON ]                   = { &app_button_configurations[ PLAY_PAUSE_BUTTON ]        },
#else
    [ PLAY_PAUSE_BUTTON ]                   = { &app_button_configurations[ PLAY_PAUSE_BUTTON ]        },
    [ VOLUME_UP_NEXT_TRACK_BUTTON ]         = { &app_button_configurations[ VOLUME_UP_NEXT_TRACK_BUTTON ]     },
    [ VOLUME_DOWN_PREVIOUS_TRACK_BUTTON ]   = { &app_button_configurations[ VOLUME_DOWN_PREVIOUS_TRACK_BUTTON ]  },
#if (APP_BUTTON_MAX >= 4)
    [ VOICE_REC_BUTTON ]                    = { &app_button_configurations[ VOICE_REC_BUTTON ] },
#endif
#endif
};

static button_manager_t app_button_manager;

static bt_hs_spk_button_action_t app_button_action[] =
{
#if defined(CYW43012C0)
    /* PLAY_PAUSE_BUTTON */
    {
        .action = ACTION_PAUSE_PLAY,
        .button = PLAY_PAUSE_BUTTON,
        .event  = BUTTON_CLICK_EVENT,
        .state  = BUTTON_STATE_RELEASED,
    },
    {
        .action = ACTION_BT_DISCOVERABLE,
        .button = PLAY_PAUSE_BUTTON,
        .event  = BUTTON_LONG_DURATION_EVENT,
        .state  = BUTTON_STATE_HELD,
    },
#else
    /* PLAY_PAUSE_BUTTON */
    {
        .action = ACTION_PAUSE_PLAY,
        .button = PLAY_PAUSE_BUTTON,
        .event  = BUTTON_CLICK_EVENT,
        .state  = BUTTON_STATE_RELEASED,
    },
    {
        .action = ACTION_BT_DISCOVERABLE,
        .button = PLAY_PAUSE_BUTTON,
        .event  = BUTTON_LONG_DURATION_EVENT,
        .state  = BUTTON_STATE_HELD,
    },

    /* VOLUME_UP_NEXT_TRACK_BUTTON */
    {
        .action = ACTION_VOLUME_UP,
        .button = VOLUME_UP_NEXT_TRACK_BUTTON,
        .event  = BUTTON_CLICK_EVENT,
        .state  = BUTTON_STATE_RELEASED,
    },
    {
        .action = ACTION_FORWARD,
        .button = VOLUME_UP_NEXT_TRACK_BUTTON,
        .event  = BUTTON_LONG_DURATION_EVENT,
        .state  = BUTTON_STATE_RELEASED,
    },
#if (APP_BUTTON_MAX < 4)
    {
        .action = ACTION_VOICE_RECOGNITION,
        .button = VOLUME_UP_NEXT_TRACK_BUTTON,
        .event  = BUTTON_HOLDING_EVENT,
        .state  = BUTTON_STATE_HELD,
        .repeat = 2,
    },
#endif

    /* VOLUME_DOWN_PREVIOUS_TRACK_BUTTON */
    {
        .action = ACTION_VOLUME_DOWN,
        .button = VOLUME_DOWN_PREVIOUS_TRACK_BUTTON,
        .event  = BUTTON_CLICK_EVENT,
        .state  = BUTTON_STATE_RELEASED,
    },
    {
        .action = ACTION_BACKWARD,
        .button = VOLUME_DOWN_PREVIOUS_TRACK_BUTTON,
        .event  = BUTTON_LONG_DURATION_EVENT,
        .state  = BUTTON_STATE_RELEASED,
    },
#if (APP_BUTTON_MAX < 4)
    {
        .action = ACTION_TRANSPORT_DETECT_ON,
        .button = VOLUME_DOWN_PREVIOUS_TRACK_BUTTON,
        .event  = BUTTON_HOLDING_EVENT,
        .state  = BUTTON_STATE_HELD,
        .repeat = 2,
    },
#endif

#if (APP_BUTTON_MAX >= 4)
    /* VOICE_REC_BUTTON */
    {
        .action = ACTION_VOICE_RECOGNITION,
        .button = VOICE_REC_BUTTON,
        .event  = BUTTON_LONG_DURATION_EVENT,
        .state  = BUTTON_STATE_HELD,
    },
    {
        .action = ACTION_TRANSPORT_DETECT_ON,
        .button = VOICE_REC_BUTTON,
        .event  = BUTTON_CLICK_EVENT,
        .state  = BUTTON_STATE_RELEASED,
    },
#endif
#endif
};

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t btheadset_init_button_interface(void)
{
    wiced_result_t result;
    bt_hs_spk_button_config_t config;

    config.p_manager                                = &app_button_manager;
    config.p_configuration                          = &app_button_manager_configuration;
    config.p_app_buttons                            = app_buttons;
    config.number_of_buttons                        = ARRAY_SIZE(app_buttons);
    config.p_pre_handler                            = NULL;
    config.button_action_config.p_action            = app_button_action;
    config.button_action_config.number_of_actions   = ARRAY_SIZE(app_button_action);

    result = bt_hs_spk_init_button_interface(&config);
    return result;
}
