/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * Libbrasero-burn
 * Copyright (C) Philippe Rouquier 2005-2009 <bonfire-app@wanadoo.fr>
 *
 * Libbrasero-burn is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The Libbrasero-burn authors hereby grant permission for non-GPL compatible
 * GStreamer plugins to be used and distributed together with GStreamer
 * and Libbrasero-burn. This permission is above and beyond the permissions granted
 * by the GPL license by which Libbrasero-burn is covered. If you modify this code
 * you may extend this exception to your version of the code, but you are not
 * obligated to do so. If you do not wish to do so, delete this exception
 * statement from your version.
 * 
 * Libbrasero-burn is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#include "brasero-misc.h"
#include "brasero-tags.h"
#include "brasero-session.h"
#include "brasero-session-helper.h"
#include "brasero-video-options.h"

typedef struct _BraseroVideoOptionsPrivate BraseroVideoOptionsPrivate;
struct _BraseroVideoOptionsPrivate
{
	BraseroBurnSession *session;

	GtkWidget *video_options;
	GtkWidget *vcd_label;
	GtkWidget *vcd_button;
	GtkWidget *svcd_button;

	GtkWidget *button_4_3;
	GtkWidget *button_16_9;
};

#define BRASERO_VIDEO_OPTIONS_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BRASERO_TYPE_VIDEO_OPTIONS, BraseroVideoOptionsPrivate))

enum {
	PROP_0,
	PROP_SESSION
};

G_DEFINE_TYPE (BraseroVideoOptions, brasero_video_options, GTK_TYPE_ALIGNMENT);

static void
brasero_video_options_audio_AC3 (BraseroVideoOptions *options)
{
	GValue *value = NULL;
	BraseroVideoOptionsPrivate *priv;

	priv = BRASERO_VIDEO_OPTIONS_PRIVATE (options);

	value = g_new0 (GValue, 1);
	g_value_init (value, G_TYPE_INT);
	g_value_set_int (value, BRASERO_AUDIO_FORMAT_AC3);
	brasero_burn_session_tag_add (priv->session,
				      BRASERO_DVD_STREAM_FORMAT,
				      value);
}

static void
brasero_video_options_audio_MP2 (BraseroVideoOptions *options)
{
	GValue *value = NULL;
	BraseroVideoOptionsPrivate *priv;

	priv = BRASERO_VIDEO_OPTIONS_PRIVATE (options);

	value = g_new0 (GValue, 1);
	g_value_init (value, G_TYPE_INT);
	g_value_set_int (value, BRASERO_AUDIO_FORMAT_MP2);
	brasero_burn_session_tag_add (priv->session,
				      BRASERO_DVD_STREAM_FORMAT,
				      value);
}

static void
brasero_video_options_update (BraseroVideoOptions *options)
{
	BraseroVideoOptionsPrivate *priv;
	BraseroMedia media;

	priv = BRASERO_VIDEO_OPTIONS_PRIVATE (options);

	/* means we haven't initialized yet */
	if (!priv->vcd_label)
		return;

	media = brasero_burn_session_get_dest_media (priv->session);
	if (media & BRASERO_MEDIUM_DVD) {
		brasero_video_options_audio_AC3 (options);
		gtk_widget_hide (priv->vcd_label);
		gtk_widget_hide (priv->vcd_button);
		gtk_widget_hide (priv->svcd_button);

		gtk_widget_set_sensitive (priv->button_4_3, TRUE);
		gtk_widget_set_sensitive (priv->button_16_9, TRUE);
	}
	else if (media & BRASERO_MEDIUM_CD) {
		brasero_video_options_audio_MP2 (options);
		gtk_widget_show (priv->vcd_label);
		gtk_widget_show (priv->vcd_button);
		gtk_widget_show (priv->svcd_button);

		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->vcd_button))) {
			gtk_widget_set_sensitive (priv->button_4_3, FALSE);
			gtk_widget_set_sensitive (priv->button_16_9, FALSE);
		}
		else {
			gtk_widget_set_sensitive (priv->button_4_3, TRUE);
			gtk_widget_set_sensitive (priv->button_16_9, TRUE);
		}
	}
	else if (media & BRASERO_MEDIUM_FILE) {
		BraseroImageFormat format;

		/* if we create a CUE file then that's a (S)VCD */
		format = brasero_burn_session_get_output_format (priv->session);
		if (format == BRASERO_IMAGE_FORMAT_NONE)
			return;

		if (format == BRASERO_IMAGE_FORMAT_CUE) {
			brasero_video_options_audio_MP2 (options);
			gtk_widget_show (priv->vcd_label);
			gtk_widget_show (priv->vcd_button);
			gtk_widget_show (priv->svcd_button);

			if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->vcd_button))) {
				gtk_widget_set_sensitive (priv->button_4_3, FALSE);
				gtk_widget_set_sensitive (priv->button_16_9, FALSE);
			}
			else {
				gtk_widget_set_sensitive (priv->button_4_3, TRUE);
				gtk_widget_set_sensitive (priv->button_16_9, TRUE);
			}
		}
		else if (format == BRASERO_IMAGE_FORMAT_BIN) {
			brasero_video_options_audio_AC3 (options);
			gtk_widget_hide (priv->vcd_label);
			gtk_widget_hide (priv->vcd_button);
			gtk_widget_hide (priv->svcd_button);

			gtk_widget_set_sensitive (priv->button_4_3, TRUE);
			gtk_widget_set_sensitive (priv->button_16_9, TRUE);
		}
	}
}

static void
brasero_video_options_set_tag (BraseroVideoOptions *options,
			       const gchar *tag,
			       gint contents)
{
	GValue *value;
	BraseroVideoOptionsPrivate *priv;

	priv = BRASERO_VIDEO_OPTIONS_PRIVATE (options);

	value = g_new0 (GValue, 1);
	g_value_init (value, G_TYPE_INT);
	g_value_set_int (value, contents);
	brasero_burn_session_tag_add (priv->session,
				      tag,
				      value);
}

static void
brasero_video_options_SVCD (GtkToggleButton *button,
			    BraseroVideoOptions *options)
{
	BraseroVideoOptionsPrivate *priv;

	if (!gtk_toggle_button_get_active (button))
		return;

	brasero_video_options_set_tag (options,
				       BRASERO_VCD_TYPE,
				       BRASERO_SVCD);

	priv = BRASERO_VIDEO_OPTIONS_PRIVATE (options);

	gtk_widget_set_sensitive (priv->button_4_3, TRUE);
	gtk_widget_set_sensitive (priv->button_16_9, TRUE);
}

static void
brasero_video_options_VCD (GtkToggleButton *button,
			   BraseroVideoOptions *options)
{
	BraseroVideoOptionsPrivate *priv;

	if (!gtk_toggle_button_get_active (button))
		return;

	brasero_video_options_set_tag (options,
				       BRASERO_VCD_TYPE,
				       BRASERO_VCD_V2);

	priv = BRASERO_VIDEO_OPTIONS_PRIVATE (options);
	gtk_widget_set_sensitive (priv->button_4_3, FALSE);
	gtk_widget_set_sensitive (priv->button_16_9, FALSE);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->button_4_3), TRUE);
}

static void
brasero_video_options_NTSC (GtkToggleButton *button,
			    BraseroVideoOptions *options)
{
	if (!gtk_toggle_button_get_active (button))
		return;

	brasero_video_options_set_tag (options,
				       BRASERO_VIDEO_OUTPUT_FRAMERATE,
				       BRASERO_VIDEO_FRAMERATE_NTSC);
}

static void
brasero_video_options_PAL_SECAM (GtkToggleButton *button,
				 BraseroVideoOptions *options)
{
	if (!gtk_toggle_button_get_active (button))
		return;

	brasero_video_options_set_tag (options,
				       BRASERO_VIDEO_OUTPUT_FRAMERATE,
				       BRASERO_VIDEO_FRAMERATE_PAL_SECAM);
}

static void
brasero_video_options_native_framerate (GtkToggleButton *button,
					BraseroVideoOptions *options)
{
	BraseroVideoOptionsPrivate *priv;

	priv = BRASERO_VIDEO_OPTIONS_PRIVATE (options);
	if (!gtk_toggle_button_get_active (button))
		return;

	brasero_burn_session_tag_remove (priv->session, BRASERO_VIDEO_OUTPUT_FRAMERATE);
}

static void
brasero_video_options_16_9 (GtkToggleButton *button,
			    BraseroVideoOptions *options)
{
	if (!gtk_toggle_button_get_active (button))
		return;

	brasero_video_options_set_tag (options,
				       BRASERO_VIDEO_OUTPUT_ASPECT,
				       BRASERO_VIDEO_ASPECT_16_9);
}

static void
brasero_video_options_4_3 (GtkToggleButton *button,
			   BraseroVideoOptions *options)
{
	if (!gtk_toggle_button_get_active (button))
		return;

	brasero_video_options_set_tag (options,
				       BRASERO_VIDEO_OUTPUT_ASPECT,
				       BRASERO_VIDEO_ASPECT_4_3);
}

static void
brasero_video_options_set_property (GObject *object,
				    guint prop_id,
				    const GValue *value,
				    GParamSpec *pspec)
{
	BraseroVideoOptionsPrivate *priv;

	g_return_if_fail (BRASERO_IS_VIDEO_OPTIONS (object));

	priv = BRASERO_VIDEO_OPTIONS_PRIVATE (object);

	switch (prop_id)
	{
	case PROP_SESSION: /* Readable and only writable at creation time */
		priv->session = BRASERO_BURN_SESSION (g_value_get_object (value));
		g_object_ref (priv->session);
		brasero_video_options_update (BRASERO_VIDEO_OPTIONS(object));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
brasero_video_options_get_property (GObject *object,
				    guint prop_id,
				    GValue *value,
				    GParamSpec *pspec)
{
	BraseroVideoOptionsPrivate *priv;

	g_return_if_fail (BRASERO_IS_VIDEO_OPTIONS (object));

	priv = BRASERO_VIDEO_OPTIONS_PRIVATE (object);

	switch (prop_id)
	{
	case PROP_SESSION:
		g_value_set_object (value, priv->session);
		g_object_ref (priv->session);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
brasero_video_options_init (BraseroVideoOptions *object)
{
	gchar *string;
	GtkWidget *label;
	GtkWidget *table;
	GtkWidget *widget;
	GtkWidget *button1;
	GtkWidget *button2;
	GtkWidget *button3;
	GtkWidget *options;
	BraseroVideoOptionsPrivate *priv;

	priv = BRASERO_VIDEO_OPTIONS_PRIVATE (object);

	widget = gtk_vbox_new (FALSE, 0);

	table = gtk_table_new (3, 4, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), 8);
	gtk_table_set_row_spacings (GTK_TABLE (table), 6);
	gtk_widget_show (table);

	label = gtk_label_new (_("Video format:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_widget_show (label);
	gtk_table_attach (GTK_TABLE (table),
			  label,
			  0, 1,
			  0, 1,
			  GTK_FILL,
			  GTK_FILL,
			  0, 0);

	button1 = gtk_radio_button_new_with_mnemonic (NULL,
						      _("_NTSC"));
	gtk_widget_set_tooltip_text (button1, _("Format used mostly on the North American Continent"));
	g_signal_connect (button1,
			  "toggled",
			  G_CALLBACK (brasero_video_options_NTSC),
			  object);
	gtk_table_attach (GTK_TABLE (table),
			  button1,
			  3, 4,
			  0, 1,
			  GTK_FILL,
			  GTK_FILL,
			  0, 0);

	button2 = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (button1),
								  _("_PAL/SECAM"));
	gtk_widget_set_tooltip_text (button2, _("Format used mostly in Europe"));
	g_signal_connect (button2,
			  "toggled",
			  G_CALLBACK (brasero_video_options_PAL_SECAM),
			  object);
	gtk_table_attach (GTK_TABLE (table),
			  button2,
			  2, 3,
			  0, 1,
			  GTK_FILL,
			  GTK_FILL,
			  0, 0);

	button3 = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (button1),
								  _("Native _format"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button3), TRUE);
	g_signal_connect (button3,
			  "toggled",
			  G_CALLBACK (brasero_video_options_native_framerate),
			  object);
	gtk_table_attach (GTK_TABLE (table),
			  button3,
			  1, 2,
			  0, 1,
			  GTK_FILL,
			  GTK_FILL,
			  0, 0);

	label = gtk_label_new (_("Aspect ratio:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_widget_show (label);
	gtk_table_attach (GTK_TABLE (table),
			  label,
			  0, 1,
			  1, 2,
			  GTK_FILL,
			  GTK_FILL,
			  0, 0);

	button1 = gtk_radio_button_new_with_mnemonic (NULL,
						      _("_4:3"));
	g_signal_connect (button1,
			  "toggled",
			  G_CALLBACK (brasero_video_options_4_3),
			  object);
	gtk_table_attach (GTK_TABLE (table),
			  button1,
			  1, 2,
			  1, 2,
			  GTK_FILL,
			  GTK_FILL,
			  0, 0);
	priv->button_4_3 = button1;

	button2 = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (button1),
								  _("_16:9"));
	g_signal_connect (button2,
			  "toggled",
			  G_CALLBACK (brasero_video_options_16_9),
			  object);
	gtk_table_attach (GTK_TABLE (table),
			  button2,
			  2, 3,
			  1, 2,
			  GTK_FILL,
			  GTK_FILL,
			  0, 0);
	priv->button_16_9 = button2;

	/* Video options for (S)VCD */
	label = gtk_label_new (_("VCD type:"));
	priv->vcd_label = label;

	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_widget_show (label);
	gtk_table_attach (GTK_TABLE (table),
			  label,
			  0, 1,
			  2, 3,
			  GTK_FILL,
			  GTK_FILL,
			  0, 0);

	button1 = gtk_radio_button_new_with_mnemonic_from_widget (NULL, _("Create a SVCD"));
	priv->svcd_button = button1;
	gtk_table_attach (GTK_TABLE (table),
			  button1,
			  1, 2,
			  2, 3,
			  GTK_FILL,
			  GTK_FILL,
			  0, 0);

	g_signal_connect (button1,
			  "clicked",
			  G_CALLBACK (brasero_video_options_SVCD),
			  object);

	button2 = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (button1), _("Create a VCD"));
	priv->vcd_button = button2;
	gtk_table_attach (GTK_TABLE (table),
			  button2,
			  2, 3,
			  2, 3,
			  GTK_FILL,
			  GTK_FILL,
			  0, 0);

	g_signal_connect (button2,
			  "clicked",
			  G_CALLBACK (brasero_video_options_VCD),
			  object);

	string = g_strdup_printf ("<b>%s</b>", _("Video Options"));
	options = brasero_utils_pack_properties (string,
						 table,
						 NULL);
	g_free (string);

	gtk_box_pack_start (GTK_BOX (widget), options, FALSE, FALSE, 0);

	/* NOTE: audio options for DVDs were removed. For SVCD that is MP2 and
	 * for Video DVD even if we have a choice AC3 is the most widespread
	 * audio format. So use AC3 by default. */

	gtk_widget_show_all (widget);
	gtk_container_add (GTK_CONTAINER (object), widget);

	/* Just to make sure our tags are correct in BraseroBurnSession */
	brasero_video_options_set_tag (object,
				       BRASERO_VCD_TYPE,
				       BRASERO_SVCD);
	brasero_video_options_set_tag (object,
				       BRASERO_VIDEO_OUTPUT_ASPECT,
				       BRASERO_VIDEO_ASPECT_4_3);
}

static void
brasero_video_options_finalize (GObject *object)
{
	BraseroVideoOptionsPrivate *priv;

	priv = BRASERO_VIDEO_OPTIONS_PRIVATE (object);
	if (priv->session) {
		g_object_unref (priv->session);
		priv->session = NULL;
	}

	G_OBJECT_CLASS (brasero_video_options_parent_class)->finalize (object);
}

static void
brasero_video_options_class_init (BraseroVideoOptionsClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (BraseroVideoOptionsPrivate));

	object_class->finalize = brasero_video_options_finalize;
	object_class->set_property = brasero_video_options_set_property;
	object_class->get_property = brasero_video_options_get_property;

	g_object_class_install_property (object_class,
					 PROP_SESSION,
					 g_param_spec_object ("session",
							      "The session",
							      "The session to work with",
							      BRASERO_TYPE_BURN_SESSION,
							      G_PARAM_READWRITE|G_PARAM_CONSTRUCT_ONLY));
}

GtkWidget *
brasero_video_options_new (BraseroBurnSession *session)
{
	return g_object_new (BRASERO_TYPE_VIDEO_OPTIONS, "session", session, NULL);
}