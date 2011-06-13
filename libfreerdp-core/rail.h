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
 * - add core virtual channel
 * - add RAIL order handlers
 * - add establishing RAIL connection ([MS-RDPERP] 1.3.2.1)
 */


#ifndef __RAIL_H
#define	__RAIL_H

#include <freerdp/vchan.h>
#include "rdp.h"


typedef struct _RAIL_UNICODE_STRING
{
	uint16  length;
	uint8	*buffer;
} RAIL_UNICODE_STRING;

typedef struct _RAIL_CACHED_ICON_INFO
{
	uint8 	cache_id;
	uint16 	cache_entry_id;
} RAIL_CACHED_ICON_INFO;

typedef struct _RAIL_ICON_INFO
{
	RAIL_CACHED_ICON_INFO cache_info;

	uint8  bpp;
	uint16 width;
	uint16 height;
	uint16 color_table_size;
	uint16 bits_mask_size;
	uint16 bits_color_size;

	uint8 *color_table;
	uint8 *bits_mask;
	uint8 *bits_color;

} RAIL_ICON_INFO;

typedef struct _RAIL_RECT_16
{
	uint16 left;
	uint16 top;
	uint16 right;
	uint16 bottom;

} RAIL_RECT_16;

typedef struct _RAIL_WINDOW_INFO
{
	RAIL_UNICODE_STRING title_info;

	uint32	owner_window_id;
	uint32	style;
	uint32	extened_style;
	uint8	show_state;

	uint32	client_offset_x;
	uint32	client_offset_y;

	uint32	client_area_width;
	uint32	client_area_height;
	uint8	rp_content;
	uint32	root_parent_handle;

	uint32	window_offset_x;
	uint32	window_offset_y;

	uint32	window_client_delta_x;
	uint32	window_client_delta_y;
	uint32	window_width;
	uint32	window_height;

	uint32			window_rects_number;
	RAIL_RECT_16* 	window_rects;

	uint32	visible_offset_x;
	uint32	visible_offset_y;

	uint32			visibility_rects_number;
	RAIL_RECT_16* 	visibility_rects;

} RAIL_WINDOW_INFO;

typedef struct _RAIL_NOTIFY_ICON_INFOTIP
{
	uint32 timeout;
	uint32 info_flags;

	RAIL_UNICODE_STRING info_tip_text;
	RAIL_UNICODE_STRING title;

} RAIL_NOTIFY_ICON_INFOTIP;

typedef struct _RAIL_NOTIFY_ICON_INFO
{
	uint32 version;
	uint32 state;

	RAIL_UNICODE_STRING 		tool_tip;
	RAIL_NOTIFY_ICON_INFOTIP 	info_tip;
	RAIL_ICON_INFO 				icon;
	RAIL_CACHED_ICON_INFO   	cached_icon;

} RAIL_NOTIFY_ICON_INFO;

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

int RailCoreVirtualChannelEntry(PCHANNEL_ENTRY_POINTS pEntryPoints);



#endif	// __RAIL_H
