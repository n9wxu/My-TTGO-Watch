/****************************************************************************
 *   Tu May 22 21:23:51 2020
 *   Copyright  2020  Dirk Brosswick
 *   Email: dirk.brosswick@googlemail.com
 ****************************************************************************/
 
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "config.h"
#include <TTGO.h>

#include "display.h"
#include "powermgm.h"
#include "motor.h"
#include "bma.h"
#include "framebuffer.h"
#include "gui/gui.h"

display_config_t display_config;
callback_t *display_callback = NULL;

static uint8_t dest_brightness = 0;
static uint8_t brightness = 0;

bool display_powermgm_event_cb( EventBits_t event, void *arg );
bool display_powermgm_loop_cb( EventBits_t event, void *arg );
bool display_send_event_cb( EventBits_t event, void *arg );

void display_setup( void ) {
    /**
     * load config from json
     */
    display_config.load();
    /**
     * setup backlight and rotation
     */
    TTGOClass *ttgo = TTGOClass::getWatch();
    ttgo->openBL();
    ttgo->bl->adjust( 0 );
    ttgo->tft->setRotation( display_config.rotation / 90 );
    bma_set_rotate_tilt( display_config.rotation );
    /**
     * setup framebuffer
     */
    framebuffer_setup( display_config.use_dma, display_config.use_double_buffering );
    /**
     * register powermgm and pwermgm loop callback functions
     */
    powermgm_register_cb( POWERMGM_SILENCE_WAKEUP | POWERMGM_STANDBY | POWERMGM_WAKEUP, display_powermgm_event_cb, "powermgm display" );
    powermgm_register_loop_cb( POWERMGM_WAKEUP, display_powermgm_loop_cb, "powermgm display loop" );
}

bool display_powermgm_event_cb( EventBits_t event, void *arg ) {
    switch( event ) {
        case POWERMGM_STANDBY:          display_standby();
                                        break;
        case POWERMGM_WAKEUP:           display_wakeup( false );
                                        break;
        case POWERMGM_SILENCE_WAKEUP:   display_wakeup( true );
                                        break;
    }
    return( true );
}

bool display_powermgm_loop_cb( EventBits_t event, void *arg ) {
    display_loop();
    return( true );
}

void display_loop( void ) {
  TTGOClass *ttgo = TTGOClass::getWatch();
    /**
     * check if backlight adjust has change
     */
    if ( dest_brightness != brightness ) {
        if ( brightness < dest_brightness ) {
            brightness++;
            ttgo->bl->adjust( brightness );
        }
        else {
            brightness--;
            ttgo->bl->adjust( brightness );
        }
  }
  /**
   * check timeout
   */
  if ( display_get_timeout() != DISPLAY_MAX_TIMEOUT ) {
        if ( lv_disp_get_inactive_time(NULL) > ( ( display_get_timeout() * 1000 ) - display_get_brightness() * 8 ) ) {
            dest_brightness = ( ( display_get_timeout() * 1000 ) - lv_disp_get_inactive_time( NULL ) ) / 8 ;
        }
        else {
            dest_brightness = display_get_brightness();
        }
    }
}

bool display_register_cb( EventBits_t event, CALLBACK_FUNC callback_func, const char *id ) {
    if ( display_callback == NULL ) {
        display_callback = callback_init( "display" );
        if ( display_callback == NULL ) {
            log_e("display_callback_callback alloc failed");
            while(true);
        }
    }
    return( callback_register( display_callback, event, callback_func, id ) );
}

bool display_send_event_cb( EventBits_t event, void *arg ) {
    return( callback_send( display_callback, event, arg ) );
}

void display_standby( void ) {
    TTGOClass *ttgo = TTGOClass::getWatch();
    log_i("go standby");
    ttgo->bl->adjust( 0 );
    ttgo->displaySleep();
    ttgo->closeBL();
    brightness = 0;
    dest_brightness = 0;
    #if defined( LILYGO_WATCH_2020_V2 )
        ttgo->power->setLDO2Voltage( 3300 );
        ttgo->power->setLDO3Voltage( 3300 );
        ttgo->power->setPowerOutPut( AXP202_LDO2, false );
        ttgo->power->setPowerOutPut( AXP202_LDO3, false );
    #endif
}

void display_wakeup( bool silence ) {
    TTGOClass *ttgo = TTGOClass::getWatch();
    /**
     * wakeup with or without display
     */
    if ( silence ) {
        log_i("go silence wakeup");
        #if defined( LILYGO_WATCH_2020_V2 )
            ttgo->power->setLDO2Voltage( 3300 );
            ttgo->power->setLDO3Voltage( 3300 );
            ttgo->power->setPowerOutPut( AXP202_LDO2, true );
            ttgo->power->setPowerOutPut( AXP202_LDO3, true );
        #endif
        ttgo->openBL();
        ttgo->displayWakeup();
        ttgo->bl->adjust( 0 );
        brightness = 0;
        dest_brightness = 0;
    }
    else {
        log_i("go wakeup");
        #if defined( LILYGO_WATCH_2020_V2 )
            ttgo->power->setLDO2Voltage( 3300 );
            ttgo->power->setLDO3Voltage( 3300 );
            ttgo->power->setPowerOutPut( AXP202_LDO2, true );
            ttgo->power->setPowerOutPut( AXP202_LDO3, true );
        #endif
        ttgo->openBL();
        ttgo->displayWakeup();
        ttgo->bl->adjust( 0 );
        brightness = 0;
        dest_brightness = display_get_brightness();
    }
}

void display_save_config( void ) {
      display_config.save();
}

void display_read_config( void ) {
    display_config.load();
}

uint32_t display_get_timeout( void ) {
    return( display_config.timeout );
}

void display_set_timeout( uint32_t timeout ) {
    display_config.timeout = timeout;
    display_send_event_cb( DISPLAYCTL_TIMEOUT, (void *)timeout );
}

uint32_t display_get_brightness( void ) {
    return( display_config.brightness );
}

void display_set_brightness( uint32_t brightness ) {
    display_config.brightness = brightness;
    dest_brightness = brightness;
    display_send_event_cb( DISPLAYCTL_BRIGHTNESS, (void *)brightness );
}

uint32_t display_get_rotation( void ) {
    return( display_config.rotation );
}

bool display_get_block_return_maintile( void ) {
    return( display_config.block_return_maintile );
}

bool display_get_use_double_buffering( void ) {
    return( display_config.use_double_buffering );
}

void display_set_use_double_buffering( bool use_double_buffering ) {
    display_config.use_double_buffering = use_double_buffering;
    framebuffer_setup( display_config.use_dma, display_config.use_double_buffering );
}

bool display_get_use_dma( void ) {
    return( display_config.use_dma );
}

void display_set_use_dma( bool use_dma ) {
    display_config.use_dma = use_dma;
    framebuffer_setup( display_config.use_dma, display_config.use_double_buffering );
}

void display_set_block_return_maintile( bool block_return_maintile ) {
    display_config.block_return_maintile = block_return_maintile;
}

void display_set_rotation( uint32_t rotation ) {
    TTGOClass *ttgo = TTGOClass::getWatch();
    display_config.rotation = rotation;
    ttgo->tft->setRotation( rotation / 90 );
    lv_obj_invalidate( lv_scr_act() );
}

uint32_t display_get_background_image( void ) {
    return( display_config.background_image );
}

void display_set_background_image( uint32_t background_image ) {
    display_config.background_image = background_image;
}

void display_set_vibe( bool vibe ) {
    display_config.vibe = vibe;
}

bool display_get_vibe( void ) {
    return display_config.vibe;
}
