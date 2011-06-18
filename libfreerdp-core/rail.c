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
