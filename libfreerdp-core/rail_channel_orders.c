/*
   FreeRDP: A Remote Desktop Protocol client.
   Remote Applications Integrated Locally (RAIL)

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
#include "secure.h"
#include <freerdp/rdpset.h>
#include <freerdp/utils/memory.h>

#include "rail.h"


/*
 * RAIL_PDU_HEADER
 * begin
 *   orderType   = 2 bytes
 *   orderLength = 2 bytes
   end
 */

static size_t RAIL_PDU_HEADER_SIZE = 4;

//------------------------------------------------------------------------------
void*
rail_alloc_order_data(
		size_t length
		)
{
	uint8 * order_start = xmalloc(length + RAIL_PDU_HEADER_SIZE);
	return (order_start + RAIL_PDU_HEADER_SIZE);
}
//------------------------------------------------------------------------------
void out_rail_unicode_string(STREAM s, RAIL_UNICODE_STRING* string)
{
	out_uint16_le(s, string->length);
	if (string->length > 0)
	{
		out_uint8a(s, string->buffer, string->length);
	}
}
//------------------------------------------------------------------------------
void out_rail_rect_16(STREAM s, RAIL_RECT_16* rect)
{
	out_uint16_le(s, rect->left); /*Left*/
	out_uint16_le(s, rect->top); /*Top*/
	out_uint16_le(s, rect->right); /*Right*/
	out_uint16_le(s, rect->bottom); /*Bottom*/
}
//------------------------------------------------------------------------------
// Used by 'rail_send_vchannel_' routines for sending constructed RAIL PDU to
// the 'rail' channel
void
rail_send_vchannel_order_data(
		RAIL_SESSION * session,
		uint16 order_type,
		void*  allocated_order_data,
		uint16 data_length
		)
{
	struct stream st_stream = {0};
	STREAM s = &st_stream;
	uint8* header_start = ((uint8*)allocated_order_data - RAIL_PDU_HEADER_SIZE);

	data_length += RAIL_PDU_HEADER_SIZE;

	stream_init_by_allocated_data(s, header_start, RAIL_PDU_HEADER_SIZE);

	out_uint16_le(s, order_type);
	out_uint16_le(s, data_length);

	session->channel_sender.send_rail_vchannel_data(
			session->channel_sender.sender_object,
			header_start, data_length);

	// In there we free memory which we allocated in rail_alloc_order_data(..)
	xfree(header_start);
}
//------------------------------------------------------------------------------
/*
 * The Handshake PDU is exchanged between the server and the client to
 * establish that both endpoints are ready to begin RAIL mode.
 * The server sends the Handshake PDU and the client responds
 * with the Handshake PDU.
 */
void
rail_send_vchannel_handshake_order(
		RAIL_SESSION * session,
		uint32 build_number
		)
{
	struct stream st_stream = {0};
	STREAM s = &st_stream;
	uint16 data_length = 4;
	void*  data = rail_alloc_order_data(data_length);

	stream_init_by_allocated_data(s, data, data_length);

	out_uint32_le(s, build_number);

	rail_send_vchannel_order_data(session, RDP_RAIL_ORDER_HANDSHAKE, data,
			data_length);
}
//------------------------------------------------------------------------------
/*
 * The Client Activate PDU is sent from client to server
 * when a local RAIL window on the client is activated or deactivated.
 */
void
rail_send_vchannel_activate_order(
		RAIL_SESSION * session,
		uint32 window_id,
		uint8 enabled
		)
{
	struct stream st_stream = {0};
	STREAM s = &st_stream;
	uint16 data_length = 5;
	void*  data = rail_alloc_order_data(data_length);

	stream_init_by_allocated_data(s, data, data_length);

	out_uint32_le(s, window_id);
	out_uint8(s, enabled);

	rail_send_vchannel_order_data(session, RDP_RAIL_ORDER_ACTIVATE, data,
			data_length);
}
//------------------------------------------------------------------------------
/*
 * Indicates a Client Execute PDU from client to server to request that a
 * remote application launch on the server.
 * */
void
rail_send_vchannel_exec_order(
		RAIL_SESSION * session,
		uint16 flags,
		RAIL_UNICODE_STRING* exe_or_file,
		RAIL_UNICODE_STRING* working_directory,
		RAIL_UNICODE_STRING* arguments
		)
{
	struct stream st_stream = {0};
	STREAM s = &st_stream;

	uint16 exe_or_file_length        = 2 + exe_or_file->length;
	uint16 working_directory_length  = 2 + working_directory->length;
	uint16 arguments_length          = 2 + arguments->length;

	size_t data_length =
		2 +                         /*Flags (2 bytes)*/
		2 +                         /*ExeOrFileLength (2 bytes)*/
		2 +                         /*WorkingDirLength (2 bytes)*/
		2 +                         /*ArgumentsLen (2 bytes)*/
		exe_or_file_length +        /*ExeOrFile (variable)*/
		working_directory_length +  /*WorkingDir (variable)*/
		arguments_length;           /*Arguments (variable):*/

	void*  data = rail_alloc_order_data(data_length);

	stream_init_by_allocated_data(s, data, data_length);

	out_uint16_le(s, flags);
	out_uint16_le(s, exe_or_file_length);
	out_uint16_le(s, working_directory_length);
	out_uint16_le(s, arguments_length);

	out_rail_unicode_string(s, exe_or_file);
	out_rail_unicode_string(s, working_directory);
	out_rail_unicode_string(s, arguments);

	rail_send_vchannel_order_data(session, RDP_RAIL_ORDER_EXEC, data,
			data_length);
}
//------------------------------------------------------------------------------
size_t get_sysparam_size_in_rdp_stream(RAIL_CLIENT_SYSPARAM * sysparam)
{
	switch (sysparam->type)
	{
	case SPI_SETDRAGFULLWINDOWS: {return 1;}
	case SPI_SETKEYBOARDCUES:    {return 1;}
	case SPI_SETKEYBOARDPREF:    {return 1;}
	case SPI_SETMOUSEBUTTONSWAP: {return 1;}
	case SPI_SETWORKAREA:        {return 8;}
	case RAIL_SPI_DISPLAYCHANGE: {return 8;}
	case RAIL_SPI_TASKBARPOS:    {return 8;}
	case SPI_SETHIGHCONTRAST:
		{
			return (4 + /*Flags (4 bytes)*/
					4 + /*ColorSchemeLength (4 bytes)*/
					2 + /*UNICODE_STRING.cbString (2 bytes)*/
					sysparam->value.high_contrast_system_info.color_scheme.length);
		}
	};

	ASSERT(!"Unknown sysparam type");
	return 0;
}
//------------------------------------------------------------------------------
/*
 * Indicates a Client System Parameters Update PDU from client to server to
 * synchronize system parameters on the server with those on the client.
 */
void
rail_send_vchannel_client_sysparam_update_order(
		RAIL_SESSION * session,
		RAIL_CLIENT_SYSPARAM* sysparam
		)
{
	struct stream st_stream = {0};
	STREAM s = &st_stream;
	size_t data_length = 4; /*SystemParam (4 bytes)*/
	void*  data = 0;

	data_length += get_sysparam_size_in_rdp_stream(sysparam);

	data = rail_alloc_order_data(data_length);
	stream_init_by_allocated_data(s, data, data_length);

	out_uint32_le(s, sysparam->type);

	switch (sysparam->type)
	{
	case SPI_SETDRAGFULLWINDOWS:
		out_uint8(s, sysparam->value.full_window_drag_enabled);
		break;

	case SPI_SETKEYBOARDCUES:
		out_uint8(s, sysparam->value.menu_access_key_always_underlined);
		break;

	case SPI_SETKEYBOARDPREF:
		out_uint8(s, sysparam->value.keyboard_for_user_prefered);
		break;

	case SPI_SETMOUSEBUTTONSWAP:
		out_uint8(s, sysparam->value.left_right_mouse_buttons_swapped);
		break;

	case SPI_SETWORKAREA:
		out_rail_rect_16(s, &sysparam->value.work_area);
		break;

	case RAIL_SPI_DISPLAYCHANGE:
		out_rail_rect_16(s, &sysparam->value.display_resolution);
		break;

	case RAIL_SPI_TASKBARPOS:
		out_rail_rect_16(s, &sysparam->value.taskbar_size);
		break;

	case SPI_SETHIGHCONTRAST:
		{
			uint32 color_scheme_length = 2 +
				sysparam->value.high_contrast_system_info.color_scheme.length;

			out_uint32_le(s, sysparam->value.high_contrast_system_info.flags);
			out_uint32_le(s, color_scheme_length);
			out_rail_unicode_string(s,
					&sysparam->value.high_contrast_system_info.color_scheme);
			break;
		}
	default:
		ASSERT(!"Unknown sysparam type");
	};

	rail_send_vchannel_order_data(session, RDP_RAIL_ORDER_SYSPARAM, data,
			data_length);
}
//------------------------------------------------------------------------------
/*
 * Indicates a Client System Command PDU from client to server when a local
 * RAIL window on the client receives a command to perform an action on the
 * window, such as minimize or maximize.
 */
void
rail_send_vchannel_syscommand_order(
		RAIL_SESSION * session,
		uint32 window_id,
		uint16 command
		)
{
	struct stream st_stream = {0};
	STREAM s = &st_stream;
	uint16 data_length = 4 + 2;
	void*  data = rail_alloc_order_data(data_length);

	stream_init_by_allocated_data(s, data, data_length);

	out_uint32_le(s, window_id);
	out_uint16_le(s, command);

	rail_send_vchannel_order_data(session, RDP_RAIL_ORDER_SYSCOMMAND, data,
			data_length);
}
//------------------------------------------------------------------------------
/*
 * The Client Notify Event PDU packet is sent from a client to a server when
 * a local RAIL Notification Icon on the client receives a keyboard or mouse
 * message from the user. This notification is forwarded to the server via
 * the Notify Event PDU.
 * */
void
rail_send_vchannel_notify_event_order(
		RAIL_SESSION * session,
		uint32 window_id,
		uint32 notify_icon_id,
		uint32 message
		)
{
	struct stream st_stream = {0};
	STREAM s = &st_stream;
	uint16 data_length = 4 * 3;
	void*  data = rail_alloc_order_data(data_length);

	stream_init_by_allocated_data(s, data, data_length);

	out_uint32_le(s, window_id);
	out_uint32_le(s, notify_icon_id);
	out_uint32_le(s, message);

	rail_send_vchannel_order_data(session, RDP_RAIL_ORDER_NOTIFY_EVENT, data,
			data_length);
}
//------------------------------------------------------------------------------
/*
 * The Client Window Move PDU packet is sent from the client to the server
 * when a local window is ending a move or resize. The client communicates the
 * locally moved or resized window's position to the server by using this packet.
 * The server uses this information to reposition its window.*/
void
rail_send_vchannel_client_windowmove_order(
		RAIL_SESSION * session,
		uint32 window_id,
		RAIL_RECT_16 * new_position
		)
{
	struct stream st_stream = {0};
	STREAM s = &st_stream;
	uint16 data_length = 4 + 2 * 4;
	void*  data = rail_alloc_order_data(data_length);

	stream_init_by_allocated_data(s, data, data_length);

	out_uint32_le(s, window_id);
	out_uint16_le(s, new_position->left);
	out_uint16_le(s, new_position->top);
	out_uint16_le(s, new_position->right);
	out_uint16_le(s, new_position->bottom);

	rail_send_vchannel_order_data(session, RDP_RAIL_ORDER_WINDOWMOVE, data,
			data_length);
}

//------------------------------------------------------------------------------
/*
 * The Client Information PDU is sent from client to server and contains
 * information about RAIL client state and features supported by the client.
 * */
void
rail_send_vchannel_client_information_order(
		RAIL_SESSION * session,
		uint32 flags
		)
{
	struct stream st_stream = {0};
	STREAM s = &st_stream;
	uint16 data_length = 4;
	void*  data = rail_alloc_order_data(data_length);

	stream_init_by_allocated_data(s, data, data_length);

	out_uint32_le(s, flags);

	rail_send_vchannel_order_data(session, RDP_RAIL_ORDER_CLIENTSTATUS, data,
			data_length);
}
//------------------------------------------------------------------------------
/*
 * The Client System Menu PDU packet is sent from the client to the server
 * when a local RAIL window on the client receives a command to display its
 * System menu. This command is forwarded to the server via
 * the System menu PDU.
 */
void
rail_send_vchannel_client_system_menu_order(
		RAIL_SESSION * session,
		uint32 window_id,
		uint16 left,
		uint16 top
		)
{
	struct stream st_stream = {0};
	STREAM s = &st_stream;
	uint16 data_length = 4 + 2 * 2;
	void*  data = rail_alloc_order_data(data_length);

	stream_init_by_allocated_data(s, data, data_length);

	out_uint32_le(s, window_id);
	out_uint16_le(s, left);
	out_uint16_le(s, top);

	rail_send_vchannel_order_data(session, RDP_RAIL_ORDER_SYSMENU, data,
			data_length);
}
//------------------------------------------------------------------------------
/*
 * The Language Bar Information PDU is used to set the language bar status.
 * It is sent from a client to a server or a server to a client, but only when
 * both support the Language Bar docking capability
 * (TS_RAIL_LEVEL_DOCKED_LANGBAR_SUPPORTED).
 * This PDU contains information about the language bar status.
 * */
void
rail_send_vchannel_client_langbar_information_order(
		RAIL_SESSION * session,
		uint32 langbar_status
		)
{
	struct stream st_stream = {0};
	STREAM s = &st_stream;
	uint16 data_length = 4;
	void*  data = rail_alloc_order_data(data_length);

	stream_init_by_allocated_data(s, data, data_length);

	out_uint32_le(s, langbar_status);

	rail_send_vchannel_order_data(session, RDP_RAIL_ORDER_LANGBARINFO, data,
			data_length);
}
//------------------------------------------------------------------------------
/*
 * The Client Get Application ID PDU is sent from a client to a server.
 * This PDU requests information from the server about the Application ID
 * that the window SHOULD <15> have on the client.
 * */
void
rail_send_vchannel_get_appid_req_order(
		RAIL_SESSION * session,
		uint32 window_id
		)
{
	struct stream st_stream = {0};
	STREAM s = &st_stream;
	uint16 data_length = 4;
	void*  data = rail_alloc_order_data(data_length);

	stream_init_by_allocated_data(s, data, data_length);

	out_uint32_le(s, window_id);

	rail_send_vchannel_order_data(session, RDP_RAIL_ORDER_GET_APPID_REQ, data,
			data_length);
}
//------------------------------------------------------------------------------
/*
 * Look at rail_send_vchannel_handshake_order(...)
 */
void
rail_process_vchannel_handshake_order(
		RAIL_SESSION* session,
		STREAM s
		)
{
	uint32 build_number = 0;

	in_uint32_le(s, build_number);
	// rail_handle_server_hadshake(session, build_number);
}
//------------------------------------------------------------------------------
/*
 * The Server Execute Result PDU is sent from server to client in response to
 * a Client Execute PDU request, and contains the result of the server's
 * attempt to launch the requested executable.
 */
void
rail_process_vchannel_exec_result_order(
		RAIL_SESSION* session,
		STREAM s
		)
{
	uint16 flags = 0;
	uint16 exec_result = 0;
	uint32 raw_result = 0;
	uint16 exe_or_file_length = 0;
	RAIL_UNICODE_STRING exe_or_file = {0};

	in_uint16_le(s, flags); /*Flags (2 bytes)*/
	in_uint16_le(s, exec_result); /*ExecResult (2 bytes)*/
	in_uint32_le(s, raw_result); /*RawResult (4 bytes)*/
	in_uint8s(s, 2);  /*Padding (2 bytes)*/
	in_uint16_le(s, exe_or_file_length); /*ExeOrFileLength (2 bytes)*/
	in_rail_unicode_string(s, &exe_or_file); /*ExeOrFile (variable)*/

	// rail_handle_exec_result(session, flags, exec_result, raw_result,);
	// free_rail_string(&exe_or_file);
}
//------------------------------------------------------------------------------
/*
 * The Server System Parameters Update PDU is sent from the server to client to
 * synchronize system parameters on the client with those on the server.
 */
void
rail_process_vchannel_server_sysparam_update_order(
		RAIL_SESSION* session,
		STREAM s
		)
{
	RAIL_SERVER_SYSPARAM sysparam = {0};

	in_uint32_le(s, sysparam.type);

	switch (sysparam.type)
	{
	case SPI_SETSCREENSAVEACTIVE:
		in_uint8(s, sysparam.value.screen_saver_enabled);
		break;

	case SPI_SETSCREENSAVESECURE:
		in_uint8(s, sysparam.value.screen_saver_lock_enabled);
		break;

	default:
		ASSERT(!"Undocumented RAIL server sysparam type");
		break;
	};

	// rail_handle_server_sysparam(session, sysparam);
}
//------------------------------------------------------------------------------
/*
 * The Server Move/Size Start PDU packet is sent by the server when a window on
 * the server is beginning a move or resize.
 * The client uses this information to initiate a local move or resize of the
 * corresponding local window.
 *
 * The Server Move/Size End PDU is sent by the server when a window on the
 * server is completing a move or resize.
 * The client uses this information to end a local move/resize of the
 * corresponding local window.
 *
 */
void
rail_process_vchannel_server_movesize_order(
		RAIL_SESSION* session,
		STREAM s
		)
{
	uint32 window_id = 0;
	uint16 move_size_started = 0;
	uint16 move_size_type = 0;
	uint16 pos_x = 0;
	uint16 pos_y = 0;

	in_uint32_le(s, window_id);
	in_uint16_le(s, move_size_started);
	in_uint16_le(s, move_size_type);
	in_uint16_le(s, pos_x);
	in_uint16_le(s, pos_y);

	// rail_handle_server_movesize(session, window_id, move_size_started,
	//    move_size_type, pos_x, pos_y);
}
//------------------------------------------------------------------------------
/*
 * The Server Min Max Info PDU is sent from a server to a client when a window
 * move or resize on the server is being initiated.
 * This PDU contains information about the minimum and maximum extents to
 * which the window can be moved or sized.
 */
void
rail_process_vchannel_server_minmax_info_order(
		RAIL_SESSION* session,
		STREAM s
		)
{
	uint32 window_id = 0;
	uint16 max_width = 0;
	uint16 max_height = 0;
	uint16 max_pos_x = 0;
	uint16 max_pos_y = 0;
	uint16 min_track_width = 0;
	uint16 min_track_height = 0;
	uint16 max_track_width = 0;
	uint16 max_track_height = 0;

	in_uint32_le(s, window_id);
	in_uint16_le(s, max_width);
	in_uint16_le(s, max_height);
	in_uint16_le(s, max_pos_x);
	in_uint16_le(s, max_pos_y);
	in_uint16_le(s, min_track_width);
	in_uint16_le(s, min_track_height);
	in_uint16_le(s, max_track_width);
	in_uint16_le(s, max_track_height);

	// rail_handle_server_minmax_info(session, window_id, max_width,
	//    max_height, max_pos_x, max_pos_y, min_track_width, min_track_height,
	//    max_track_width, max_track_height);
}
//------------------------------------------------------------------------------
/*
 *The Language Bar Information PDU is used to set the language bar status.
 */
void
rail_process_vchannel_server_langbar_info_order(
		RAIL_SESSION* session,
		STREAM s
		)
{
	uint32 langbar_status = 0;

	in_uint32_le(s, langbar_status);

	// rail_handle_server_langbar_info(session, langbar_status);
}
//------------------------------------------------------------------------------
/*
 * The Server Get Application ID Response PDU is sent from a server to a client.
 * This PDU MAY be sent to the client as a response to a Client Get Application
 * ID PDU. This PDU specifies the Application ID that the specified window
 * SHOULD have on the client. The client MAY ignore this PDU.
 */
void
rail_process_vchannel_server_get_appid_resp_order(
		RAIL_SESSION* session,
		STREAM s
		)
{
	uint32 window_id = 0;
	RAIL_UNICODE_STRING app_id = {0};

	app_id.length = 256;
	app_id.buffer = xmalloc(app_id.length);

	in_uint32_le(s, window_id);
	in_uint8a(s, app_id.buffer, app_id.length);

	// rail_handle_server_get_app_resp(session, window_id, app_id);
	// free_rail_string(&app_id);
}
//------------------------------------------------------------------------------
void
rail_channel_process_received_data(
		RAIL_SESSION * session,
		void*  data,
		size_t length
		)
{
	struct stream st_stream = {0};
	STREAM s = &st_stream;
	uint16 order_type = 0;
	uint16 order_length = 0;

	stream_init_by_allocated_data(s, data, length);

	out_uint16_le(s, order_type);   /* orderType */
	out_uint16_le(s, order_length); /* orderLength */

	//TODO: ASSERT((orderLength - 4) <= ((uint8*)s->p - (uint8*)s->data) );

	switch (order_type)
	{
	case RDP_RAIL_ORDER_HANDSHAKE:
		rail_process_vchannel_handshake_order(session, s);
		break;
	case RDP_RAIL_ORDER_EXEC_RESULT:
		rail_process_vchannel_exec_result_order(session, s);
		break;
	case RDP_RAIL_ORDER_SYSPARAM:
		rail_process_vchannel_server_sysparam_update_order(session, s);
		break;
	case RDP_RAIL_ORDER_LOCALMOVESIZE:
		rail_process_vchannel_server_movesize_order(session, s);
		break;
	case RDP_RAIL_ORDER_MINMAXINFO:
		rail_process_vchannel_server_minmax_info_order(session, s);
		break;
	case RDP_RAIL_ORDER_LANGBARINFO:
		rail_process_vchannel_server_langbar_info_order(session, s);
		break;
	case RDP_RAIL_ORDER_GET_APPID_RESP:
		rail_process_vchannel_server_get_appid_resp_order(session, s);
		break;
	}
	ASSERT(!"Undocumented RAIL channels server PDU order_type");
}
//------------------------------------------------------------------------------
void
rail_on_channel_data_received(
		RAIL_SESSION * rail_session,
		void*  data,
		size_t length
		)
{
	rail_channel_process_received_data(rail_session, data, length);
}

