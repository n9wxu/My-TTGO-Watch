/****************************************************************************
              config.h

    Tu May 22 21:23:51 2020
    Copyright  2020  Dirk Brosswick
 *  Email: dirk.brosswick@googlemail.com
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
#ifndef _CONFIG_H

    #if defined( LILYGO_WATCH_2020_V1 )
        #define WATCH_VERSION_NAME  "V1"
    #elif defined( LILYGO_WATCH_2020_V2 )
        #define WATCH_VERSION_NAME  "V2"
    #elif defined( LILYGO_WATCH_2020_V3 )
        #define WATCH_VERSION_NAME  "V3"
    #else
        #error "no ttgo t-watch 2020 version defined"
    #endif


    #define LILYGO_WATCH_LVGL                       /** @brief To use LVGL, you need to enable the macro LVGL */
    #define TWATCH_USE_PSRAM_ALLOC_LVGL             /** @brief enabled lillygo-lib to use PSRAM */ 
    /**
     * Built-in applications
     */
    #define ENABLE_WEBSERVER                        /** @brief To disable built-in webserver, comment this line */
    #define ENABLE_FTPSERVER                        /** @brief To disable built-in ftpserver, comment this line */
    /**
     * Enable non-latin languages support
     */
    #define USE_EXTENDED_CHARSET CHARSET_CYRILLIC
    /**
     * firmeware version string
     */
    #define __FIRMWARE__            "2021070601"
    /**
     * Allows to include config.h from C code
     */
    #ifdef __cplusplus
        #include <LilyGoWatch.h>
        #define _CONFIG_H 
    #endif

#endif // _CONFIG_H
