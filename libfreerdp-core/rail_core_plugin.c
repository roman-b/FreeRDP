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
/*
 * Used for opening channel and sent to core channel pointer to RAIL session.
 *
 * Main difference between core channels and plugin channels is time of
 * calling plugin 'VirtualChannelEntry'.
 * For plugin 'VirtualChannelEntry' is called when rdpInst for channel manager
 * is not yet defined. So we can't link it with rdp session.
 *
 * For core virtual channels 'VirtualChannelEntry' is called when rdpInst
 * linked with channel manager in 'freerdp_chanman_pre_connect' routine by
 * library consumer (in UI).
 *
 * So on this stage all rdpInst structure already builded, so we can call
 * core function for linking with RAIL session structure.
 */
int RailCoreVirtualChannelEntry(PCHANNEL_ENTRY_POINTS pEntryPoints)
{
	return 0;
}

