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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include <freerdp/rdpset.h>
#include <freerdp/utils/memory.h>
#include <freerdp/vchan.h>
#include <freerdp/utils/stream.h>
#include <freerdp/utils/chan_plugin.h>
#include <freerdp/utils/wait_obj.h>


#include "frdp.h"
#include "rdp.h"
#include "rdp.h"
#include "rail.h"

#define LOG_LEVEL 11
#define LLOG(_level, _args) \
  do { if (_level < LOG_LEVEL) { printf _args ; } } while (0)
#define LLOGLN(_level, _args) \
  do { if (_level < LOG_LEVEL) { printf _args ; printf("\n"); } } while (0)

struct data_in_item
{
	struct data_in_item * next;
	char * data;
	int data_size;
};

struct rail_core_plugin
{
	rdpChanPlugin chan_plugin;

	CHANNEL_ENTRY_POINTS ep;
	CHANNEL_DEF channel_def;
	uint32 open_handle;
	char * data_in;
	int data_in_size;
	int data_in_read;
	struct wait_obj * term_event;
	struct wait_obj * data_in_event;
	struct data_in_item * in_list_head;
	struct data_in_item * in_list_tail;
	/* for locking the linked list */
	pthread_mutex_t * in_mutex;
	int thread_status;

	RAIL_SESSION * session;
};

typedef struct rail_core_plugin railCorePlugin;

//------------------------------------------------------------------------------
int
rail_core_plugin_send_packet(
		railCorePlugin * plugin,
		void * data,
		size_t length
		)
{
	char * out_data;
	size_t size;
	uint32 error;

	size = length;
	out_data = (char *) malloc(size);
	memcpy(out_data, data, size);

	error = plugin->ep.pVirtualChannelWrite(plugin->open_handle,
		out_data, size, out_data);

	if (error != CHANNEL_RC_OK)
	{
		LLOGLN(0, ("rail_core_plugin_send_packet: "
			"VirtualChannelWrite "
			"failed %d", error));
		return 1;
	}
	return 0;
}
//------------------------------------------------------------------------------
void
rail_core_plugin_send_rail_vchannel_data(
		void* sender_object,
		void* data,
		size_t length
		)
{
	railCorePlugin* plugin = (railCorePlugin*)sender_object;

	rail_core_plugin_send_packet(plugin, data, length);
}
//------------------------------------------------------------------------------
/* process the linked list of data that has come in */
static int
process_received_data(railCorePlugin * plugin)
{
	char * data;
	int data_size;
	struct data_in_item * item;

	while (1)
	{
		if (wait_obj_is_set(plugin->term_event))
		{
			break;
		}

		pthread_mutex_lock(plugin->in_mutex);
		if (plugin->in_list_head == NULL)
		{
			pthread_mutex_unlock(plugin->in_mutex);
			break;
		}
		data = plugin->in_list_head->data;
		data_size = plugin->in_list_head->data_size;

		item = plugin->in_list_head;
		plugin->in_list_head = plugin->in_list_head->next;
		if (plugin->in_list_head == NULL)
		{
			plugin->in_list_tail = NULL;
		}
		pthread_mutex_unlock(plugin->in_mutex);

		if (data != NULL)
		{
			rail_on_channel_data_received(plugin->session, data, data_size);
			free(data);
		}
		if (item != NULL)
		{
			free(item);
		}
	}
	return 0;
}
//------------------------------------------------------------------------------
static void *
received_data_processing_thread_func(void * arg)
{
	railCorePlugin * plugin;
	struct wait_obj * listobj[2];
	int numobj;

	plugin = (railCorePlugin *) arg;

	plugin->thread_status = 1;
	LLOGLN(10, ("rail_core_plugin:received_data_processing_thread_func: in"));
	while (1)
	{
		listobj[0] = plugin->term_event;
		listobj[1] = plugin->data_in_event;
		numobj = 2;
		wait_obj_select(listobj, numobj, NULL, 0, 500);

		if (wait_obj_is_set(plugin->term_event))
		{
			break;
		}
		if (wait_obj_is_set(plugin->data_in_event))
		{
			wait_obj_clear(plugin->data_in_event);

			/* process data in */
			process_received_data(plugin);
		}
	}
	LLOGLN(10, ("rail_core_plugin:received_data_processing_thread_func: out"));
	plugin->thread_status = -1;
	return 0;
}
//------------------------------------------------------------------------------
/* called by main thread
   add item to linked list and inform worker thread that there is data */
static void
queue_data_and_signal_data_received(railCorePlugin * plugin)
{
	struct data_in_item * item;

	item = (struct data_in_item *) malloc(sizeof(struct data_in_item));

	item->data = plugin->data_in;
	item->data_size = plugin->data_in_size;
	item->next = NULL;

	plugin->data_in = NULL;
	plugin->data_in_size = 0;

	pthread_mutex_lock(plugin->in_mutex);
	if (plugin->in_list_tail == NULL)
	{
		plugin->in_list_head = item;
		plugin->in_list_tail = item;
	}
	else
	{
		plugin->in_list_tail->next = item;
		plugin->in_list_tail = item;
	}
	pthread_mutex_unlock(plugin->in_mutex);
	wait_obj_set(plugin->data_in_event);
}
//------------------------------------------------------------------------------
static void
OpenEventProcessReceived(uint32 openHandle, void * pData, uint32 dataLength,
	uint32 totalLength, uint32 dataFlags)
{
	railCorePlugin * plugin;

	plugin = (railCorePlugin *) chan_plugin_find_by_open_handle(openHandle);

	LLOGLN(10, ("rail_core_plugin:OpenEventProcessReceived: receive openHandle %d dataLength %d "
		"totalLength %d dataFlags %d",
		openHandle, dataLength, totalLength, dataFlags));
	if (dataFlags & CHANNEL_FLAG_FIRST)
	{
		plugin->data_in_read = 0;
		if (plugin->data_in != NULL)
		{
			free(plugin->data_in);
		}
		plugin->data_in = (char *) malloc(totalLength);
		plugin->data_in_size = totalLength;
	}
	memcpy(plugin->data_in + plugin->data_in_read, pData, dataLength);
	plugin->data_in_read += dataLength;
	if (dataFlags & CHANNEL_FLAG_LAST)
	{
		if (plugin->data_in_read != plugin->data_in_size)
		{
			LLOGLN(0, ("rail_core_plugin:OpenEventProcessReceived: read error"));
		}
		queue_data_and_signal_data_received(plugin);
	}
}
//------------------------------------------------------------------------------
static void
OpenEvent(uint32 openHandle, uint32 event, void * pData, uint32 dataLength,
	uint32 totalLength, uint32 dataFlags)
{
	LLOGLN(10, ("rail_core_plugin:OpenEvent: event %d", event));
	switch (event)
	{
		case CHANNEL_EVENT_DATA_RECEIVED:
			OpenEventProcessReceived(openHandle, pData, dataLength,
				totalLength, dataFlags);
			break;
		case CHANNEL_EVENT_WRITE_COMPLETE:
			free(pData);
			break;
	}
}
//------------------------------------------------------------------------------
static void
InitEventProcessConnected(void * pInitHandle, void * pData, uint32 dataLength)
{
	railCorePlugin * plugin;
	uint32 error;
	pthread_t thread;

	LLOGLN(10, ("rail_core_plugin:InitEventProcessConnected:"));

	plugin = (railCorePlugin *) chan_plugin_find_by_init_handle(pInitHandle);
	if (plugin == NULL)
	{
		LLOGLN(0, ("rail_core_plugin:InitEventProcessConnected: error no match"));
		return;
	}

	error = plugin->ep.pVirtualChannelOpen(pInitHandle, &(plugin->open_handle),
		plugin->channel_def.name, OpenEvent);
	if (error != CHANNEL_RC_OK)
	{
		LLOGLN(0, ("rail_core_plugin:InitEventProcessConnected: Open failed"));
		return;
	}
	chan_plugin_register_open_handle((rdpChanPlugin *) plugin,
		plugin->open_handle);

	pthread_create(&thread, 0, received_data_processing_thread_func, plugin);
	pthread_detach(thread);

	// Notify RAIL lib about channel connection.
	rail_on_channel_connected(plugin->session);
}
//------------------------------------------------------------------------------
static void
InitEventProcessTerminated(void * pInitHandle)
{
	railCorePlugin * plugin;
	int index;
	struct data_in_item * in_item;

	LLOGLN(10, ("rail_core_plugin:InitEventProcessTerminated: pInitHandle=0x%p",
			pInitHandle));

	plugin = (railCorePlugin *) chan_plugin_find_by_init_handle(pInitHandle);
	if (plugin == NULL)
	{
		LLOGLN(0, ("rail_core_plugin:InitEventProcessTerminated: error no match"));
		return;
	}


	wait_obj_set(plugin->term_event);
	index = 0;
	while ((plugin->thread_status > 0) && (index < 100))
	{
		index++;
		usleep(250 * 1000);
	}
	wait_obj_free(plugin->term_event);
	wait_obj_free(plugin->data_in_event);

	pthread_mutex_destroy(plugin->in_mutex);
	free(plugin->in_mutex);

	/* free the un-processed in/out queue */
	while (plugin->in_list_head != NULL)
	{
		in_item = plugin->in_list_head;
		plugin->in_list_head = in_item->next;
		free(in_item->data);
		free(in_item);
	}
	if (plugin->data_in != NULL)
	{
		free(plugin->data_in);
	}

	chan_plugin_uninit((rdpChanPlugin *) plugin);

	rail_on_channel_terminated(plugin->session);
	free(plugin);
}
//------------------------------------------------------------------------------
static void
InitEventInitialized(void * pInitHandle)
{
	railCorePlugin * plugin;

	LLOGLN(10, ("rail_core_plugin:InitEventInitialized: pInitHandle=0x%p",
			pInitHandle));

	plugin = (railCorePlugin *) chan_plugin_find_by_init_handle(pInitHandle);
	if (plugin == NULL)
	{
		LLOGLN(0, ("rail_core_plugin:InitEventInitialized: error no match"));
		return;
	}
	LLOGLN(10, ("rail_core_plugin:InitEventInitialized: session=0x%p",
			plugin->session));
}
//------------------------------------------------------------------------------
static void
InitEvent(void * pInitHandle, uint32 event, void * pData, uint32 dataLength)
{
	LLOGLN(10, ("rail_core_plugin:InitEvent: event %d", event));
	switch (event)
	{
		case CHANNEL_EVENT_INITIALIZED:
			InitEventInitialized(pInitHandle);
			break;
		case CHANNEL_EVENT_CONNECTED:
			InitEventProcessConnected(pInitHandle, pData, dataLength);
			break;
		case CHANNEL_EVENT_DISCONNECTED:
			break;
		case CHANNEL_EVENT_TERMINATED:
			InitEventProcessTerminated(pInitHandle);
			break;
	}
}
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
	railCorePlugin * plugin;

	LLOGLN(10, ("rail_core_plugin:Rail Core plugin VirtualChannelEntry:"));

	plugin = (railCorePlugin *) malloc(sizeof(railCorePlugin));
	memset(plugin, 0, sizeof(railCorePlugin));

	chan_plugin_init((rdpChanPlugin *) plugin);

	plugin->data_in_size = 0;
	plugin->data_in = NULL;
	plugin->ep = *pEntryPoints;

	memset(&(plugin->channel_def), 0, sizeof(plugin->channel_def));
	plugin->channel_def.options = CHANNEL_OPTION_INITIALIZED |
		CHANNEL_OPTION_ENCRYPT_RDP | CHANNEL_OPTION_COMPRESS_RDP |
		CHANNEL_OPTION_SHOW_PROTOCOL;
	strcpy(plugin->channel_def.name, "rail");

	plugin->in_mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(plugin->in_mutex, 0);
	plugin->in_list_head = NULL;
	plugin->in_list_tail = NULL;

	plugin->term_event = wait_obj_new("freerdprailterm");
	plugin->data_in_event = wait_obj_new("freerdpraildatain");


	plugin->session = NULL;
	if (pEntryPoints->cbSize >= sizeof(CHANNEL_ENTRY_POINTS_EX))
	{
		RAIL_VCHANNEL_SENDER sender = {0};

		sender.sender_object = plugin;
		sender.send_rail_vchannel_data = rail_core_plugin_send_rail_vchannel_data;

		plugin->session = (((PCHANNEL_ENTRY_POINTS_EX)pEntryPoints)->pExtendedData);

		rail_register_channel_sender(plugin->session, &sender);

		plugin->ep.pVirtualChannelInit(&plugin->chan_plugin.init_handle,
				&plugin->channel_def, 1, VIRTUAL_CHANNEL_VERSION_WIN2000,
				InitEvent);
	}


	return 1;
}

