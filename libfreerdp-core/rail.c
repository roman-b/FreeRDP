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
