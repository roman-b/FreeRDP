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
#include <freerdp/utils/hexdump.h>
#include "rail.h"
#include "stream.h"

#define LOG_LEVEL 11
#define LLOG(_level, _args) \
  do { if (_level < LOG_LEVEL) { printf _args ; } } while (0)
#define LLOGLN(_level, _args) \
  do { if (_level < LOG_LEVEL) { printf _args ; printf("\n"); } } while (0)


//------------------------------------------------------------------------------
void
in_rail_unicode_string(STREAM s, RAIL_UNICODE_STRING * string)
{
	in_uint16_le(s, string->length);

	string->buffer = NULL;
	if (string->length > 0)
	{
		string->buffer = xmalloc(string->length);
		in_uint8a(s, string->buffer, string->length);
	}
}
//------------------------------------------------------------------------------
void
free_rail_unicode_string(RAIL_UNICODE_STRING * string)
{
	if (string->buffer != NULL)
	{
		xfree(string->buffer);
		string->buffer = NULL;
		string->length = 0;
	}
}
//------------------------------------------------------------------------------
void
in_rail_rect_16(STREAM s, RAIL_RECT_16 * rect)
{
	in_uint16_le(s, rect->left); /*Left*/
	in_uint16_le(s, rect->top); /*Top*/
	in_uint16_le(s, rect->right); /*Right*/
	in_uint16_le(s, rect->bottom); /*Bottom*/
}
//------------------------------------------------------------------------------
static void
in_rail_cached_icon_info(STREAM s, RAIL_CACHED_ICON_INFO * cached_info)
{
	in_uint16_le(s, cached_info->cache_entry_id); /*CacheEntry*/
	in_uint8(s, cached_info->cache_id); /*CacheId*/
}
//------------------------------------------------------------------------------
static void
in_rail_icon_info(STREAM s, RAIL_ICON_INFO * icon_info)
{
	in_rail_cached_icon_info(s, &icon_info->cache_info);

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

	LLOGLN(10, ("ICON_INFO: cache_entry_id=0x%X cache_id=0x%X bpp=%d width=%d "
			"height=%d color_table_size=%d bits_mask_size=%d bits_color_size=%d",
			icon_info->cache_info.cache_entry_id,
			icon_info->cache_info.cache_id,
			icon_info->bpp,
			icon_info->width,
			icon_info->height,
			icon_info->color_table_size,
			icon_info->bits_mask_size,
			icon_info->bits_color_size
			));
}
//------------------------------------------------------------------------------
/* Process a Window Information Orders*/
static void
process_window_information(RAIL_SESSION* rail_session, STREAM s,
		uint32 fields_present_flags)
{
	uint32 window_id = 0;
	int new_window_flag = 0;
	RAIL_WINDOW_INFO window_info;

	in_uint32_le(s, window_id); /*WindowId*/

	new_window_flag = (fields_present_flags & WINDOW_ORDER_STATE_NEW) ? 1 : 0;

	LLOGLN(10, ("WINDOW_ORDER_TYPE_WINDOW order (wnd_id=0x%X is_new=%d)", window_id,
			new_window_flag));

	/* process Deleted Window order*/
	if (fields_present_flags & WINDOW_ORDER_STATE_DELETED)
	{
		/*TODO:
		 * - call rail_handle_window_deletion(window_id);*/
		LLOGLN(10, ("WINDOW_ORDER_STATE_DELETED event."));

		return;
	}


	/* process Cached Icon order*/
	if (fields_present_flags & WINDOW_ORDER_CACHEDICON)
	{
		RAIL_CACHED_ICON_INFO cached_info = {0};

		in_rail_cached_icon_info(s, &cached_info);
		LLOGLN(10, ("WINDOW_ORDER_CACHEDICON event.(cache_entry_id=0x%X cache_id=0x%X)",
				cached_info.cache_entry_id,
				cached_info.cache_id
				));


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
		uint8 is_icon_big = ((fields_present_flags & WINDOW_ORDER_FIELD_ICON_BIG) != 0);

		LLOGLN(10, ("WINDOW_ORDER_ICON event. (is_icon_big=%d)", is_icon_big));
		in_rail_icon_info(s, &icon_info);

		/*TODO:
		 * call rail_handle_window_icon(
		 	 	 window_id,
		 	 	 new_window_flag,
		 	 	 (fields_present_flags & WINDOW_ORDER_FIELD_ICON_BIG) ? 1 : 0,
		 	 	 &icon_info);
		 */
		return;
	}

	LLOGLN(10, ("WINDOW_ORDER_WINDOW event for new or existing window."));


	/* Otherwise process New or Existing Window order*/

	/*OwnerWindowId*/
	window_info.owner_window_id = 0;
	if (fields_present_flags & WINDOW_ORDER_FIELD_OWNER)
	{
		in_uint32_le(s, window_info.owner_window_id);
		LLOGLN(10, ("WINDOW_ORDER_FIELD_OWNER exists. (owner_window_id=0x%X).",
				window_info.owner_window_id));
	}

	/*Style*/
	window_info.style = 0;
	window_info.extened_style = 0;
	if (fields_present_flags & WINDOW_ORDER_FIELD_STYLE)
	{
		in_uint32_le(s, window_info.style);
		in_uint32_le(s, window_info.extened_style);

		LLOGLN(10, ("WINDOW_ORDER_FIELD_STYLE exists. (style=0x%X style_ex=0x%X).",
				window_info.owner_window_id,
				window_info.extened_style
				));

	}

	/*ShowState*/
	window_info.show_state = 0;
	if (fields_present_flags & WINDOW_ORDER_FIELD_SHOW)
	{
		in_uint8(s, window_info.show_state);
		LLOGLN(10, ("WINDOW_ORDER_FIELD_SHOW exists. (state=%d).",
				window_info.show_state
				));
	}

	/*TitleInfo*/
	window_info.title_info.length = 0;
	window_info.title_info.buffer = NULL;
	if (fields_present_flags & WINDOW_ORDER_FIELD_TITLE)
	{
		// TODO: add 520 bytes limit for title
		in_rail_unicode_string(s, &window_info.title_info);

		LLOGLN(10, ("WINDOW_ORDER_FIELD_TITLE exists. (title length=%d) UNICODE string dump:",
				window_info.title_info.length
				));
		freerdp_hexdump(window_info.title_info.buffer, window_info.title_info.length);
	}

	/* ClientOffsetX/ClientOffsetY */
	window_info.client_offset_x = 0;
	window_info.client_offset_y = 0;
	if (fields_present_flags & WINDOW_ORDER_FIELD_CLIENTAREAOFFSET)
	{
		in_uint32_le(s, window_info.client_offset_x);
		in_uint32_le(s, window_info.client_offset_y);

		LLOGLN(10, ("WINDOW_ORDER_FIELD_CLIENTAREAOFFSET exists. (cl_off_x=%d cl_off_y=%d).",
				window_info.client_offset_x,
				window_info.client_offset_y
				));
	}

	/* ClientAreaWidth/ClientAreaHeight */
	window_info.client_area_width = 0;
	window_info.client_area_height = 0;
	if (fields_present_flags & WINDOW_ORDER_FIELD_CLIENTAREASIZE)
	{
		in_uint32_le(s, window_info.client_area_width);
		in_uint32_le(s, window_info.client_area_height);

		LLOGLN(10, ("WINDOW_ORDER_FIELD_CLIENTAREASIZE exists. (cl_area_w=%d cl_area_h=%d).",
				window_info.client_area_width,
				window_info.client_area_height
				));
	}

	/* RPContent */
	window_info.rp_content = 0;
	if (fields_present_flags & WINDOW_ORDER_FIELD_RPCONTENT)
	{
		in_uint8(s, window_info.rp_content);
		LLOGLN(10, ("WINDOW_ORDER_FIELD_RPCONTENT exists. (rp_content=%d).",
				window_info.rp_content
				));
	}

	/* RootParentHandle */
	window_info.root_parent_handle = 0;
	if (fields_present_flags & WINDOW_ORDER_FIELD_ROOTPARENT)
	{
		in_uint32_le(s, window_info.root_parent_handle);
		LLOGLN(10, ("WINDOW_ORDER_FIELD_ROOTPARENT exists. (root_handle=0x%X).",
				window_info.root_parent_handle
				));
	}

	/* WindowOffsetX/WindowOffsetY */
	window_info.window_offset_x = 0;
	window_info.window_offset_y = 0;
	if (fields_present_flags & WINDOW_ORDER_FIELD_WNDOFFSET)
	{
		in_uint32_le(s, window_info.window_offset_x);
		in_uint32_le(s, window_info.window_offset_y);

		LLOGLN(10, ("WINDOW_ORDER_FIELD_WNDOFFSET exists. (offset_x=%d offset_y=%d).",
				window_info.window_offset_x,
				window_info.window_offset_y
				));

	}

	/* WindowClientDeltaX/WindowClientDeltaY */
	window_info.window_client_delta_x = 0;
	window_info.window_client_delta_y = 0;
	if (fields_present_flags & WINDOW_ORDER_FIELD_WNDCLIENTDELTA)
	{
		in_uint32_le(s, window_info.window_client_delta_x);
		in_uint32_le(s, window_info.window_client_delta_y);

		LLOGLN(10, ("WINDOW_ORDER_FIELD_WNDCLIENTDELTA exists. (cl_delta_x=%d cl_delta_y=%d).",
				window_info.window_client_delta_x,
				window_info.window_client_delta_y
				));
	}

	/* WindowWidth/WindowHeight */
	window_info.window_width = 0;
	window_info.window_height = 0;
	if (fields_present_flags &  WINDOW_ORDER_FIELD_WNDSIZE)
	{
		in_uint32_le(s, window_info.window_width);
		in_uint32_le(s, window_info.window_height);

		LLOGLN(10, ("WINDOW_ORDER_FIELD_WNDSIZE exists. (window_w=%d window_h=%d).",
				window_info.window_width,
				window_info.window_height
				));
	}

	/* NumWindowRects and WindowRects */
	window_info.window_rects_number = 0;
	window_info.window_rects = NULL;
	if (fields_present_flags &  WINDOW_ORDER_FIELD_WNDRECTS)
	{
		int i = 0;

		in_uint16_le(s, window_info.window_rects_number);

		LLOGLN(10, ("WINDOW_ORDER_FIELD_WNDRECTS exists. "
				"(window_rects_number=%d).",
				window_info.window_rects_number
				));

		window_info.window_rects = (RAIL_RECT_16*)xmalloc(
				window_info.window_rects_number * sizeof(RAIL_RECT_16));

		for (i = 0; i < window_info.window_rects_number; i++)
		{
			in_rail_rect_16(s, &window_info.window_rects[i]);

			LLOGLN(10, ("WINDOW_ORDER_FIELD_WNDRECTS exists. "
					"(rectN=%d left=%d top=%d right=%d bottom=%d).",
					i+1,
					window_info.window_rects[i].left,
					window_info.window_rects[i].top,
					window_info.window_rects[i].right,
					window_info.window_rects[i].bottom
					));
		}
	}

	/* VisibleOffsetX/VisibleOffsetY */
	window_info.visible_offset_x = 0;
	window_info.visible_offset_y = 0;
	if (fields_present_flags &  WINDOW_ORDER_FIELD_VISOFFSET)
	{
		in_uint32_le(s, window_info.visible_offset_x);
		in_uint32_le(s, window_info.visible_offset_y);

		LLOGLN(10, ("WINDOW_ORDER_FIELD_VISOFFSET exists. "
				"(visible_offset_x=%d visible_offset_y=%d).",
				window_info.visible_offset_x,
				window_info.visible_offset_y
				));
	}

	/* NumVisibilityRects and VisibilityRects */
	window_info.visibility_rects_number = 0;
	window_info.visibility_rects = NULL;
	if (fields_present_flags &  WINDOW_ORDER_FIELD_VISIBILITY)
	{
		int i = 0;

		in_uint16_le(s, window_info.visibility_rects_number);

		LLOGLN(10, ("WINDOW_ORDER_FIELD_VISIBILITY exists. "
				"(visibility_rects_number=%d).",
				window_info.visibility_rects_number
				));

		window_info.visibility_rects = (RAIL_RECT_16*)xmalloc(
				window_info.visibility_rects_number * sizeof(RAIL_RECT_16));

		for (i = 0; i < window_info.visibility_rects_number; i++)
		{
			in_rail_rect_16(s, &window_info.visibility_rects[i]);
			LLOGLN(10, ("WINDOW_ORDER_FIELD_VISIBILITY exists. "
					"(rectN=%d left=%d top=%d right=%d bottom=%d).",
					i+1,
					window_info.visibility_rects[i].left,
					window_info.visibility_rects[i].top,
					window_info.visibility_rects[i].right,
					window_info.visibility_rects[i].bottom
					));

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
in_rail_notify_icon_infotip(
		STREAM s,
		RAIL_NOTIFY_ICON_INFOTIP * icon_infotip
		)
{
	in_uint32_le(s, icon_infotip->timeout); /*Timeout*/
	in_uint32_le(s, icon_infotip->info_flags); /*InfoFlags*/

	in_rail_unicode_string(s, &icon_infotip->info_tip_text);/*InfoTipText*/
	in_rail_unicode_string(s, &icon_infotip->title);/*Title*/
}
//------------------------------------------------------------------------------
/* Process a Notification Icon Information orders*/
static void
process_notification_icon_information(
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

	LLOGLN(10, ("WINDOW_ORDER_TYPE_NOTIFY order (wnd_id=0x%X notify_icon_id=0x%X is_new=%d)",
			window_id,
			notify_icon_id,
			new_notify_icon_flag
			));


	if (fields_present_flags &  WINDOW_ORDER_STATE_DELETED)
	{
		/*TODO:
		 rail_handle_notification_icon_deleted(
			 	 	 window_id,
			 	 	 notify_icon_id)
		 */
		LLOGLN(10, ("WINDOW_ORDER_STATE_DELETED event."));

		return;
	}

	/* Reading New or Existing Notification Icons order*/

	/* Version */
	if (fields_present_flags &  WINDOW_ORDER_FIELD_NOTIFY_VERSION)
	{
		in_uint32_le(s, notify_icon_info.version);
		LLOGLN(10, ("WINDOW_ORDER_FIELD_NOTIFY_VERSION exists.(version=%d).",
				notify_icon_info.version
				));
	}

	/*ToolTip*/
	if (fields_present_flags &  WINDOW_ORDER_FIELD_NOTIFY_TIP)
	{
		in_rail_unicode_string(s, &notify_icon_info.tool_tip);

		LLOGLN(10, ("WINDOW_ORDER_FIELD_NOTIFY_TIP exists.(length=%d dump).",
				notify_icon_info.tool_tip.length
				));
		freerdp_hexdump(notify_icon_info.tool_tip.buffer,
				notify_icon_info.tool_tip.length
				);
	}

	/*InfoTip*/
	if (fields_present_flags &  WINDOW_ORDER_FIELD_NOTIFY_INFO_TIP)
	{
		// TODO:
		// - add 510 byte limit for InfoTipText
		// - add 126 byte limit for Title
		in_rail_notify_icon_infotip(s, &notify_icon_info.info_tip);

		LLOGLN(10, ("WINDOW_ORDER_FIELD_NOTIFY_INFO_TIP exists.("
				"timeout=%d info_flags=0x%X info_tip_length=%d title_length=%d).",
				notify_icon_info.info_tip.timeout,
				notify_icon_info.info_tip.info_flags,
				notify_icon_info.info_tip.info_tip_text.length,
				notify_icon_info.info_tip.title.length
				));
		freerdp_hexdump(notify_icon_info.info_tip.info_tip_text.buffer,
				notify_icon_info.info_tip.info_tip_text.length
				);
		freerdp_hexdump(notify_icon_info.info_tip.title.buffer,
				notify_icon_info.info_tip.title.length
				);
	}

	/*State*/
	if (fields_present_flags & WINDOW_ORDER_FIELD_NOTIFY_STATE)
	{
		in_uint32_le(s, notify_icon_info.state);
		LLOGLN(10, ("WINDOW_ORDER_FIELD_NOTIFY_STATE exists.(state=%d).",
				notify_icon_info.state
				));
	}

	/*Icon*/
	if (fields_present_flags &  WINDOW_ORDER_ICON)
	{
		LLOGLN(10, ("WINDOW_ORDER_ICON exists."));
		in_rail_icon_info(s, &notify_icon_info.icon);
	}

	/*CachedIcon*/
	if (fields_present_flags &  WINDOW_ORDER_CACHEDICON)
	{
		in_rail_cached_icon_info(s, &notify_icon_info.cached_icon);
		LLOGLN(10, ("WINDOW_ORDER_CACHEDICON exists.(entry_id=0x%X cache_id=0x%X)",
			notify_icon_info.cached_icon.cache_entry_id,
			notify_icon_info.cached_icon.cache_id
			));
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
process_desktop_information(
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

	LLOGLN(10, ("WINDOW_ORDER_TYPE_DESKTOP order"));


	/*Non-Monitored Desktop*/
	if (fields_present_flags &  WINDOW_ORDER_FIELD_DESKTOP_NONE)
	{
		LLOGLN(10, ("WINDOW_ORDER_FIELD_DESKTOP_NONE event."));
		return;
	}

	if (fields_present_flags &  WINDOW_ORDER_FIELD_DESKTOP_HOOKED)
	{
		LLOGLN(10, ("WINDOW_ORDER_FIELD_DESKTOP_HOOKED exists."));
		desktop_hooked = 1;
	}

	if (fields_present_flags &  WINDOW_ORDER_FIELD_DESKTOP_ARC_BEGAN)
	{
		LLOGLN(10, ("WINDOW_ORDER_FIELD_DESKTOP_ARC_BEGAN exists."));
		desktop_arc_began = 1;
	}

	if (fields_present_flags &  WINDOW_ORDER_FIELD_DESKTOP_ARC_COMPLETED)
	{
		LLOGLN(10, ("WINDOW_ORDER_FIELD_DESKTOP_ARC_COMPLETED exists."));
		desktop_arc_completed = 1;
	}

	if (fields_present_flags &  WINDOW_ORDER_FIELD_DESKTOP_ACTIVEWND)
	{
		in_uint32_le(s, active_window_id);
		LLOGLN(10, ("WINDOW_ORDER_FIELD_DESKTOP_ACTIVEWND exists.(active_window_id=0x%X)",
				active_window_id
				));
	}

	if (fields_present_flags &  WINDOW_ORDER_FIELD_DESKTOP_ZORDER)
	{
		int i = 0;

		in_uint8(s, window_ids_number);
		window_ids = (uint32*)xmalloc(window_ids_number * sizeof(uint32));

		LLOGLN(10, ("WINDOW_ORDER_FIELD_DESKTOP_ZORDER exists: number=%d.",
				window_ids_number));

		for (i = 0; i < window_ids_number; i++)
		{
			in_uint32_le(s, window_ids[i]);
			LLOGLN(10, ("WINDOW_ORDER_FIELD_DESKTOP_ZORDER exists: window_id[%d]=0x%X.",
					i+1,
					window_ids[i]
					));
		}
	}

	/*TODO: create desktop information handlers*/
}
//------------------------------------------------------------------------------
/* Process a Windowing Alternate Secondary Drawing Order*/
void
rail_on_altsec_window_order_received(
		RAIL_SESSION * rail_session,
		uint32 fields_present_flags,
		void* data,
		size_t order_size
		)
{
	struct stream s_stream = {0};
	STREAM        s = &s_stream;

	stream_init_by_allocated_data(s, data, order_size);

	if (fields_present_flags & WINDOW_ORDER_TYPE_WINDOW)
	{
		process_window_information(rail_session, s, fields_present_flags);
	}
	else if (fields_present_flags & WINDOW_ORDER_TYPE_NOTIFY)
	{
		process_notification_icon_information(rail_session, s, fields_present_flags);
	}
	else if (fields_present_flags & WINDOW_ORDER_TYPE_DESKTOP)
	{
		process_desktop_information(rail_session, s, fields_present_flags);
	}
	else
	{
		ui_unimpl(rail_session->rdp->inst,
				"windowing order (FieldsPresentFlags=0x%X)\n",
				fields_present_flags);
	}
}
