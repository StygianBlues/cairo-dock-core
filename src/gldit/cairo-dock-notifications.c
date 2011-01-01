/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "cairo-dock-icon-factory.h"
#include "cairo-dock-log.h"
#include "cairo-dock-notifications.h"

static GPtrArray *s_pNotificationsTab = NULL;  // tables des notifications globales.

static void _cairo_dock_register_notification_in_tab (GPtrArray *pNotificationsTab, CairoDockNotificationType iNotifType, CairoDockNotificationFunc pFunction, gboolean bRunFirst, gpointer pUserData)
{
	g_return_if_fail (iNotifType < pNotificationsTab->len);
	
	CairoDockNotificationRecord *pNotificationRecord = g_new (CairoDockNotificationRecord, 1);
	pNotificationRecord->pFunction = pFunction;
	pNotificationRecord->pUserData = pUserData;
	
	GSList *pNotificationRecordList = g_ptr_array_index (pNotificationsTab, iNotifType);
	if (bRunFirst)
		pNotificationsTab->pdata[iNotifType] = g_slist_prepend (pNotificationRecordList, pNotificationRecord);
	else
		pNotificationsTab->pdata[iNotifType] = g_slist_append (pNotificationRecordList, pNotificationRecord);
}


static void _cairo_dock_remove_notification_func_in_tab (GPtrArray *pNotificationsTab, CairoDockNotificationType iNotifType, CairoDockNotificationFunc pFunction, gpointer pUserData)
{
	if (pNotificationsTab == NULL)
		return ;
	GSList *pNotificationRecordList = g_ptr_array_index (pNotificationsTab, iNotifType);
	CairoDockNotificationRecord *pNotificationRecord;
	GSList *pElement;
	for (pElement = pNotificationRecordList; pElement != NULL; pElement = pElement->next)
	{
		pNotificationRecord = pElement->data;
		if (pNotificationRecord->pFunction == pFunction && pNotificationRecord->pUserData == pUserData)
		{
			pNotificationsTab->pdata[iNotifType] = g_slist_delete_link (pNotificationRecordList, pElement);
			g_free (pNotificationRecord);
		}
	}
}
void cairo_dock_remove_notification_func (CairoDockNotificationType iNotifType, CairoDockNotificationFunc pFunction, gpointer pUserData)
{
	_cairo_dock_remove_notification_func_in_tab (s_pNotificationsTab, iNotifType, pFunction, pUserData);
}


GSList *cairo_dock_get_notifications_list (CairoDockNotificationType iNotifType)
{
	if (s_pNotificationsTab == NULL || iNotifType >= s_pNotificationsTab->len)
		return NULL;
	
	return g_ptr_array_index (s_pNotificationsTab, iNotifType);
}


void cairo_dock_free_notification_table (GPtrArray *pNotificationsTab)
{
	if (pNotificationsTab == NULL)
		return ;
	guint i;
	for (i = 0; i < pNotificationsTab->len; i ++)
	{
		GSList *pNotificationRecordList = g_ptr_array_index (pNotificationsTab, i);
		g_slist_foreach (pNotificationRecordList, (GFunc)g_free, NULL);
		g_slist_free (pNotificationRecordList);
	}
	g_ptr_array_free (pNotificationsTab, TRUE);
}



void cairo_dock_register_notification_on_object (gpointer pObject, CairoDockNotificationType iNotifType, CairoDockNotificationFunc pFunction, gboolean bRunFirst, gpointer pUserData)
{
	g_return_if_fail (pObject != NULL);
	GPtrArray **pNotificationsTabPtr = (GPtrArray**) pObject;
	GPtrArray *pNotificationsTab = *pNotificationsTabPtr;
	
	if (pNotificationsTab == NULL)
	{
		pNotificationsTab = g_ptr_array_new ();
		*pNotificationsTabPtr = pNotificationsTab;
		g_ptr_array_set_size (pNotificationsTab, 10);
	}
	
	if (pNotificationsTab->len < iNotifType)
	{
		cd_warning ("someone tried to register to an inexisting notification (%d) on an object", iNotifType);
		g_ptr_array_set_size (pNotificationsTab, iNotifType+1);
	}
	
	_cairo_dock_register_notification_in_tab (pNotificationsTab, iNotifType, pFunction, bRunFirst, pUserData);
}

void cairo_dock_remove_notification_func_on_object (gpointer pObject, CairoDockNotificationType iNotifType, CairoDockNotificationFunc pFunction, gpointer pUserData)
{
	g_return_if_fail (pObject != NULL);
	GPtrArray **pNotificationsTabPtr = (GPtrArray**) pObject;
	GPtrArray *pNotificationsTab = *pNotificationsTabPtr;
	_cairo_dock_remove_notification_func_in_tab (pNotificationsTab, iNotifType, pFunction, pUserData);
}
