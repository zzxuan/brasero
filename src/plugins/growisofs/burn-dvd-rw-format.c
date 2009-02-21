/***************************************************************************
 *            dvd-rw-format.c
 *
 *  sam fév  4 13:50:07 2006
 *  Copyright  2006  Rouquier Philippe
 *  brasero-app@wanadoo.fr
 ***************************************************************************/

/*
 *  Brasero is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Brasero is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n-lib.h>

#include <gmodule.h>

#include "burn-basics.h"
#include "burn-plugin.h"
#include "burn-job.h"
#include "burn-process.h"
#include "brasero-medium.h"
#include "burn-dvd-rw-format.h"
#include "burn-growisofs-common.h"

BRASERO_PLUGIN_BOILERPLATE (BraseroDvdRwFormat, brasero_dvd_rw_format, BRASERO_TYPE_PROCESS, BraseroProcess);

static GObjectClass *parent_class = NULL;

static BraseroBurnResult
brasero_dvd_rw_format_read_stderr (BraseroProcess *process, const gchar *line)
{
	int perc_1 = 0, perc_2 = 0;
	float percent;

	if (strstr (line, "unable to proceed with format")
	||  strstr (line, "media is not blank")
	||  strstr (line, "media is already formatted")
	||  strstr (line, "you have the option to re-run")) {
		brasero_job_error (BRASERO_JOB (process),
				   g_error_new (BRASERO_BURN_ERROR,
						BRASERO_BURN_ERROR_MEDIUM_INVALID,
						_("The disc is not supported")));
		return BRASERO_BURN_OK;
	}
	else if (strstr (line, "unable to umount")) {
		brasero_job_error (BRASERO_JOB (process),
				   g_error_new (BRASERO_BURN_ERROR,
						BRASERO_BURN_ERROR_DRIVE_BUSY,
						_("The drive is busy")));
		return BRASERO_BURN_OK;
	}

	if ((sscanf (line, "* blanking %d.%1d%%,", &perc_1, &perc_2) == 2)
	||  (sscanf (line, "* formatting %d.%1d%%,", &perc_1, &perc_2) == 2)
	||  (sscanf (line, "* relocating lead-out %d.%1d%%,", &perc_1, &perc_2) == 2))
		brasero_job_set_dangerous (BRASERO_JOB (process), TRUE);
	else 
		sscanf (line, "%d.%1d%%", &perc_1, &perc_2);

	percent = (float) perc_1 / 100.0 + (float) perc_2 / 1000.0;
	if (percent) {
		brasero_job_start_progress (BRASERO_JOB (process), FALSE);
		brasero_job_set_progress (BRASERO_JOB (process), percent);
	}

	return BRASERO_BURN_OK;
}

static BraseroBurnResult
brasero_dvd_rw_format_set_argv (BraseroProcess *process,
				GPtrArray *argv,
				GError **error)
{
	BraseroMedia media;
	BraseroBurnFlag flags;
	gchar *device;

	g_ptr_array_add (argv, g_strdup ("dvd+rw-format"));

	/* undocumented option to show progress */
	g_ptr_array_add (argv, g_strdup ("-gui"));

	brasero_job_get_media (BRASERO_JOB (process), &media);
	brasero_job_get_flags (BRASERO_JOB (process), &flags);
        if (!BRASERO_MEDIUM_IS (media, BRASERO_MEDIUM_BDRE)
	&&  !BRASERO_MEDIUM_IS (media, BRASERO_MEDIUM_DVDRW_PLUS)
	&&  !BRASERO_MEDIUM_IS (media, BRASERO_MEDIUM_DVDRW_RESTRICTED)) {
		gchar *blank_str;

		/* This creates a sequential DVD-RW */
		blank_str = g_strdup_printf ("-blank%s",
					     (flags & BRASERO_BURN_FLAG_FAST_BLANK) ? "" : "=full");
		g_ptr_array_add (argv, blank_str);
	}
	else {
		gchar *format_str;

		/* This creates a restricted overwrite DVD-RW or reformat a + */
		format_str = g_strdup ("-force");
		g_ptr_array_add (argv, format_str);
	}

	brasero_job_get_device (BRASERO_JOB (process), &device);
	g_ptr_array_add (argv, device);

	brasero_job_set_current_action (BRASERO_JOB (process),
					BRASERO_BURN_ACTION_BLANKING,
					NULL,
					FALSE);
	return BRASERO_BURN_OK;
}

static void
brasero_dvd_rw_format_class_init (BraseroDvdRwFormatClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	BraseroProcessClass *process_class = BRASERO_PROCESS_CLASS (klass);

	parent_class = g_type_class_peek_parent(klass);
	object_class->finalize = brasero_dvd_rw_format_finalize;

	process_class->set_argv = brasero_dvd_rw_format_set_argv;
	process_class->stderr_func = brasero_dvd_rw_format_read_stderr;
	process_class->post = brasero_job_finished_session;
}

static void
brasero_dvd_rw_format_init (BraseroDvdRwFormat *obj)
{ }

static void
brasero_dvd_rw_format_finalize (GObject *object)
{
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static BraseroBurnResult
brasero_dvd_rw_format_export_caps (BraseroPlugin *plugin, gchar **error)
{
	/* NOTE: sequential and restricted are added later on demand */
	const BraseroMedia media = BRASERO_MEDIUM_DVD|
				   BRASERO_MEDIUM_DUAL_L|
				   BRASERO_MEDIUM_REWRITABLE|
				   BRASERO_MEDIUM_APPENDABLE|
				   BRASERO_MEDIUM_CLOSED|
				   BRASERO_MEDIUM_HAS_DATA|
				   BRASERO_MEDIUM_UNFORMATTED|
				   BRASERO_MEDIUM_BLANK;
	BraseroBurnResult result;
	GSList *output;

	brasero_plugin_define (plugin,
			       "dvd+rw-format",
			       _("Dvd+rw-format erases and formats DVD+/-R(W)"),
			       "Philippe Rouquier",
			       4);

	/* First see if this plugin can be used */
	result = brasero_process_check_path ("dvd+rw-format", error);
	if (result != BRASERO_BURN_OK)
		return result;

	output = brasero_caps_disc_new (media|
					BRASERO_MEDIUM_BDRE|
					BRASERO_MEDIUM_PLUS|
					BRASERO_MEDIUM_RESTRICTED|
					BRASERO_MEDIUM_SEQUENTIAL);
	brasero_plugin_blank_caps (plugin, output);
	g_slist_free (output);

	brasero_plugin_set_blank_flags (plugin,
					media|
					BRASERO_MEDIUM_BDRE|
					BRASERO_MEDIUM_PLUS|
					BRASERO_MEDIUM_RESTRICTED,
					BRASERO_BURN_FLAG_NOGRACE,
					BRASERO_BURN_FLAG_NONE);
	brasero_plugin_set_blank_flags (plugin,
					media|
					BRASERO_MEDIUM_SEQUENTIAL,
					BRASERO_BURN_FLAG_NOGRACE|
					BRASERO_BURN_FLAG_FAST_BLANK,
					BRASERO_BURN_FLAG_NONE);

	brasero_plugin_register_group (plugin, _(GROWISOFS_DESCRIPTION));

	return BRASERO_BURN_OK;
}
