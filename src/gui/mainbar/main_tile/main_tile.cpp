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
#include <stdio.h>
#include <time.h>

#include "config.h"
#include "main_tile.h"

#include "gui/mainbar/mainbar.h"
#include "gui/mainbar/setup_tile/time_settings/time_settings.h"
#include "gui/widget_styles.h"

#include "hardware/timesync.h"
#include "hardware/powermgm.h"

#include "utils/alloc.h"

static bool maintile_init = false;

static lv_obj_t *main_cont = NULL;
static lv_obj_t *clock_cont = NULL;
static lv_obj_t *timelabel = NULL;
static lv_obj_t *datelabel = NULL;
uint32_t main_tile_num;

static lv_style_t *style;
static lv_style_t timestyle;
static lv_style_t datestyle;

icon_t widget_entry[ MAX_WIDGET_NUM ];

LV_FONT_DECLARE(Ubuntu_72px);
LV_FONT_DECLARE(Ubuntu_16px);

lv_task_t * main_tile_task;

void main_tile_update_task( lv_task_t * task );
void main_tile_align_widgets( void );
bool main_tile_powermgm_event_cb( EventBits_t event, void *arg );
bool main_tile_time_update_ebent_cb( EventBits_t event, void *arg );

void main_tile_setup( void ) {
    /*
     * check if maintile alread initialized
     */
    if ( maintile_init ) {
        log_e("maintile already initialized");
        return;
    }

    main_tile_num = mainbar_add_tile( 0, 0, "main tile" );
    main_cont = mainbar_get_tile_obj( main_tile_num );
    style = ws_get_mainbar_style();

    lv_style_copy( &timestyle, style);
    lv_style_set_text_font( &timestyle, LV_STATE_DEFAULT, &Ubuntu_72px);

    lv_style_copy( &datestyle, style);
    lv_style_set_text_font( &datestyle, LV_STATE_DEFAULT, &Ubuntu_16px);

    clock_cont = mainbar_obj_create( main_cont );
    lv_obj_set_size( clock_cont, lv_disp_get_hor_res( NULL ) , lv_disp_get_ver_res( NULL ) / 2 );
    lv_obj_add_style( clock_cont, LV_OBJ_PART_MAIN, style );
    lv_obj_align( clock_cont, main_cont, LV_ALIGN_CENTER, 0, 0 );

    timelabel = lv_label_create( clock_cont , NULL);
    lv_label_set_text(timelabel, "00:00");
    lv_obj_reset_style_list( timelabel, LV_OBJ_PART_MAIN );
    lv_obj_add_style( timelabel, LV_OBJ_PART_MAIN, &timestyle );
    lv_obj_align(timelabel, NULL, LV_ALIGN_CENTER, 0, 0);

    datelabel = lv_label_create( clock_cont , NULL);
    lv_label_set_text(datelabel, "1.Jan 1970");
    lv_obj_reset_style_list( datelabel, LV_OBJ_PART_MAIN );
    lv_obj_add_style( datelabel, LV_OBJ_PART_MAIN, &datestyle );
    lv_obj_align( datelabel, clock_cont, LV_ALIGN_IN_BOTTOM_MID, 0, 0 );

    main_tile_update_time( true );

    for ( int widget = 0 ; widget < MAX_WIDGET_NUM ; widget++ ) {
        widget_entry[ widget ].active = false;

        widget_entry[ widget ].icon_cont = mainbar_obj_create( main_cont );
        lv_obj_reset_style_list( widget_entry[ widget ].icon_cont, LV_OBJ_PART_MAIN );
        lv_obj_add_style( widget_entry[ widget ].icon_cont, LV_OBJ_PART_MAIN, style );
        lv_obj_set_size( widget_entry[ widget ].icon_cont, WIDGET_X_SIZE, WIDGET_Y_SIZE );
        lv_obj_set_hidden( widget_entry[ widget ].icon_cont, true );
        // create app label
        widget_entry[ widget ].label = lv_label_create( widget_entry[ widget ].icon_cont , NULL );
        mainbar_add_slide_element( widget_entry[ widget ].label);
        lv_obj_reset_style_list( widget_entry[ widget ].label, LV_OBJ_PART_MAIN );
        lv_obj_add_style( widget_entry[ widget ].label, LV_OBJ_PART_MAIN, style );
        lv_obj_set_size( widget_entry[ widget ].label, WIDGET_X_SIZE, WIDGET_LABEL_Y_SIZE );
        lv_obj_align( widget_entry[ widget ].label , widget_entry[ widget ].icon_cont, LV_ALIGN_IN_BOTTOM_MID, 0, 0 );
        // create app label
        widget_entry[ widget ].ext_label = lv_label_create( widget_entry[ widget ].icon_cont , NULL );
        mainbar_add_slide_element( widget_entry[ widget ].ext_label);
        lv_obj_reset_style_list( widget_entry[ widget ].ext_label, LV_OBJ_PART_MAIN );
        lv_obj_add_style( widget_entry[ widget ].ext_label, LV_OBJ_PART_MAIN, style );
        lv_obj_set_size( widget_entry[ widget ].ext_label, WIDGET_X_SIZE, WIDGET_LABEL_Y_SIZE );
        lv_obj_align( widget_entry[ widget ].ext_label , widget_entry[ widget ].label, LV_ALIGN_OUT_TOP_MID, 0, 0 );
        // create img and indicator
        widget_entry[ widget ].icon_img = lv_imgbtn_create( widget_entry[ widget ].icon_cont , NULL );
        widget_entry[ widget ].icon_indicator = lv_img_create( widget_entry[ widget ].icon_cont, NULL );
        // hide all
        lv_obj_set_hidden( widget_entry[ widget ].icon_cont, true );
        lv_obj_set_hidden( widget_entry[ widget ].icon_img, true );
        lv_obj_set_hidden( widget_entry[ widget ].icon_indicator, true );
        lv_obj_set_hidden( widget_entry[ widget ].label, true );
        lv_obj_set_hidden( widget_entry[ widget ].ext_label, true );
    }

    main_tile_task = lv_task_create( main_tile_update_task, 500, LV_TASK_PRIO_MID, NULL );

    powermgm_register_cb( POWERMGM_WAKEUP , main_tile_powermgm_event_cb, "main tile time update" );
    timesync_register_cb( TIME_SYNC_UPDATE, main_tile_time_update_ebent_cb, "main tile time sync" );

    maintile_init = true;
}

bool main_tile_time_update_ebent_cb( EventBits_t event, void *arg ) {
    switch( event ) {
        case TIME_SYNC_UPDATE:
            main_tile_update_time( true );
            break;
    }
    return( true );
}

bool main_tile_powermgm_event_cb( EventBits_t event, void *arg ) {
    /*
     * check if maintile alread initialized
     */
    if ( !maintile_init ) {
        log_e("maintile not initialized");
        return( true );
    }

    switch( event ) {
        case POWERMGM_WAKEUP:
            main_tile_update_time( true );
            break;
    }
    return( true );
}

lv_obj_t *main_tile_register_widget( void ) {
    /*
     * check if maintile alread initialized
     */
    if ( !maintile_init ) {
        log_e("maintile not initialized");
        while( true );
    }

    for( int widget = 0 ; widget < MAX_WIDGET_NUM ; widget++ ) {
        if ( widget_entry[ widget ].active == false ) {
            widget_entry[ widget ].active = true;
            lv_obj_set_hidden( widget_entry[ widget ].icon_cont, false );
            main_tile_align_widgets();
            return( widget_entry[ widget ].icon_cont );
        }
    }
    log_e("no more space for a widget");
    return( NULL );
}

icon_t *main_tile_get_free_widget_icon( void ) {
    /*
     * check if maintile alread initialized
     */
    if ( !maintile_init ) {
        log_e("maintile not initialized");
        while( true );
    }

    for( int widget = 0 ; widget < MAX_WIDGET_NUM ; widget++ ) {
        if ( widget_entry[ widget ].active == false ) {
            lv_obj_set_hidden( widget_entry[ widget ].icon_cont, false );
            return( &widget_entry[ widget ] );
        }
    }
    log_e("no more space for a widget");
    return( NULL );
}

void main_tile_align_widgets( void ) {
    /*
     * check if maintile alread initialized
     */
    if ( !maintile_init ) {
        log_e("maintile not initialized");
        return;
    }

    int active_widgets = 0;
    lv_coord_t xpos = 0;

    for ( int widget = 0 ; widget < MAX_WIDGET_NUM ; widget++ ) {
        if ( widget_entry[ widget ].active )
        active_widgets++;
    }

    if ( active_widgets == 0 ) {
        lv_obj_align( clock_cont, main_cont, LV_ALIGN_CENTER, 0, 0 );
        return;
    };

    lv_obj_align( clock_cont, main_cont, LV_ALIGN_IN_TOP_MID, 0, 0 );

    xpos = 0 - ( ( WIDGET_X_SIZE * active_widgets ) + ( ( active_widgets - 1 ) * WIDGET_X_CLEARENCE ) ) / 2;

    active_widgets = 0;

    for ( int widget = 0 ; widget < MAX_WIDGET_NUM ; widget++ ) {
        if ( widget_entry[ widget ].active ) {
            lv_obj_align( widget_entry[ widget ].icon_cont , main_cont, LV_ALIGN_IN_BOTTOM_MID, xpos + ( WIDGET_X_SIZE * active_widgets ) + ( active_widgets * WIDGET_X_CLEARENCE ) + 32 , -32 );
            active_widgets++;
        }
    }

}

uint32_t main_tile_get_tile_num( void ) {
    /*
     * check if maintile alread initialized
     */
    if ( !maintile_init ) {
        log_e("maintile not initialized");
        while( true );
    }

    return( main_tile_num );
}

void main_tile_update_time( bool force ) {
    /*
     * check if maintile alread initialized
     */
    if ( !maintile_init ) {
        log_e("maintile not initialized");
        return;
    }

    time_t now;
    static time_t last = 0;
    struct tm  info, last_info;
    char time_str[64]="";
    /*
     * copy current time into now and convert it local time info
     */
    time( &now );
    localtime_r( &now, &info );
    /*
     * convert last time_t into tm from
     * last check if last equal zero (first run condition)
     */
    if ( last != 0 ) {
        localtime_r( &last, &last_info );
    }
    /*
     * Time:
     * only update while time changes or force ist set
     * Display has a minute resolution
     */
    if ( last == 0 || info.tm_min != last_info.tm_min || info.tm_hour != last_info.tm_hour || force ) {
        timesync_get_current_timestring( time_str, sizeof(time_str) );
        log_d("renew time: %s", time_str );
        lv_label_set_text( timelabel, time_str );
        lv_obj_align( timelabel, clock_cont, LV_ALIGN_CENTER, 0, 0 );
        /*
         * Date:
         * only update while date changes
         */
        if ( last == 0 || info.tm_yday != last_info.tm_yday ) {
            strftime( time_str, sizeof(time_str), "%a %d.%b %Y", &info );
            log_d("renew date: %s", time_str );
            lv_label_set_text( datelabel, time_str );
            lv_obj_align( datelabel, clock_cont, LV_ALIGN_IN_BOTTOM_MID, 0, 0 );
        }
        /*
         * Save for next loop
         */
        last = now;
    }
}

void main_tile_update_task( lv_task_t * task ) {
    /*
     * check if maintile alread initialized
     */
    if ( !maintile_init ) {
        log_e("maintile not initialized");
        return;
    }

    main_tile_update_time( false );
}
