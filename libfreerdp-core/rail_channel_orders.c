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

//------------------------------------------------------------------------------
// Used by 'rail_send_vchannel_' routines for sending constructed RAIL PDU to
// the 'rail' channel
void
rail_send_vchannel_data(
		RAIL_SESSION * session,
		void* data,
		size_t data_length)
{
	session->channel_sender.send_rail_vchannel_data(
			session->channel_sender.sender_object,
			data, data_length);
}
//------------------------------------------------------------------------------
/*
 * The Handshake PDU is exchanged between the server and the client to
 * establish that both endpoints are ready to begin RAIL mode.
 * The server sends the Handshake PDU and the client responds
 * with the Handshake PDU.
 */
void
rail_send_vchannel_handshake_order()
{
	//TS_RAIL_ORDER_HANDSHAKE
}
//------------------------------------------------------------------------------
/*
 * The Client Activate PDU is sent from client to server
 * when a local RAIL window on the client is activated or deactivated.
 */
void
rail_send_vchannel_activate_order()
{
	//TS_RAIL_ORDER_ACTIVATE
}
//------------------------------------------------------------------------------
/*
 * Indicates a Client Execute PDU from client to server to request that a
 * remote application launch on the server.
 * */
void
rail_send_vchannel_exec_order()
{
//	//TS_RAIL_ORDER_EXEC
//
//	STREAM s;
//	uint16 flags;
//	char *arguments;
//	char *application_name;
//	char *working_directory;
//	size_t arguments_len;
//	size_t application_name_len;
//	size_t working_directory_len;
//
//	/* Still lacking proper packet initialization */
//	s = NULL;
//	//rdp_out_rail_pdu_header(s, RDP_RAIL_ORDER_EXEC, 12);
//
//	s = stream_new(12);
//
//	out_uint16_le(s, (uint16)RDP_RAIL_ORDER_EXEC);
//	out_uint16_le(s, (uint16)12);
//
//
//
//	application_name = freerdp_uniconv_out(rdp->uniconv,
//			rdp->settings->rail_exe_or_file, &application_name_len);
//	working_directory = freerdp_uniconv_out(rdp->uniconv,
//			rdp->settings->rail_working_directory, &working_directory_len);
//	arguments = freerdp_uniconv_out(rdp->uniconv,
//			rdp->settings->rail_arguments, &arguments_len);
//
//	flags = RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY | RAIL_EXEC_FLAG_EXPAND_ARGUMENTS;
//
//	out_uint16_le(s, flags); /* flags */
//	out_uint16_le(s, application_name_len); /* ExeOrFileLength */
//	out_uint16_le(s, working_directory_len); /* WorkingDirLength */
//	out_uint16_le(s, arguments_len); /* ArgumentsLength */
//	out_uint8a(s, application_name, application_name_len + 2); /* ExeOrFile */
//	out_uint8a(s, working_directory, working_directory_len + 2); /* WorkingDir */
//	out_uint8a(s, arguments, arguments_len + 2); /* Arguments */
//
//	xfree(application_name);
//	xfree(working_directory);
//	xfree(arguments);
//
//	s_mark_end(s);
//	//sec_send(rdp->sec, s, rdp->settings->encryption ? SEC_ENCRYPT : 0);
}
//------------------------------------------------------------------------------
/*
 * Indicates a Client System Parameters Update PDU from client to server to
 * synchronize system parameters on the server with those on the client.
 */
void
rail_send_vchannel_client_sysparam_update_order()
{
	//TS_RAIL_ORDER_SYSPARAM
}
//------------------------------------------------------------------------------
/*
 * Indicates a Client System Command PDU from client to server when a local
 * RAIL window on the client receives a command to perform an action on the
 * window, such as minimize or maximize.
 */
void
rail_send_vchannel_syscommand_order()
{
	//TS_RAIL_ORDER_SYSCOMMAND
}
//------------------------------------------------------------------------------
/*
 * The Client Notify Event PDU packet is sent from a client to a server when
 * a local RAIL Notification Icon on the client receives a keyboard or mouse
 * message from the user. This notification is forwarded to the server via
 * the Notify Event PDU.
 * */
void
rail_send_vchannel_notify_event_order()
{
	//TS_RAIL_ORDER_NOTIFY_EVENT
}
//------------------------------------------------------------------------------
/*
 * The Client Window Move PDU packet is sent from the client to the server
 * when a local window is ending a move or resize. The client communicates the
 * locally moved or resized window's position to the server by using this packet.
 * The server uses this information to reposition its window.*/
void
rail_send_vchannel_windowmove_order()
{
	// TS_RAIL_ORDER_WINDOWMOVE
}
//------------------------------------------------------------------------------
/*
 * The Client Information PDU is sent from client to server and contains
 * information about RAIL client state and features supported by the client.
 * */
void
rail_send_vchannel_client_information_order()
{
	// TS_RAIL_ORDER_CLIENTSTATUS
}
//------------------------------------------------------------------------------
/*
 * The Client System Menu PDU packet is sent from the client to the server
 * when a local RAIL window on the client receives a command to display its
 * System menu. This command is forwarded to the server via
 * the System menu PDU.
 */
void
rail_send_vchannel_client_system_menu_order()
{
	// TS_RAIL_ORDER_SYSMENU
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
rail_send_vchannel_client_langbar_information_order()
{
	// TS_RAIL_ORDER_LANGBARINFO
}
//------------------------------------------------------------------------------
/*
 * The Client Get Application ID PDU is sent from a client to a server.
 * This PDU requests information from the server about the Application ID
 * that the window SHOULD <15> have on the client.
 * */
void
rail_send_vchannel_get_appid_req_order()
{
	// TS_RAIL_ORDER_GET_APPID_REQ
}
//------------------------------------------------------------------------------
/*
 * Look at rail_send_vchannel_handshake_order(...)
 * */
void
rail_process_vchannel_handshake_order()
{
	// RDP_RAIL_ORDER_HANDSHAKE
}
//------------------------------------------------------------------------------
/*
 * The Server Execute Result PDU is sent from server to client in response to
 * a Client Execute PDU request, and contains the result of the server's
 * attempt to launch the requested executable.
 */
void
rail_process_vchannel_exec_result_order()
{
	// RDP_RAIL_ORDER_EXEC_RESULT
}
//------------------------------------------------------------------------------
/*
 * The Server System Parameters Update PDU is sent from the server to client to
 * synchronize system parameters on the client with those on the server.
 */
void
rail_process_vchannel_server_sysparam_update_order()
{
	// RDP_RAIL_ORDER_SYSPARAM
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
rail_process_vchannel_server_movesize_order()
{
	// RDP_RAIL_ORDER_LOCALMOVESIZE
}
//------------------------------------------------------------------------------
/*
 * The Server Min Max Info PDU is sent from a server to a client when a window
 * move or resize on the server is being initiated.
 * This PDU contains information about the minimum and maximum extents to
 * which the window can be moved or sized.
 */
void
rail_process_vchannel_server_minmax_info_order()
{
	// RDP_RAIL_ORDER_MINMAXINFO
}
//------------------------------------------------------------------------------
/*
 *The Language Bar Information PDU is used to set the language bar status.
 */
void
rail_process_vchannel_server_langbar_info_order()
{
	// RDP_RAIL_ORDER_LANGBARINFO
}
//------------------------------------------------------------------------------
/*
 * The Server Get Application ID Response PDU is sent from a server to a client.
 * This PDU MAY be sent to the client as a response to a Client Get Application
 * ID PDU. This PDU specifies the Application ID that the specified window
 * SHOULD have on the client. The client MAY ignore this PDU.
 */
void
rail_process_vchannel_server_get_appid_resp_order()
{
	// RDP_RAIL_ORDER_GET_APPID_RESP
}
//------------------------------------------------------------------------------
void
rail_channel_process_received_pdu(STREAM s)
{
	uint16 order_type = 0;
	uint16 order_length = 0;

	out_uint16_le(s, order_type); /* orderType */
	out_uint16_le(s, order_length); /* orderLength */

	//TODO: ASSERT((orderLength - 4) <= ((uint8*)s->p - (uint8*)s->data) );

	switch (order_type)
	{
	case RDP_RAIL_ORDER_HANDSHAKE:
		break;
	case RDP_RAIL_ORDER_EXEC_RESULT:
		break;
	case RDP_RAIL_ORDER_SYSPARAM:
		break;
	case RDP_RAIL_ORDER_LOCALMOVESIZE:
		break;
	case RDP_RAIL_ORDER_MINMAXINFO:
		break;
	case RDP_RAIL_ORDER_LANGBARINFO:
		break;
	case RDP_RAIL_ORDER_GET_APPID_RESP:
		break;
	default:
		ASSERT(!"Undocumented RAIL channes server PDU order_type");
		break;
	}
}

/*
Flow of init stage over channel;

   Client notify UI about session start andgo to RAIL_ESTABLISHING state.

   Client send Handshake request
   Server send Handshake response
   Client check Handshake response. If NOT OK - exit with specified reason

   Server send Server System Parameters Update
   Client send Client Information
   Client send Client System Parameters Update
   Client send Client Execute
   Server send Server Execute Result
   Client check Server Execute Result. If NOT OK - exit with specified reason

   Client notify UI about success session establishing and go to
   RAIL_ESTABLISHED state.
*/
//------------------------------------------------------------------------------
/*For processing Capacities*/
void
rail_get_rail_capset(
		RAIL_SESSION * rail_session,
		uint32 * rail_support_level
		)
{
	*rail_support_level = (RAIL_LEVEL_SUPPORTED |
			RAIL_LEVEL_DOCKED_LANGBAR_SUPPORTED);
}
//------------------------------------------------------------------------------
void
rail_process_rail_capset(
		RAIL_SESSION * rail_session,
		uint32 rail_support_level
		)
{
	rail_session->rail_mode_supported 		=
		((rail_support_level & RAIL_LEVEL_SUPPORTED) ? 1 : 0);

	rail_session->docked_langbar_supported	=
		((rail_support_level & RAIL_LEVEL_DOCKED_LANGBAR_SUPPORTED) ? 1 : 0);
}
//------------------------------------------------------------------------------
void
rail_get_window_capset(
		RAIL_SESSION * rail_session,
		uint32 * window_support_level,
		uint8  * number_icon_caches,
		uint16 * number_icon_cache_entries
		)
{
	*window_support_level = (WINDOW_LEVEL_SUPPORTED | WINDOW_LEVEL_SUPPORTED_EX);
	*number_icon_caches = rail_session->number_icon_caches;
	*number_icon_cache_entries = rail_session->number_icon_cache_entries;
}
//------------------------------------------------------------------------------
void
rail_process_window_capset(
		RAIL_SESSION * rail_session,
		uint32 window_support_level,
		uint8  number_icon_caches,
		uint16 number_icon_cache_entries
		)
{
	rail_session->window_level_supported =
			((window_support_level & WINDOW_LEVEL_SUPPORTED) ? 1 : 0);
	rail_session->window_level_ex_supported =
			((window_support_level & WINDOW_LEVEL_SUPPORTED_EX) ? 1 : 0);
	rail_session->number_icon_caches = number_icon_caches;
	rail_session->number_icon_cache_entries = number_icon_cache_entries;
}
//------------------------------------------------------------------------------
