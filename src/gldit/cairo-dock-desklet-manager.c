/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
** Login : <ctaf42@gmail.com>
** Started on  Sun Jan 27 18:35:38 2008 Cedric GESTES
** $Id$
**
** Author(s)
**  - Cedric GESTES <ctaf42@gmail.com>
**  - Fabrice REY
**
** Copyright (C) 2008 Cedric GESTES
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/


#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <GL/gl.h> 
#include <GL/glu.h> 
#include <GL/glx.h> 
#include <gdk/gdkx.h>

#include "gldi-config.h"
#include "cairo-dock-module-factory.h"  // cairo_dock_pre_read_module_instance_config
#include "cairo-dock-config.h"
#include "cairo-dock-icon-facility.h"  // cairo_dock_set_icon_container
#include "cairo-dock-X-manager.h"
#include "cairo-dock-notifications.h"
#include "cairo-dock-log.h"
#include "cairo-dock-container.h"
#include "cairo-dock-X-utilities.h"  // cairo_dock_set_xwindow_type_hint
#include "cairo-dock-backends-manager.h"
#include "cairo-dock-draw-opengl.h"
#include "cairo-dock-image-buffer.h"
#include "cairo-dock-gui-manager.h"
#include "cairo-dock-desklet-factory.h"
#define _MANAGER_DEF_
#include "cairo-dock-desklet-manager.h"

// public (manager, config, data)
CairoDeskletsParam myDeskletsParam;
CairoDeskletManager myDeskletsMgr;

// dependancies
extern gboolean g_bUseOpenGL;
extern CairoDock *g_pMainDock;  // pour savoir s'il faut afficher les boutons rattach.

// private
static CairoDockImageBuffer s_pRotateButtonBuffer;
static CairoDockImageBuffer s_pRetachButtonBuffer;
static CairoDockImageBuffer s_pDepthRotateButtonBuffer;
static CairoDockImageBuffer s_pNoInputButtonBuffer;
static GList *s_pDeskletList = NULL;
static time_t s_iStartupTime = 0;

static gboolean _cairo_dock_update_desklet_notification (gpointer data, CairoDesklet *pDesklet, gboolean *bContinueAnimation);
static gboolean _cairo_dock_enter_leave_desklet_notification (gpointer data, CairoDesklet *pDesklet, gboolean *bStartAnimation);
static gboolean _cairo_dock_render_desklet_notification (gpointer pUserData, CairoDesklet *pDesklet, cairo_t *pCairoContext);

#define _no_input_button_alpha(pDesklet) (pDesklet->bNoInput ? .4+.6*pDesklet->fButtonsAlpha : pDesklet->fButtonsAlpha)
#define ANGLE_MIN .1  // under this angle, the rotation is ignored.

void cairo_dock_load_desklet_buttons (void)
{
	//g_print ("%s ()\n", __func__);
	if (myDeskletsParam.cRotateButtonImage != NULL)
	{
		cairo_dock_load_image_buffer (&s_pRotateButtonBuffer,
			myDeskletsParam.cRotateButtonImage,
			myDeskletsParam.iDeskletButtonSize,
			myDeskletsParam.iDeskletButtonSize,
			CAIRO_DOCK_FILL_SPACE);
	}
	if (s_pRotateButtonBuffer.pSurface == NULL)
	{
		cairo_dock_load_image_buffer (&s_pRotateButtonBuffer,
			GLDI_SHARE_DATA_DIR"/icons/rotate-desklet.svg",
			myDeskletsParam.iDeskletButtonSize,
			myDeskletsParam.iDeskletButtonSize,
			CAIRO_DOCK_FILL_SPACE);
	}
	
	if (myDeskletsParam.cRetachButtonImage != NULL)
	{
		cairo_dock_load_image_buffer (&s_pRetachButtonBuffer,
			myDeskletsParam.cRetachButtonImage,
			myDeskletsParam.iDeskletButtonSize,
			myDeskletsParam.iDeskletButtonSize,
			CAIRO_DOCK_FILL_SPACE);
	}
	if (s_pRetachButtonBuffer.pSurface == NULL)
	{
		cairo_dock_load_image_buffer (&s_pRetachButtonBuffer,
			GLDI_SHARE_DATA_DIR"/icons/retach-desklet.svg",
			myDeskletsParam.iDeskletButtonSize,
			myDeskletsParam.iDeskletButtonSize,
			CAIRO_DOCK_FILL_SPACE);
	}

	if (myDeskletsParam.cDepthRotateButtonImage != NULL)
	{
		cairo_dock_load_image_buffer (&s_pDepthRotateButtonBuffer,
			myDeskletsParam.cDepthRotateButtonImage,
			myDeskletsParam.iDeskletButtonSize,
			myDeskletsParam.iDeskletButtonSize,
			CAIRO_DOCK_FILL_SPACE);
	}
	if (s_pDepthRotateButtonBuffer.pSurface == NULL)
	{
		cairo_dock_load_image_buffer (&s_pDepthRotateButtonBuffer,
			GLDI_SHARE_DATA_DIR"/icons/depth-rotate-desklet.svg",
			myDeskletsParam.iDeskletButtonSize,
			myDeskletsParam.iDeskletButtonSize,
			CAIRO_DOCK_FILL_SPACE);
	}
	
	if (myDeskletsParam.cNoInputButtonImage != NULL)
	{
		cairo_dock_load_image_buffer (&s_pNoInputButtonBuffer,
			myDeskletsParam.cNoInputButtonImage,
			myDeskletsParam.iDeskletButtonSize,
			myDeskletsParam.iDeskletButtonSize,
			CAIRO_DOCK_FILL_SPACE);
	}
	if (s_pNoInputButtonBuffer.pSurface == NULL)
	{
		cairo_dock_load_image_buffer (&s_pNoInputButtonBuffer,
			GLDI_SHARE_DATA_DIR"/icons/no-input-desklet.png",
			myDeskletsParam.iDeskletButtonSize,
			myDeskletsParam.iDeskletButtonSize,
			CAIRO_DOCK_FILL_SPACE);
	}
}

void cairo_dock_unload_desklet_buttons (void)
{
	//g_print ("%s ()\n", __func__);
	cairo_dock_unload_image_buffer (&s_pRotateButtonBuffer);
	cairo_dock_unload_image_buffer (&s_pRetachButtonBuffer);
	cairo_dock_unload_image_buffer (&s_pDepthRotateButtonBuffer);
	cairo_dock_unload_image_buffer (&s_pNoInputButtonBuffer);
}


  ///////////////
 /// DRAWING ///
///////////////

static inline double _compute_zoom_for_rotation (CairoDesklet *pDesklet)
{
	double w = pDesklet->container.iWidth/2, h = pDesklet->container.iHeight/2;
	double alpha = atan2 (h, w);
	double theta = fabs (pDesklet->fRotation);
	if (theta > G_PI/2)
		theta -= G_PI/2;
	
	double d = sqrt (w * w + h * h);
	double xmax = d * MAX (fabs (cos (alpha + theta)), fabs (cos (alpha - theta)));
	double ymax = d * MAX (fabs (sin (alpha + theta)), fabs (sin (alpha - theta)));
	double fZoom = MIN (w / xmax, h / ymax);
	return fZoom;
}

static inline void _render_desklet_cairo (CairoDesklet *pDesklet, cairo_t *pCairoContext)
{
	cairo_save (pCairoContext);
	
	if (pDesklet->container.fRatio != 1)
	{
		//g_print (" desklet zoom : %.2f (%dx%d)\n", pDesklet->container.fRatio, pDesklet->container.iWidth, pDesklet->container.iHeight);
		cairo_translate (pCairoContext,
			pDesklet->container.iWidth * (1 - pDesklet->container.fRatio)/2,
			pDesklet->container.iHeight * (1 - pDesklet->container.fRatio)/2);
		cairo_scale (pCairoContext, pDesklet->container.fRatio, pDesklet->container.fRatio);
	}
	
	if (fabs (pDesklet->fRotation) > ANGLE_MIN)
	{
		double fZoom = _compute_zoom_for_rotation (pDesklet);
		
		cairo_translate (pCairoContext,
			.5*pDesklet->container.iWidth,
			.5*pDesklet->container.iHeight);
		
		cairo_rotate (pCairoContext, pDesklet->fRotation);
		
		cairo_scale (pCairoContext, fZoom, fZoom);
		
		cairo_translate (pCairoContext,
			-.5*pDesklet->container.iWidth,
			-.5*pDesklet->container.iHeight);
	}
	
	if (pDesklet->backGroundImageBuffer.pSurface != NULL)
	{
		cairo_set_source_surface (pCairoContext,
			pDesklet->backGroundImageBuffer.pSurface,
			0.,
			0.);
			cairo_paint (pCairoContext);
	}
	
	cairo_save (pCairoContext);
	if (pDesklet->iLeftSurfaceOffset != 0 || pDesklet->iTopSurfaceOffset != 0 || pDesklet->iRightSurfaceOffset != 0 || pDesklet->iBottomSurfaceOffset != 0)
	{
		cairo_translate (pCairoContext, pDesklet->iLeftSurfaceOffset, pDesklet->iTopSurfaceOffset);
		cairo_scale (pCairoContext,
			1. - 1.*(pDesklet->iLeftSurfaceOffset + pDesklet->iRightSurfaceOffset) / pDesklet->container.iWidth,
			1. - 1.*(pDesklet->iTopSurfaceOffset + pDesklet->iBottomSurfaceOffset) / pDesklet->container.iHeight);
	}
	
	if (pDesklet->pRenderer != NULL && pDesklet->pRenderer->render != NULL)  // un moteur de rendu specifique a ete fourni.
	{
		pDesklet->pRenderer->render (pCairoContext, pDesklet);
	}
	cairo_restore (pCairoContext);
	
	if (pDesklet->foreGroundImageBuffer.pSurface != NULL)
	{
		cairo_set_source_surface (pCairoContext,
			pDesklet->foreGroundImageBuffer.pSurface,
			0.,
			0.);
			cairo_paint (pCairoContext);
	}
	
	if (! pDesklet->rotating)  // si on est en train de tourner, les boutons suivent le mouvement, sinon ils sont dans les coins.
	{
		cairo_restore (pCairoContext);
		cairo_save (pCairoContext);
	}
	if ((pDesklet->container.bInside || pDesklet->rotating || pDesklet->fButtonsAlpha != 0) && cairo_dock_desklet_is_free (pDesklet))
	{
		if (s_pRotateButtonBuffer.pSurface != NULL)
		{
			cairo_set_source_surface (pCairoContext, s_pRotateButtonBuffer.pSurface, 0., 0.);
			cairo_paint_with_alpha (pCairoContext, pDesklet->fButtonsAlpha);
		}
		if (s_pRetachButtonBuffer.pSurface != NULL && g_pMainDock)
		{
			cairo_set_source_surface (pCairoContext, s_pRetachButtonBuffer.pSurface, pDesklet->container.iWidth - myDeskletsParam.iDeskletButtonSize, 0.);
			cairo_paint_with_alpha (pCairoContext, pDesklet->fButtonsAlpha);
		}
	}
	if ((pDesklet->container.bInside || pDesklet->bNoInput || pDesklet->fButtonsAlpha) && s_pNoInputButtonBuffer.pSurface != NULL && pDesklet->bAllowNoClickable)
	{
		cairo_set_source_surface (pCairoContext, s_pNoInputButtonBuffer.pSurface, pDesklet->container.iWidth - myDeskletsParam.iDeskletButtonSize, pDesklet->container.iHeight - myDeskletsParam.iDeskletButtonSize);
		cairo_paint_with_alpha (pCairoContext, _no_input_button_alpha (pDesklet));
	}
	cairo_restore (pCairoContext);
}

static inline void _cairo_dock_set_desklet_matrix (CairoDesklet *pDesklet)
{
	double fDepthRotationY = (fabs (pDesklet->fDepthRotationY) > ANGLE_MIN ? pDesklet->fDepthRotationY : 0.);
	double fDepthRotationX = (fabs (pDesklet->fDepthRotationX) > ANGLE_MIN ? pDesklet->fDepthRotationX : 0.);
	glTranslatef (0., 0., -pDesklet->container.iHeight * sqrt(3)/2 - 
		.45 * MAX (pDesklet->container.iWidth * fabs (sin (fDepthRotationY)),
			pDesklet->container.iHeight * fabs (sin (fDepthRotationX)))
		);  // avec 60 deg de perspective
	
	if (pDesklet->container.fRatio != 1)
	{
		glScalef (pDesklet->container.fRatio, pDesklet->container.fRatio, 1.);
	}
	
	if (fabs (pDesklet->fRotation) > ANGLE_MIN)
	{
		double fZoom = _compute_zoom_for_rotation (pDesklet);
		glScalef (fZoom, fZoom, 1.);
		glRotatef (- pDesklet->fRotation / G_PI * 180., 0., 0., 1.);
	}
	
	if (fDepthRotationY != 0)
	{
		glRotatef (- pDesklet->fDepthRotationY / G_PI * 180., 0., 1., 0.);
	}
	
	if (fDepthRotationX != 0)
	{
		glRotatef (- pDesklet->fDepthRotationX / G_PI * 180., 1., 0., 0.);
	}
}

static inline void _render_desklet_opengl (CairoDesklet *pDesklet)
{
	glPushMatrix ();
	///glTranslatef (0*pDesklet->container.iWidth/2, 0*pDesklet->container.iHeight/2, 0.);  // avec une perspective ortho.
	///glTranslatef (0*pDesklet->container.iWidth/2, 0*pDesklet->container.iHeight/2, -pDesklet->container.iWidth*(1.87 +.35*fabs (sin(pDesklet->fDepthRotationY))));  // avec 30 deg de perspective
	_cairo_dock_set_desklet_matrix (pDesklet);
	
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_pbuffer ();
	
	if (pDesklet->backGroundImageBuffer.iTexture != 0/** && pDesklet->fBackGroundAlpha != 0*/)
	{
		_cairo_dock_apply_texture_at_size_with_alpha (pDesklet->backGroundImageBuffer.iTexture, pDesklet->container.iWidth, pDesklet->container.iHeight, 1./**pDesklet->fBackGroundAlpha*/);
	}
	
	glPushMatrix ();
	if (pDesklet->iLeftSurfaceOffset != 0 || pDesklet->iTopSurfaceOffset != 0 || pDesklet->iRightSurfaceOffset != 0 || pDesklet->iBottomSurfaceOffset != 0)
	{
		glTranslatef ((pDesklet->iLeftSurfaceOffset - pDesklet->iRightSurfaceOffset)/2, (pDesklet->iBottomSurfaceOffset - pDesklet->iTopSurfaceOffset)/2, 0.);
		glScalef (1. - 1.*(pDesklet->iLeftSurfaceOffset + pDesklet->iRightSurfaceOffset) / pDesklet->container.iWidth,
			1. - 1.*(pDesklet->iTopSurfaceOffset + pDesklet->iBottomSurfaceOffset) / pDesklet->container.iHeight,
			1.);
	}
	
	if (pDesklet->pRenderer != NULL && pDesklet->pRenderer->render_opengl != NULL)  // un moteur de rendu specifique a ete fourni.
	{
		_cairo_dock_set_alpha (1.);
		pDesklet->pRenderer->render_opengl (pDesklet);
	}
	glPopMatrix ();
	
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_pbuffer ();
	if (pDesklet->foreGroundImageBuffer.iTexture != 0/** && pDesklet->fForeGroundAlpha != 0*/)
	{
		_cairo_dock_apply_texture_at_size_with_alpha (pDesklet->foreGroundImageBuffer.iTexture, pDesklet->container.iWidth, pDesklet->container.iHeight, 1./**pDesklet->fForeGroundAlpha*/);
	}
	
	//if (pDesklet->container.bInside && cairo_dock_desklet_is_free (pDesklet))
	{
		if (! pDesklet->rotating && ! pDesklet->rotatingY && ! pDesklet->rotatingX)
		{
			glPopMatrix ();
			glPushMatrix ();
			glTranslatef (0., 0., -pDesklet->container.iHeight*(sqrt(3)/2));
		}
	}
	
	if ((pDesklet->container.bInside || pDesklet->fButtonsAlpha != 0 || pDesklet->rotating || pDesklet->rotatingY || pDesklet->rotatingX) && cairo_dock_desklet_is_free (pDesklet))
	{
		_cairo_dock_set_blend_alpha ();
		_cairo_dock_set_alpha (sqrt(pDesklet->fButtonsAlpha));
		if (s_pRotateButtonBuffer.iTexture != 0)
		{
			glPushMatrix ();
			glTranslatef (-pDesklet->container.iWidth/2 + myDeskletsParam.iDeskletButtonSize/2,
				pDesklet->container.iHeight/2 - myDeskletsParam.iDeskletButtonSize/2,
				0.);
			_cairo_dock_apply_texture_at_size (s_pRotateButtonBuffer.iTexture, myDeskletsParam.iDeskletButtonSize, myDeskletsParam.iDeskletButtonSize);
			glPopMatrix ();
		}
		if (s_pRetachButtonBuffer.iTexture != 0 && g_pMainDock)
		{
			glPushMatrix ();
			glTranslatef (pDesklet->container.iWidth/2 - myDeskletsParam.iDeskletButtonSize/2,
				pDesklet->container.iHeight/2 - myDeskletsParam.iDeskletButtonSize/2,
				0.);
			_cairo_dock_apply_texture_at_size (s_pRetachButtonBuffer.iTexture, myDeskletsParam.iDeskletButtonSize, myDeskletsParam.iDeskletButtonSize);
			glPopMatrix ();
		}
		if (s_pDepthRotateButtonBuffer.iTexture != 0)
		{
			glPushMatrix ();
			glTranslatef (0.,
				pDesklet->container.iHeight/2 - myDeskletsParam.iDeskletButtonSize/2,
				0.);
			_cairo_dock_apply_texture_at_size (s_pDepthRotateButtonBuffer.iTexture, myDeskletsParam.iDeskletButtonSize, myDeskletsParam.iDeskletButtonSize);
			glPopMatrix ();
			
			glPushMatrix ();
			glRotatef (90., 0., 0., 1.);
			glTranslatef (0.,
				pDesklet->container.iWidth/2 - myDeskletsParam.iDeskletButtonSize/2,
				0.);
			_cairo_dock_apply_texture_at_size (s_pDepthRotateButtonBuffer.iTexture, myDeskletsParam.iDeskletButtonSize, myDeskletsParam.iDeskletButtonSize);
			glPopMatrix ();
		}
	}
	if ((pDesklet->container.bInside || pDesklet->fButtonsAlpha != 0 || pDesklet->bNoInput) && s_pNoInputButtonBuffer.iTexture != 0 && pDesklet->bAllowNoClickable)
	{
		_cairo_dock_set_blend_alpha ();
		_cairo_dock_set_alpha (_no_input_button_alpha(pDesklet));
		glPushMatrix ();
		glTranslatef (pDesklet->container.iWidth/2 - myDeskletsParam.iDeskletButtonSize/2,
			- pDesklet->container.iHeight/2 + myDeskletsParam.iDeskletButtonSize/2,
			0.);
		_cairo_dock_apply_texture_at_size (s_pNoInputButtonBuffer.iTexture, myDeskletsParam.iDeskletButtonSize, myDeskletsParam.iDeskletButtonSize);
		glPopMatrix ();
	}
	
	_cairo_dock_disable_texture ();
	glPopMatrix ();
}

static gboolean _cairo_dock_render_desklet_notification (gpointer pUserData, CairoDesklet *pDesklet, cairo_t *pCairoContext)
{
	if (pCairoContext != NULL)
		_render_desklet_cairo (pDesklet, pCairoContext);
	else
		_render_desklet_opengl (pDesklet);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

static gboolean _cairo_dock_enter_leave_desklet_notification (gpointer data, CairoDesklet *pDesklet, gboolean *bStartAnimation)
{
	pDesklet->bButtonsApparition = TRUE;
	*bStartAnimation = TRUE;
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

static gboolean _cairo_dock_update_desklet_notification (gpointer data, CairoDesklet *pDesklet, gboolean *bContinueAnimation)
{
	if (!pDesklet->bButtonsApparition && !pDesklet->bGrowingUp)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (pDesklet->bButtonsApparition)
	{
		pDesklet->fButtonsAlpha += (pDesklet->container.bInside ? .1 : -.1);
		//g_print ("fButtonsAlpha <- %.2f\n", pDesklet->fButtonsAlpha);
		
		if (pDesklet->fButtonsAlpha <= 0 || pDesklet->fButtonsAlpha >= 1)
		{
			pDesklet->bButtonsApparition = FALSE;
			if (pDesklet->fButtonsAlpha < 0)
				pDesklet->fButtonsAlpha = 0.;
			else if (pDesklet->fButtonsAlpha > 1)
				pDesklet->fButtonsAlpha = 1.;
		}
		else
		{
			*bContinueAnimation = TRUE;
		}
	}
	
	if (pDesklet->bGrowingUp)
	{
		pDesklet->container.fRatio += .04;
		//g_print ("pDesklet->container.fRatio:%.2f\n", pDesklet->container.fRatio);
		
		if (pDesklet->container.fRatio >= 1.1)  // la derniere est a x1.1
		{
			pDesklet->container.fRatio = 1;
			pDesklet->bGrowingUp = FALSE;
		}
		else
		{
			*bContinueAnimation = TRUE;
		}
	}
	
	gtk_widget_queue_draw (pDesklet->container.pWidget);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


  ///////////////
 /// MANAGER ///
///////////////

CairoDesklet *cairo_dock_create_desklet (Icon *pIcon, CairoDeskletAttribute *pAttributes)
{
	CairoDesklet *pDesklet = cairo_dock_new_desklet ();
	pDesklet->pIcon = pIcon;
	cairo_dock_set_icon_container (pIcon, pDesklet);
	if (CAIRO_DOCK_IS_APPLET (pIcon))
		gtk_window_set_title (GTK_WINDOW (pDesklet->container.pWidget), pIcon->pModuleInstance->pModule->pVisitCard->cModuleName);
	
	if (pAttributes != NULL)
		cairo_dock_configure_desklet (pDesklet, pAttributes);
	
	if (s_pRotateButtonBuffer.pSurface == NULL)
	{
		cairo_dock_load_desklet_buttons ();
	}
	
	s_pDeskletList = g_list_prepend (s_pDeskletList, pDesklet);
	
	cairo_dock_notify_on_object (pDesklet, NOTIFICATION_NEW_DESKLET, pDesklet);  // in practice, only the notification on the manager can be caught, since 'pDesklet' is brand new.
	
	return pDesklet;
}

void cairo_dock_destroy_desklet (CairoDesklet *pDesklet)
{
	cairo_dock_free_desklet (pDesklet);
	s_pDeskletList = g_list_remove (s_pDeskletList, pDesklet);
}


CairoDesklet *cairo_dock_foreach_desklet (CairoDockForeachDeskletFunc pCallback, gpointer user_data)
{
	CairoDesklet *pDesklet;
	GList *dl;
	for (dl = s_pDeskletList; dl != NULL; dl = dl->next)
	{
		pDesklet = dl->data;
		if (pCallback (pDesklet, user_data))
			return pDesklet;
	}
	
	return NULL;
}

static gboolean _cairo_dock_foreach_icons_in_desklet (CairoDesklet *pDesklet, gpointer *data)
{
	CairoDockForeachIconFunc pFunction = data[0];
	gpointer pUserData = data[1];
	if (pDesklet->pIcon != NULL)
		pFunction (pDesklet->pIcon, CAIRO_CONTAINER (pDesklet), pUserData);
	Icon *pIcon;
	GList *ic;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		pFunction ((Icon*)ic->data, CAIRO_CONTAINER (pDesklet), pUserData);
	}
	return FALSE;
}
void cairo_dock_foreach_icons_in_desklets (CairoDockForeachIconFunc pFunction, gpointer pUserData)
{
	gpointer data[2] = {pFunction, pUserData};
	cairo_dock_foreach_desklet ((CairoDockForeachDeskletFunc) _cairo_dock_foreach_icons_in_desklet, data);
}


static gboolean _cairo_dock_reload_one_desklet_decorations (CairoDesklet *pDesklet, gpointer data)
{
	gboolean bDefaultThemeOnly = GPOINTER_TO_INT (data);
	
	if (bDefaultThemeOnly)
	{
		if (pDesklet->cDecorationTheme == NULL || strcmp (pDesklet->cDecorationTheme, "default") == 0)
		{
			cd_debug ("on recharge les decorations de ce desklet");
			cairo_dock_load_desklet_decorations (pDesklet);
		}
	}
	else  // tous ceux qui ne sont pas encore charges et qui ont leur taille definitive.
	{
		if (pDesklet->iDesiredWidth == 0 && pDesklet->iDesiredHeight == 0 && pDesklet->iSidWriteSize == 0 && pDesklet->backGroundImageBuffer.pSurface == NULL && pDesklet->foreGroundImageBuffer.pSurface == NULL)
		{
			cd_debug ("ce desklet a saute le chargement de ses deco => on l'aide.");
			cairo_dock_load_desklet_decorations (pDesklet);
		}
	}
	return FALSE;
}
void cairo_dock_reload_desklets_decorations (gboolean bDefaultThemeOnly)  // tous ou bien seulement ceux avec "default".
{
	cd_message ("%s (%d)", __func__, bDefaultThemeOnly);
	CairoDesklet *pDesklet;
	GList *dl;
	for (dl = s_pDeskletList; dl != NULL; dl = dl->next)
	{
		pDesklet = dl->data;
		_cairo_dock_reload_one_desklet_decorations (pDesklet, GINT_TO_POINTER (bDefaultThemeOnly));
	}
}

static void _cairo_dock_set_one_desklet_visible (CairoDesklet *pDesklet, gpointer data)
{
	gboolean bOnWidgetLayerToo = GPOINTER_TO_INT (data);
	Window Xid = gldi_container_get_Xid (CAIRO_CONTAINER (pDesklet));
	//gboolean bIsOnWidgetLayer = cairo_dock_window_is_utility (Xid);
	gboolean bIsOnWidgetLayer = (pDesklet->iVisibility == CAIRO_DESKLET_ON_WIDGET_LAYER);
	if (bOnWidgetLayerToo || ! bIsOnWidgetLayer)
	{
		cd_debug ("%s (%d)", __func__, Xid);
		if (bIsOnWidgetLayer)  // on le passe sur la couche visible.
			cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_NORMAL");
		
		gtk_window_set_keep_below (GTK_WINDOW (pDesklet->container.pWidget), FALSE);
		
		cairo_dock_show_desklet (pDesklet);
	}
}
void cairo_dock_set_all_desklets_visible (gboolean bOnWidgetLayerToo)
{
	cd_debug ("%s (%d)", __func__, bOnWidgetLayerToo);
	CairoDesklet *pDesklet;
	GList *dl;
	for (dl = s_pDeskletList; dl != NULL; dl = dl->next)
	{
		pDesklet = dl->data;
		_cairo_dock_set_one_desklet_visible (pDesklet, GINT_TO_POINTER (bOnWidgetLayerToo));
	}
}

static void _cairo_dock_set_one_desklet_visibility_to_default (CairoDesklet *pDesklet, CairoDockMinimalAppletConfig *pMinimalConfig)
{
	if (pDesklet->pIcon != NULL)
	{
		CairoDockModuleInstance *pInstance = pDesklet->pIcon->pModuleInstance;
		GKeyFile *pKeyFile = cairo_dock_pre_read_module_instance_config (pInstance, pMinimalConfig);
		g_key_file_free (pKeyFile);
		
		cairo_dock_set_desklet_accessibility (pDesklet, pMinimalConfig->deskletAttribute.iVisibility, FALSE);
	}
	pDesklet->bAllowMinimize = FALSE;  /// utile ?...
}
void cairo_dock_set_desklets_visibility_to_default (void)
{
	CairoDockMinimalAppletConfig minimalConfig;
	CairoDesklet *pDesklet;
	GList *dl;
	for (dl = s_pDeskletList; dl != NULL; dl = dl->next)
	{
		pDesklet = dl->data;
		_cairo_dock_set_one_desklet_visibility_to_default (pDesklet, &minimalConfig);
	}
}

CairoDesklet *cairo_dock_get_desklet_by_Xid (Window Xid)
{
	CairoDesklet *pDesklet;
	GList *dl;
	for (dl = s_pDeskletList; dl != NULL; dl = dl->next)
	{
		pDesklet = dl->data;
		if (gldi_container_get_Xid (CAIRO_CONTAINER (pDesklet)) == Xid)
			return pDesklet;
	}
	return NULL;
}


static Icon *_cairo_dock_pick_icon_on_opengl_desklet (CairoDesklet *pDesklet)
{
	GLuint selectBuf[4];
	GLint hits=0;
	GLint viewport[4];
	
	if (! gldi_glx_make_current (CAIRO_CONTAINER (pDesklet)))
		return NULL;
	
	glGetIntegerv (GL_VIEWPORT, viewport);
	glSelectBuffer (4, selectBuf);
	
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);
	
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();
	gluPickMatrix ((GLdouble) pDesklet->container.iMouseX, (GLdouble) (viewport[3] - pDesklet->container.iMouseY), 2.0, 2.0, viewport);
	gluPerspective (60.0, 1.0*(GLfloat)pDesklet->container.iWidth/(GLfloat)pDesklet->container.iHeight, 1., 4*pDesklet->container.iHeight);
	
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glLoadIdentity ();
	
	_cairo_dock_set_desklet_matrix (pDesklet);
	
	if (pDesklet->iLeftSurfaceOffset != 0 || pDesklet->iTopSurfaceOffset != 0 || pDesklet->iRightSurfaceOffset != 0 || pDesklet->iBottomSurfaceOffset != 0)
	{
		glTranslatef ((pDesklet->iLeftSurfaceOffset - pDesklet->iRightSurfaceOffset)/2, (pDesklet->iBottomSurfaceOffset - pDesklet->iTopSurfaceOffset)/2, 0.);
		glScalef (1. - 1.*(pDesklet->iLeftSurfaceOffset + pDesklet->iRightSurfaceOffset) / pDesklet->container.iWidth,
			1. - 1.*(pDesklet->iTopSurfaceOffset + pDesklet->iBottomSurfaceOffset) / pDesklet->container.iHeight,
			1.);
	}
	
	glPolygonMode (GL_FRONT, GL_FILL);
	glColor4f (1., 1., 1., 1.);
	
	pDesklet->iPickedObject = 0;
	if (pDesklet->render_bounding_box != NULL)  // surclasse la fonction du moteur de rendu.
	{
		pDesklet->render_bounding_box (pDesklet);
	}
	else if (pDesklet->pRenderer && pDesklet->pRenderer->render_bounding_box != NULL)
	{
		pDesklet->pRenderer->render_bounding_box (pDesklet);
	}
	else  // on le fait nous-memes a partir des coordonnees des icones.
	{
		glTranslatef (-pDesklet->container.iWidth/2, -pDesklet->container.iHeight/2, 0.);
		
		double x, y, w, h;
		Icon *pIcon;
		
		pIcon = pDesklet->pIcon;
		if (pIcon != NULL && pIcon->iIconTexture != 0)
		{
			w = pIcon->fWidth/2;
			h = pIcon->fHeight/2;
			x = pIcon->fDrawX + w;
			y = pDesklet->container.iHeight - pIcon->fDrawY - h;
			
			glLoadName(pIcon->iIconTexture);
			
			glBegin(GL_QUADS);
			glVertex3f(x-w, y+h, 0.);
			glVertex3f(x+w, y+h, 0.);
			glVertex3f(x+w, y-h, 0.);
			glVertex3f(x-w, y-h, 0.);
			glEnd();
		}
		
		GList *ic;
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->iIconTexture == 0)
				continue;
			
			w = pIcon->fWidth/2;
			h = pIcon->fHeight/2;
			x = pIcon->fDrawX + w;
			y = pDesklet->container.iHeight - pIcon->fDrawY - h;
			
			glLoadName(pIcon->iIconTexture);
			
			glBegin(GL_QUADS);
			glVertex3f(x-w, y+h, 0.);
			glVertex3f(x+w, y+h, 0.);
			glVertex3f(x+w, y-h, 0.);
			glVertex3f(x-w, y-h, 0.);
			glEnd();
		}
	}
	
	glPopName();
	
	hits = glRenderMode (GL_RENDER);

	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix ();
	
	Icon *pFoundIcon = NULL;
	if (hits != 0)
	{
		GLuint id = selectBuf[3];
		Icon *pIcon;
		
		if (pDesklet->render_bounding_box != NULL)
		{
			pDesklet->iPickedObject = id;
			//g_print ("iPickedObject <- %d\n", id);
			pFoundIcon = pDesklet->pIcon;  // il faut mettre qqch, sinon la notification est filtree par la macro CD_APPLET_ON_CLICK_BEGIN.
		}
		else
		{
			pIcon = pDesklet->pIcon;
			if (pIcon != NULL && pIcon->iIconTexture != 0)
			{
				if (pIcon->iIconTexture == id)
				{
					pFoundIcon = pIcon;
				}
			}
			
			if (pFoundIcon == NULL)
			{
				GList *ic;
				for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
				{
					pIcon = ic->data;
					if (pIcon->iIconTexture == id)
					{
						pFoundIcon = pIcon;
						break ;
					}
				}
			}
		}
	}
	
	return pFoundIcon;
}
Icon *cairo_dock_find_clicked_icon_in_desklet (CairoDesklet *pDesklet)
{
	if (g_bUseOpenGL && pDesklet->pRenderer && pDesklet->pRenderer->render_opengl)
	{
		return _cairo_dock_pick_icon_on_opengl_desklet (pDesklet);
	}
	
	int iMouseX = pDesklet->container.iMouseX, iMouseY = pDesklet->container.iMouseY;
	if (fabs (pDesklet->fRotation) > ANGLE_MIN)
	{
		//g_print (" clic en (%d;%d) rotations : %.2frad\n", iMouseX, iMouseY, pDesklet->fRotation);
		double x, y;  // par rapport au centre du desklet.
		x = iMouseX - pDesklet->container.iWidth/2;
		y = pDesklet->container.iHeight/2 - iMouseY;
		
		double r, t;  // coordonnees polaires.
		r = sqrt (x*x + y*y);
		t = atan2 (y, x);
		
		double z = _compute_zoom_for_rotation (pDesklet);
		r /= z;
		
		x = r * cos (t + pDesklet->fRotation);  // la rotation de cairo est dans le sene horaire.
		y = r * sin (t + pDesklet->fRotation);
		
		iMouseX = x + pDesklet->container.iWidth/2;
		iMouseY = pDesklet->container.iHeight/2 - y;
		//g_print (" => (%d;%d)\n", iMouseX, iMouseY);
	}
	pDesklet->iMouseX2d = iMouseX;
	pDesklet->iMouseY2d = iMouseY;
	
	Icon *icon = pDesklet->pIcon;
	g_return_val_if_fail (icon != NULL, NULL);  // peut arriver au tout debut, car on associe l'icone au desklet _apres_ l'avoir cree, et on fait tourner la gtk_main entre-temps (pour le redessiner invisible).
	if (icon->fDrawX < iMouseX && icon->fDrawX + icon->fWidth * icon->fScale > iMouseX && icon->fDrawY < iMouseY && icon->fDrawY + icon->fHeight * icon->fScale > iMouseY)
	{
		return icon;
	}
	
	if (pDesklet->icons != NULL)
	{
		GList* ic;
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			if (icon->fDrawX < iMouseX && icon->fDrawX + icon->fWidth * icon->fScale > iMouseX && icon->fDrawY < iMouseY && icon->fDrawY + icon->fHeight * icon->fScale > iMouseY)
			{
				return icon;
			}
		}
	}
	return NULL;
}

gboolean cairo_dock_desklet_manager_is_ready (void)
{
	static gboolean bReady = FALSE;
	if (!bReady)  // once we are ready, no need to test it again.
	{
		bReady = (time (NULL) > s_iStartupTime + 5);  // 5s delay on startup
	}
	return bReady;
}

  //////////////////
 /// GET CONFIG ///
//////////////////

static gboolean get_config (GKeyFile *pKeyFile, CairoDeskletsParam *pDesklets)
{
	gboolean bFlushConfFileNeeded = FALSE;
	
	pDesklets->cDeskletDecorationsName = cairo_dock_get_string_key_value (pKeyFile, "Desklets", "decorations", &bFlushConfFileNeeded, "dark", NULL, NULL);
	CairoDeskletDecoration *pUserDeskletDecorations = cairo_dock_get_desklet_decoration ("personnal");
	if (pUserDeskletDecorations == NULL)
	{
		pUserDeskletDecorations = g_new0 (CairoDeskletDecoration, 1);
		pUserDeskletDecorations->cDisplayedName = _("_custom decoration_");
		cairo_dock_register_desklet_decoration ("personnal", pUserDeskletDecorations);
	}
	if (pDesklets->cDeskletDecorationsName == NULL || strcmp (pDesklets->cDeskletDecorationsName, "personnal") == 0)
	{
		g_free (pUserDeskletDecorations->cBackGroundImagePath);
		pUserDeskletDecorations->cBackGroundImagePath = cairo_dock_get_string_key_value (pKeyFile, "Desklets", "bg desklet", &bFlushConfFileNeeded, NULL, NULL, NULL);
		g_free (pUserDeskletDecorations->cForeGroundImagePath);
		pUserDeskletDecorations->cForeGroundImagePath = cairo_dock_get_string_key_value (pKeyFile, "Desklets", "fg desklet", &bFlushConfFileNeeded, NULL, NULL, NULL);
		pUserDeskletDecorations->iLoadingModifier = CAIRO_DOCK_FILL_SPACE;
		pUserDeskletDecorations->fBackGroundAlpha = cairo_dock_get_double_key_value (pKeyFile, "Desklets", "bg alpha", &bFlushConfFileNeeded, 1.0, NULL, NULL);
		pUserDeskletDecorations->fForeGroundAlpha = cairo_dock_get_double_key_value (pKeyFile, "Desklets", "fg alpha", &bFlushConfFileNeeded, 1.0, NULL, NULL);
		pUserDeskletDecorations->iLeftMargin = cairo_dock_get_integer_key_value (pKeyFile, "Desklets", "left offset", &bFlushConfFileNeeded, 0, NULL, NULL);
		pUserDeskletDecorations->iTopMargin = cairo_dock_get_integer_key_value (pKeyFile, "Desklets", "top offset", &bFlushConfFileNeeded, 0, NULL, NULL);
		pUserDeskletDecorations->iRightMargin = cairo_dock_get_integer_key_value (pKeyFile, "Desklets", "right offset", &bFlushConfFileNeeded, 0, NULL, NULL);
		pUserDeskletDecorations->iBottomMargin = cairo_dock_get_integer_key_value (pKeyFile, "Desklets", "bottom offset", &bFlushConfFileNeeded, 0, NULL, NULL);
	}
	pDesklets->iDeskletButtonSize = cairo_dock_get_integer_key_value (pKeyFile, "Desklets", "button size", &bFlushConfFileNeeded, 16, NULL, NULL);
	pDesklets->cRotateButtonImage = cairo_dock_get_string_key_value (pKeyFile, "Desklets", "rotate image", &bFlushConfFileNeeded, NULL, NULL, NULL);
	pDesklets->cRetachButtonImage = cairo_dock_get_string_key_value (pKeyFile, "Desklets", "retach image", &bFlushConfFileNeeded, NULL, NULL, NULL);
	pDesklets->cDepthRotateButtonImage = cairo_dock_get_string_key_value (pKeyFile, "Desklets", "depth rotate image", &bFlushConfFileNeeded, NULL, NULL, NULL);
	pDesklets->cNoInputButtonImage = cairo_dock_get_string_key_value (pKeyFile, "Desklets", "no input image", &bFlushConfFileNeeded, NULL, NULL, NULL);
	return bFlushConfFileNeeded;
}


  ////////////////////
 /// RESET CONFIG ///
////////////////////

static void reset_config (CairoDeskletsParam *pDesklets)
{
	g_free (pDesklets->cDeskletDecorationsName);
	g_free (pDesklets->cRotateButtonImage);
	g_free (pDesklets->cRetachButtonImage);
	g_free (pDesklets->cDepthRotateButtonImage);
	g_free (pDesklets->cNoInputButtonImage);
}


  //////////////
 /// RELOAD ///
//////////////

static void reload (CairoDeskletsParam *pPrevDesklets, CairoDeskletsParam *pDesklets)
{
	if (cairo_dock_strings_differ (pPrevDesklets->cRotateButtonImage, pDesklets->cRotateButtonImage) ||
		cairo_dock_strings_differ (pPrevDesklets->cRetachButtonImage, pDesklets->cRetachButtonImage) ||
		cairo_dock_strings_differ (pPrevDesklets->cDepthRotateButtonImage, pDesklets->cDepthRotateButtonImage) ||
		cairo_dock_strings_differ (pPrevDesklets->cNoInputButtonImage, pDesklets->cNoInputButtonImage))
	{
		cairo_dock_unload_desklet_buttons ();
		cairo_dock_load_desklet_buttons ();
	}
	if (cairo_dock_strings_differ (pPrevDesklets->cDeskletDecorationsName, pDesklets->cDeskletDecorationsName))
	{
		cairo_dock_reload_desklets_decorations (TRUE);  // TRUE <=> bDefaultThemeOnly
	}
}


  //////////////
 /// UNLOAD ///
//////////////

static void unload (void)
{
	cairo_dock_unload_desklet_buttons ();
}

  ////////////
 /// INIT ///
////////////

static void init (void)
{
	cairo_dock_register_notification_on_object (&myDeskletsMgr,
		NOTIFICATION_UPDATE,
		(CairoDockNotificationFunc) _cairo_dock_update_desklet_notification,
		CAIRO_DOCK_RUN_FIRST, NULL);
	cairo_dock_register_notification_on_object (&myDeskletsMgr,
		NOTIFICATION_ENTER_DESKLET,
		(CairoDockNotificationFunc) _cairo_dock_enter_leave_desklet_notification,
		CAIRO_DOCK_RUN_FIRST, NULL);
	cairo_dock_register_notification_on_object (&myDeskletsMgr,
		NOTIFICATION_LEAVE_DESKLET,
		(CairoDockNotificationFunc) _cairo_dock_enter_leave_desklet_notification,
		CAIRO_DOCK_RUN_FIRST, NULL);
	cairo_dock_register_notification_on_object (&myDeskletsMgr,
		NOTIFICATION_RENDER,
		(CairoDockNotificationFunc) _cairo_dock_render_desklet_notification,
		CAIRO_DOCK_RUN_FIRST, NULL);
	s_iStartupTime = time (NULL);  // on startup, the WM can take a long time before it has positionned all the desklets. To avoid irrelevant configure events, we set a delay.
}

  ///////////////
 /// MANAGER ///
///////////////

void gldi_register_desklets_manager (void)
{
	// Manager
	memset (&myDeskletsMgr, 0, sizeof (CairoDeskletManager));
	myDeskletsMgr.mgr.cModuleName 	= "Desklets";
	myDeskletsMgr.mgr.init 		= init;
	myDeskletsMgr.mgr.load 		= NULL;  // data are loaded the first time a desklet is created, to avoid create them for nothing.
	myDeskletsMgr.mgr.unload 		= unload;
	myDeskletsMgr.mgr.reload 		= (GldiManagerReloadFunc)reload;
	myDeskletsMgr.mgr.get_config 	= (GldiManagerGetConfigFunc)get_config;
	myDeskletsMgr.mgr.reset_config = (GldiManagerResetConfigFunc)reset_config;
	// Config
	memset (&myDeskletsParam, 0, sizeof (CairoDeskletsParam));
	myDeskletsMgr.mgr.pConfig = (GldiManagerConfigPtr)&myDeskletsParam;
	myDeskletsMgr.mgr.iSizeOfConfig = sizeof (CairoDeskletsParam);
	// data
	memset (&s_pRotateButtonBuffer, 0, sizeof (CairoDockImageBuffer));
	memset (&s_pRetachButtonBuffer, 0, sizeof (CairoDockImageBuffer));
	memset (&s_pDepthRotateButtonBuffer, 0, sizeof (CairoDockImageBuffer));
	memset (&s_pNoInputButtonBuffer, 0, sizeof (CairoDockImageBuffer));
	myDeskletsMgr.mgr.iSizeOfData = 0;
	myDeskletsMgr.mgr.pData = (GldiManagerDataPtr)NULL;
	// signals
	cairo_dock_install_notifications_on_object (&myDeskletsMgr, NB_NOTIFICATIONS_DESKLET);
	gldi_object_set_manager (GLDI_OBJECT (&myDeskletsMgr), GLDI_MANAGER (&myContainersMgr));
	// register
	gldi_register_manager (GLDI_MANAGER(&myDeskletsMgr));
}
