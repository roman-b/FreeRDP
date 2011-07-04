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


//  EXAMPLE for uniconv conversation
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


//------------------------------------------------------------------------------
RAIL_SESSION *
rail_session_new(struct rdp_rdp * rdp)
{
	RAIL_SESSION * self;

	self = (RAIL_SESSION *) xmalloc(sizeof(RAIL_SESSION));
	if (self != NULL)
	{
		memset(self, 0, sizeof(RAIL_SESSION));
		self->rdp = rdp;
		self->number_icon_caches = 3;
		self->number_icon_cache_entries = 17;

		self->channel_sender.sender_object = NULL;
		self->channel_sender.send_rail_vchannel_data = NULL;
	}
	return self;
}
//------------------------------------------------------------------------------
void
rail_session_free(RAIL_SESSION * rail_session)
{
	if (rail_session != NULL)
	{
		xfree(rail_session);
	}
}
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
void
rail_on_channel_connected(RAIL_SESSION* rail_session)
{
}
//------------------------------------------------------------------------------
void
rail_on_channel_terminated(RAIL_SESSION* rail_session)
{
}
//------------------------------------------------------------------------------
void
rail_register_channel_sender(
		RAIL_SESSION* rail_session,
		RAIL_VCHANNEL_SENDER* sender
		)
{
	rail_session->channel_sender.sender_object = sender->sender_object;
	rail_session->channel_sender.send_rail_vchannel_data =
			sender->send_rail_vchannel_data;
}
//------------------------------------------------------------------------------
