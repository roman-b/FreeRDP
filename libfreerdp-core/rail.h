/*
   FreeRDP: A Remote Desktop Protocol client.
   Remote Applications Integrated Locally (RAIL)

   Copyright 2009 Marc-Andre Moreau <marcandre.moreau@gmail.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

/* TODO:
 * - add session termination if server does not support session
 * - add orders receiving
 * - add core virtual channel
 * - add establishing RAIL connection ([MS-RDPERP] 1.3.2.1)
 */


#ifndef __RAIL_H
#define	__RAIL_H

#include "rdp.h"

typedef struct _RD_UNICODE_STRING
{
	uint16  length;
	uint16	*buffer;
} RD_UNICODE_STRING;

typedef struct _RAIL_CACHED_ICON_INFO
{
	size_t cache_id;
	size_t cache_entry_id;
} RAIL_CACHED_ICON_INFO;

typedef struct _RAIL_ICON_INFO
{
	RAIL_CACHED_ICON_INFO cache_info;

	size_t bpp;
	size_t width;
	size_t height;
	size_t color_table_size;
	size_t bits_mask_image_size;
	size_t bits_color_image_size;

	uint8 *color_table;
	uint8 *bits_mask;
	uint8 *bits_color;

} RAIL_ICON_INFO;

typedef struct _RAIL_WINDOW_INFO
{

} RAIL_WINDOW_INFO;

struct rdp_rail
{
	struct rdp_rdp * rdp;

	int  rail_mode_supported;
	int  docked_langbar_supported;
	int  window_level_supported;
	int  window_level_ex_supported;

	size_t  number_icon_caches;
	size_t  number_icon_cache_entries;
};
typedef struct rdp_rail rdpRail;

rdpRail *
rail_new(struct rdp_rdp * rdp);

void
rail_free(rdpRail * rail);


void
rdp_send_client_execute_pdu(rdpRdp * rdp);


#endif	// __RAIL_H
