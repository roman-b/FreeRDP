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

/* TODO:
 * - add session termination if server does not support session
 * - add core virtual channel
 * - add RAIL order handlers
 * - add establishing RAIL connection ([MS-RDPERP] 1.3.2.1)
 */


#ifndef __RAIL_CORE_PLUGIN_H
#define	__RAIL_CORE_PLUGIN_H

#include <freerdp/vchan.h>
#include <freerdp/constants/rail.h>
#include "rdp.h"
#include "rail.h"

/*
 * RAIL virtual core plugin
 * Used for 'rail' channel creation and manipulation
 */


int RailCoreVirtualChannelEntry(PCHANNEL_ENTRY_POINTS pEntryPoints);

#endif	// __RAIL_CORE_PLUGIN_H
