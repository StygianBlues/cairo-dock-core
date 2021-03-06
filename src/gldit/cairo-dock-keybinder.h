/*
* cairo-dock-keybinder.h
* This file is a part of the Cairo-Dock project
* Login : <ctaf42@localhost.localdomain>
* Started on  Thu Jan 31 03:57:17 2008 Cedric GESTES
* $Id$
*
* Author(s)
*  - Cedric GESTES <ctaf42@gmail.com>
*  - Havoc Pennington
*  - Tim Janik
*
* Copyright : (C) 2008 Cedric GESTES
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
*
* imported from tomboy_key_binder.h
*/

#ifndef __CD_KEY_BINDER_H__
#define __CD_KEY_BINDER_H__

#include "cairo-dock-struct.h"
#include "cairo-dock-manager.h"

G_BEGIN_DECLS

/**
*@file cairo-dock-keybinder.h This class defines the Shortkeys, which are objects that bind a keyboard shortcut to an action. The keyboard shortcut is defined globally on the desktop, that is to say they will be effective whatever window has the focus.
* Keyboard shortcuts are of the form &lt;alt&gt;F1 or &lt;ctrl&gt;&lt;shift&gt;s.
* 
* Use \ref gldi_shortkey_new to create a new shortkey, and simply unref it with \ref gldi_object_unref to unbind the keyboard shortcut.
* To update a binding (whenever the shortcut or the description change, or just to re-grab it), use \ref gldi_shortkey_rebind.
*/


/// Definition of a callback, called when a shortcut is pressed by the user.
typedef void (* CDBindkeyHandler) (const gchar *keystring, gpointer user_data);

struct _GldiShortkey {
	/// object.
	GldiObject object;
	gchar            *keystring;
	CDBindkeyHandler  handler;
	gpointer          user_data;
	guint             keycode;
	guint             modifiers;
	gboolean          bSuccess;
	gchar            *cDemander;
	gchar            *cDescription;
	gchar            *cIconFilePath;
	gchar            *cConfFilePath;
	gchar            *cGroupName;
	gchar            *cKeyName;
} ;


// manager
typedef struct _GldiShortkeyAttr GldiShortkeyAttr;

#ifndef _MANAGER_DEF_
extern GldiObjectManager myShortkeyObjectMgr;
#endif

struct _GldiShortkeyAttr {
	const gchar            *keystring;
	CDBindkeyHandler        handler;
	gpointer                user_data;
	const gchar            *cDemander;
	const gchar            *cDescription;
	const gchar            *cIconFilePath;
	const gchar            *cConfFilePath;
	const gchar            *cGroupName;
	const gchar            *cKeyName;
};

// signals
typedef enum {
	NOTIFICATION_SHORTKEY_CHANGED = NB_NOTIFICATIONS_OBJECT,
	NB_NOTIFICATIONS_SHORTKEYS
	} GldiShortkeysNotifications;


/** Create a new shortkey, that binds an action to a shortkey. Unref it when you don't want it anymore, or when 'user_data' is freed.
 * @param keystring a shortcut.
 * @param cDemander the actor making the demand
 * @param cDescription a short description of the action
 * @param cIconFilePath an icon that represents the demander
 * @param cConfFilePath conf file where the shortkey stored
 * @param cGroupName group name where it's stored in the conf file
 * @param cKeyName key name where it's stored in the conf file
 * @param handler function called when the shortkey is pressed by the user
 * @param user_data data passed to the callback
 * @return the shortkey, already bound.
*/
GldiShortkey *gldi_shortkey_new (const gchar *keystring,
	const gchar *cDemander,
	const gchar *cDescription,
	const gchar *cIconFilePath,
	const gchar *cConfFilePath,
	const gchar *cGroupName,
	const gchar *cKeyName,
	CDBindkeyHandler handler,
	gpointer user_data);

/** Says if the shortkey of a key binding could be grabbed.
 * @param binding a key binding.
 * @return TRUE iif the shortkey has been successfuly grabbed by the key binding.
*/
#define gldi_shortkey_could_grab(binding) ((binding)->bSuccess)


/** Rebind a shortkey to a new one. If the shortkey is the same, don't re-bind it.
 * @param binding a key binding.
 * @param cNewKeyString the new shortkey
 * @param cNewDescription the new description, or NULL to keep the current one.
 * @return TRUE on success
*/
gboolean gldi_shortkey_rebind (GldiShortkey *binding,
	const gchar *cNewKeyString,
	const gchar *cNewDescription);


void gldi_shortkeys_foreach (GFunc pCallback, gpointer data);

/** Trigger a given shortkey. It will be as if the user effectively pressed the shortkey on its keyboard. It uses the 'XTest' X extension.
 * @param cKeyString a shortkey.
 * @return TRUE if success.
*/
gboolean cairo_dock_trigger_shortkey (const gchar *cKeyString);


void gldi_register_shortkeys_manager (void);

G_END_DECLS

#endif /* __CD_KEY_BINDER_H__ */

