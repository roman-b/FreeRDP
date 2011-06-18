/*
   FreeRDP: A Remote Desktop Protocol client.
   Remote Applications Integrated Locally (RAIL)
   Processing Windowing Alternate Secondary Drawing Order

   Copyright 2009 Marc-Andre Moreau <marcandre.moreau@gmail.com>
   Copyright 2011 Roman Barabanov <romanbarabanov@gmail.com>

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

#include "frdp.h"
#include "rdp.h"
#include <freerdp/utils/memory.h>
#include "rail.h"
#include "stream.h"

//------------------------------------------------------------------------------
static void
rdp_in_rail_unicode_string(STREAM s, RAIL_UNICODE_STRING * string)
{
	in_uint16_le(s, string->length);

	string->buffer = xmalloc(string->length);
	in_uint8a(s, string->buffer, string->length);
}
//------------------------------------------------------------------------------
static void
rdp_in_rail_rect_16(STREAM s, RAIL_RECT_16 * rect)
{
	in_uint16_le(s, rect->left); /*Left*/
	in_uint16_le(s, rect->top); /*Top*/
	in_uint16_le(s, rect->right); /*Right*/
	in_uint16_le(s, rect->bottom); /*Bottom*/
}
//------------------------------------------------------------------------------
static void
rdp_in_rail_cached_icon_info(STREAM s, RAIL_CACHED_ICON_INFO * cached_info)
{
	in_uint16_le(s, cached_info->cache_entry_id); /*CacheEntry*/
	in_uint8(s, cached_info->cache_id); /*CacheId*/
}
//------------------------------------------------------------------------------
static void
rdp_in_rail_icon_info(STREAM s, RAIL_ICON_INFO * icon_info)
{
	rdp_in_rail_cached_icon_info(s, &icon_info->cache_info);

	in_uint8(s, icon_info->bpp); /*Bpp*/
	in_uint16_le(s, icon_info->width); /*Width*/
	in_uint16_le(s, icon_info->height); /*Height*/

	/* CbColorTable present ONLY if Bpp equal to 1, 4, 8*/
	icon_info->color_table_size = 0;
	if ((icon_info->bpp == 1) || (icon_info->bpp == 4) || (icon_info->bpp == 8))
	{
		in_uint16_le(s, icon_info->color_table_size); /*CbColorTable*/
	}

	in_uint16_le(s, icon_info->bits_mask_size); /*CbBitsMask*/
	in_uint16_le(s, icon_info->bits_color_size); /*CbBitsColor*/

	/* BitsMask*/
	icon_info->bits_mask = NULL;
	if (icon_info->bits_mask_size > 0)
	{
		icon_info->bits_mask = xmalloc(icon_info->bits_mask_size);
		in_uint8a(s, icon_info->bits_mask, icon_info->bits_mask_size);
	}

	/* ColorTable */
	icon_info->color_table = NULL;
	if (icon_info->color_table_size > 0)
	{
		icon_info->color_table = xmalloc(icon_info->color_table_size);
		in_uint8a(s, icon_info->color_table, icon_info->color_table_size);
	}

	/* BitsColor */
	icon_info->bits_color = NULL;
	if (icon_info->bits_color_size > 0)
	{
		icon_info->bits_color = xmalloc(icon_info->bits_color_size);
		in_uint8a(s, icon_info->bits_color, icon_info->bits_color_size);
	}
}
//------------------------------------------------------------------------------
/* Process a Window Information Orders*/
static void
process_windowing_window_information(RAIL_SESSION* rail_session, STREAM s,
		uint32 fields_present_flags)
{
	uint32 window_id = 0;
	int new_window_flag = 0;
	RAIL_WINDOW_INFO window_info;

	in_uint32_le(s, window_id); /*WindowId*/

	new_window_flag = (fields_present_flags & WINDOW_ORDER_STATE_NEW) ? 1 : 0;

	/* process Deleted Window order*/
	if (fields_present_flags & WINDOW_ORDER_STATE_DELETED)
	{
		/*TODO:
		 * - assert order_size and other flags
		 * - call rail_handle_window_deletion(window_id);*/
		return;
	}

	/* process Cached Icon order*/
	if (fields_present_flags & WINDOW_ORDER_CACHEDICON)
	{
		RAIL_CACHED_ICON_INFO cached_info = {0};

		rdp_in_rail_cached_icon_info(s, &cached_info);

		/*TODO:
		 * call rail_handle_window_cached_icon(window_id,
		 	 	 new_window_flag,
		 	 	 (fields_present_flags & WINDOW_ORDER_FIELD_ICON_BIG) ? 1 : 0,
		 	 	 &cached_info);
		 */
		return;
	}

	/* process Window Icon order*/
	if (fields_present_flags & WINDOW_ORDER_ICON)
	{
		RAIL_ICON_INFO icon_info = {.cache_info = {0}, 0};

		rdp_in_rail_icon_info(s, &icon_info);

		/*TODO:
		 * call rail_handle_window_icon(
		 	 	 window_id,
		 	 	 new_window_flag,
		 	 	 (fields_present_flags & WINDOW_ORDER_FIELD_ICON_BIG) ? 1 : 0,
		 	 	 &icon_info);
		 */
		return;
	}

	/* Otherwise process New or Existing Window order*/

	/*OwnerWindowId*/
	window_info.owner_window_id = 0;
	if (fields_present_flags & WINDOW_ORDER_FIELD_OWNER)
	{
		in_uint32_le(s, window_info.owner_window_id);
	}

	/*Style*/
	window_info.style = 0;
	window_info.extened_style = 0;
	if (fields_present_flags & WINDOW_ORDER_FIELD_STYLE)
	{
		in_uint32_le(s, window_info.style);
		in_uint32_le(s, window_info.extened_style);
	}

	/*ShowState*/
	window_info.show_state = 0;
	if (fields_present_flags & WINDOW_ORDER_FIELD_SHOW)
	{
		in_uint8(s, window_info.show_state);
	}

	/*TitleInfo*/
	window_info.title_info.length = 0;
	window_info.title_info.buffer = NULL;
	if (fields_present_flags & WINDOW_ORDER_FIELD_TITLE)
	{
		rdp_in_rail_unicode_string(s, &window_info.title_info);
	}

	/* ClientOffsetX/ClientOffsetY */
	window_info.client_offset_x = 0;
	window_info.client_offset_y = 0;
	if (fields_present_flags & WINDOW_ORDER_FIELD_CLIENTAREAOFFSET)
	{
		in_uint32_le(s, window_info.client_offset_x);
		in_uint32_le(s, window_info.client_offset_y);
	}

	/* ClientAreaWidth/ClientAreaHeight */
	window_info.client_area_width = 0;
	window_info.client_area_height = 0;
	if (fields_present_flags & WINDOW_ORDER_FIELD_CLIENTAREASIZE)
	{
		in_uint32_le(s, window_info.client_area_width);
		in_uint32_le(s, window_info.client_area_height);
	}

	/* RPContent */
	window_info.rp_content = 0;
	if (fields_present_flags & WINDOW_ORDER_FIELD_RPCONTENT)
	{
		in_uint8(s, window_info.rp_content);
	}

	/* RootParentHandle */
	window_info.root_parent_handle = 0;
	if (fields_present_flags & WINDOW_ORDER_FIELD_ROOTPARENT)
	{
		in_uint32_le(s, window_info.root_parent_handle);
	}

	/* WindowOffsetX/WindowOffsetY */
	window_info.window_offset_x = 0;
	window_info.window_offset_y = 0;
	if (fields_present_flags & WINDOW_ORDER_FIELD_WNDOFFSET)
	{
		in_uint32_le(s, window_info.window_offset_x);
		in_uint32_le(s, window_info.window_offset_y);
	}

	/* WindowClientDeltaX/WindowClientDeltaY */
	window_info.window_client_delta_x = 0;
	window_info.window_client_delta_y = 0;
	if (fields_present_flags & WINDOW_ORDER_FIELD_WNDCLIENTDELTA)
	{
		in_uint32_le(s, window_info.window_client_delta_x);
		in_uint32_le(s, window_info.window_client_delta_y);
	}

	/* WindowWidth/WindowHeight */
	window_info.window_width = 0;
	window_info.window_height = 0;
	if (fields_present_flags &  WINDOW_ORDER_FIELD_WNDSIZE)
	{
		in_uint32_le(s, window_info.window_width);
		in_uint32_le(s, window_info.window_height);
	}

	/* NumWindowRects and WindowRects */
	window_info.window_rects_number = 0;
	window_info.window_rects = NULL;
	if (fields_present_flags &  WINDOW_ORDER_FIELD_WNDRECTS)
	{
		int i = 0;

		in_uint32_le(s, window_info.window_rects_number);
		window_info.window_rects = (RAIL_RECT_16*)xmalloc(
				window_info.window_rects_number * sizeof(RAIL_RECT_16));

		for (i = 0; i < window_info.window_rects_number; i++)
		{
			rdp_in_rail_rect_16(s, &window_info.window_rects[i]);
		}
	}

	/* VisibleOffsetX/VisibleOffsetY */
	window_info.visible_offset_x = 0;
	window_info.visible_offset_y = 0;
	if (fields_present_flags &  WINDOW_ORDER_FIELD_VISOFFSET)
	{
		in_uint32_le(s, window_info.visible_offset_x);
		in_uint32_le(s, window_info.visible_offset_y);
	}

	/* NumVisibilityRects and VisibilityRects */
	window_info.visibility_rects_number = 0;
	window_info.visibility_rects = NULL;
	if (fields_present_flags &  WINDOW_ORDER_FIELD_VISIBILITY)
	{
		int i = 0;

		in_uint32_le(s, window_info.visibility_rects_number);
		window_info.visibility_rects = (RAIL_RECT_16*)xmalloc(
				window_info.visibility_rects_number * sizeof(RAIL_RECT_16));

		for (i = 0; i < window_info.visibility_rects_number; i++)
		{
			rdp_in_rail_rect_16(s, &window_info.visibility_rects[i]);
		}
	}

	/*TODO:
	 rail_handle_window_information(
		 	 	 window_id,
		 	 	 new_window_flag,
		 	 	 (fields_present_flags & WINDOW_ORDER_FIELD_ICON_BIG) ? 1 : 0,
		 	 	 &icon_info)
	 */
}
//------------------------------------------------------------------------------
static void
rdp_in_rail_notify_icon_infotip(
		STREAM s,
		RAIL_NOTIFY_ICON_INFOTIP * icon_infotip
		)
{
	in_uint32_le(s, icon_infotip->timeout); /*Timeout*/
	in_uint32_le(s, icon_infotip->info_flags); /*InfoFlags*/

	rdp_in_rail_unicode_string(s, &icon_infotip->info_tip_text);/*InfoTipText*/
	rdp_in_rail_unicode_string(s, &icon_infotip->title);/*Title*/
}
//------------------------------------------------------------------------------
/* Process a Notification Icon Information orders*/
static void
process_windowing_notification_icon_information(
		RAIL_SESSION* rail_session,
		STREAM s,
		uint32 fields_present_flags)
{
	uint32 window_id = 0;
	uint32 notify_icon_id = 0;
	int new_notify_icon_flag = 0;
	RAIL_NOTIFY_ICON_INFO notify_icon_info;

	in_uint32_le(s, window_id); /*WindowId*/
	in_uint32_le(s, notify_icon_id); /*NotifyIconId*/

	new_notify_icon_flag = (fields_present_flags & WINDOW_ORDER_STATE_NEW) ? 1 : 0;

	if (fields_present_flags &  WINDOW_ORDER_STATE_DELETED)
	{
		/*TODO:
		 rail_handle_notification_icon_deleted(
			 	 	 window_id,
			 	 	 notify_icon_id)
		 */
		return;
	}

	/* Reading New or Existing Notification Icons order*/

	/* Version */
	if (fields_present_flags &  WINDOW_ORDER_FIELD_NOTIFY_VERSION)
	{
		in_uint32_le(s, notify_icon_info.version);
	}

	/*ToolTip*/
	if (fields_present_flags &  WINDOW_ORDER_FIELD_NOTIFY_TIP)
	{
		rdp_in_rail_unicode_string(s, &notify_icon_info.tool_tip);
	}

	/*InfoTip*/
	if (fields_present_flags &  WINDOW_ORDER_FIELD_NOTIFY_INFO_TIP)
	{
		rdp_in_rail_notify_icon_infotip(s, &notify_icon_info.info_tip);
	}

	/*State*/
	if (fields_present_flags &  WINDOW_ORDER_FIELD_NOTIFY_STATE)
	{
		in_uint32_le(s, notify_icon_info.state);
	}

	/*Icon*/
	if (fields_present_flags &  WINDOW_ORDER_ICON)
	{
		rdp_in_rail_icon_info(s, &notify_icon_info.icon);
	}

	/*CachedIcon*/
	if (fields_present_flags &  WINDOW_ORDER_ICON)
	{
		rdp_in_rail_cached_icon_info(s, &notify_icon_info.cached_icon);
	}

	/*TODO:
	 rail_handle_notification_icon_information(
		 	 	 window_id,
		 	 	 notify_icon_id,
		 	 	 new_notify_icon_flag,
		 	 	 &notify_icon_info);
	 */
}
//------------------------------------------------------------------------------
/* Process a Desktop Information Orders*/
static void
process_windowing_desktop_information(
		RAIL_SESSION* rail_session,
		STREAM s,
		uint32 fields_present_flags
		)
{
	uint32 	active_window_id = 0;
	uint8 	window_ids_number = 0;
	uint32 	*window_ids = 0;
	int    	desktop_hooked = 0;
	int    	desktop_arc_began = 0;
	int    	desktop_arc_completed = 0;

	/*Non-Monitored Desktop*/
	if (fields_present_flags &  WINDOW_ORDER_FIELD_DESKTOP_NONE)
	{
		return;
	}

	if (fields_present_flags &  WINDOW_ORDER_FIELD_DESKTOP_HOOKED)
	{
		desktop_hooked = 1;
	}

	if (fields_present_flags &  WINDOW_ORDER_FIELD_DESKTOP_ARC_BEGAN)
	{
		desktop_arc_began = 1;
	}

	if (fields_present_flags &  WINDOW_ORDER_FIELD_DESKTOP_ARC_COMPLETED)
	{
		desktop_arc_completed = 1;
	}

	if (fields_present_flags &  WINDOW_ORDER_FIELD_DESKTOP_ACTIVEWND)
	{
		in_uint32_le(s, active_window_id);
	}

	if (fields_present_flags &  WINDOW_ORDER_FIELD_DESKTOP_ZORDER)
	{
		int i = 0;

		in_uint8(s, window_ids_number);
		window_ids = (uint32*)xmalloc(window_ids_number * sizeof(uint32));

		for (i = 0; i < window_ids_number; i++)
		{
			in_uint32_le(s, window_ids[i]);
		}
	}

	/*TODO: create desktop information handlers*/
}
//------------------------------------------------------------------------------
/* Process a Windowing Alternate Secondary Drawing Order*/
void
rail_on_altsec_window_order_received(
		RAIL_SESSION* rail_session,
		void* data,
		size_t length
		)
{
	struct stream s_stream = {0};
	STREAM        s = &s_stream;
	uint16 order_size;
	uint32 fields_present_flags;

	stream_init_by_allocated_data(s, data, length);

	in_uint16_le(s, order_size); /*OrderSize*/
	in_uint32_le(s, fields_present_flags); /*FieldsPresentFlags*/

	if (fields_present_flags & WINDOW_ORDER_TYPE_WINDOW)
	{
		process_windowing_window_information(rail_session, s, fields_present_flags);
	}
	else if (fields_present_flags & WINDOW_ORDER_TYPE_NOTIFY)
	{
		process_windowing_notification_icon_information(rail_session, s,
				fields_present_flags);
	}
	else if (fields_present_flags & WINDOW_ORDER_TYPE_DESKTOP)
	{
		process_windowing_desktop_information(rail_session, s, fields_present_flags);
	}
	else
	{
		ui_unimpl(rail_session->rdp->inst,
				"windowing order (FieldsPresentFlags=0x%X)\n",
				fields_present_flags);
	}
}
