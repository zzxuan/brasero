/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * Libbrasero-media
 * Copyright (C) Philippe Rouquier 2005-2009 <bonfire-app@wanadoo.fr>
 *
 * Libbrasero-media is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The Libbrasero-media authors hereby grant permission for non-GPL compatible
 * GStreamer plugins to be used and distributed together with GStreamer
 * and Libbrasero-media. This permission is above and beyond the permissions granted
 * by the GPL license by which Libbrasero-media is covered. If you modify this code
 * you may extend this exception to your version of the code, but you are not
 * obligated to do so. If you do not wish to do so, delete this exception
 * statement from your version.
 * 
 * Libbrasero-media is distributed in the hope that it will be useful,
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

#include <glib.h>

#include "scsi-base.h"
#include "scsi-error.h"
#include "scsi-device.h"
#include "scsi-read-capacity.h"
#include "scsi-get-configuration.h"
#include "scsi-read-disc-structure.h"
#include "scsi-read-format-capacities.h"

#ifndef _SCSI_MMC2_H
#define _SCSI_MMC2_H

G_BEGIN_DECLS

BraseroScsiResult
brasero_mmc2_get_profile (BraseroDeviceHandle *handle,
			  BraseroScsiProfile *profile,
			  BraseroScsiErrCode *error);

BraseroScsiResult
brasero_mmc2_read_capacity (BraseroDeviceHandle *handle,
			    BraseroScsiReadCapacityData *data,
			    int size,
			    BraseroScsiErrCode *error);

BraseroScsiResult
brasero_mmc2_get_configuration_feature (BraseroDeviceHandle *handle,
					BraseroScsiFeatureType type,
					BraseroScsiGetConfigHdr **data,
					int *size,
					BraseroScsiErrCode *error);

BraseroScsiResult
brasero_mmc2_read_generic_structure (BraseroDeviceHandle *handle,
				     BraseroScsiGenericFormatType type,
				     BraseroScsiReadDiscStructureHdr **data,
				     int *size,
				     BraseroScsiErrCode *error);

BraseroScsiResult
brasero_mmc2_read_format_capacities (BraseroDeviceHandle *handle,
				     BraseroScsiFormatCapacitiesHdr **data,
				     int *size,
				     BraseroScsiErrCode *error);
G_END_DECLS

#endif /* _SCSI_MMC2_H */

 
