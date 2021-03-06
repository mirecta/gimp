/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpoperationhardlightmode.c
 * Copyright (C) 2008 Michael Natterer <mitch@gimp.org>
 *               2012 Ville Sokk <ville.sokk@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <gegl-plugin.h>

#include "operations-types.h"

#include "gimpoperationhardlightmode.h"


static gboolean gimp_operation_hardlight_mode_process (GeglOperation       *operation,
                                                       void                *in_buf,
                                                       void                *aux_buf,
                                                       void                *aux2_buf,
                                                       void                *out_buf,
                                                       glong                samples,
                                                       const GeglRectangle *roi,
                                                       gint                 level);


G_DEFINE_TYPE (GimpOperationHardlightMode, gimp_operation_hardlight_mode,
               GIMP_TYPE_OPERATION_POINT_LAYER_MODE)


static void
gimp_operation_hardlight_mode_class_init (GimpOperationHardlightModeClass *klass)
{
  GeglOperationClass               *operation_class;
  GeglOperationPointComposer3Class *point_class;

  operation_class = GEGL_OPERATION_CLASS (klass);
  point_class     = GEGL_OPERATION_POINT_COMPOSER3_CLASS (klass);

  gegl_operation_class_set_keys (operation_class,
                                 "name",        "gimp:hardlight-mode",
                                 "description", "GIMP hardlight mode operation",
                                 NULL);

  point_class->process = gimp_operation_hardlight_mode_process;
}

static void
gimp_operation_hardlight_mode_init (GimpOperationHardlightMode *self)
{
}

static gboolean
gimp_operation_hardlight_mode_process (GeglOperation       *operation,
                                       void                *in_buf,
                                       void                *aux_buf,
                                       void                *aux2_buf,
                                       void                *out_buf,
                                       glong                samples,
                                       const GeglRectangle *roi,
                                       gint                 level)
{
  gdouble        opacity  = GIMP_OPERATION_POINT_LAYER_MODE (operation)->opacity;
  gfloat        *in       = in_buf;
  gfloat        *layer    = aux_buf;
  gfloat        *mask     = aux2_buf;
  gfloat        *out      = out_buf;
  const gboolean has_mask = mask != NULL;

  while (samples--)
    {
      gfloat comp_alpha, new_alpha;

      comp_alpha = MIN (in[ALPHA], layer[ALPHA]) * opacity;
      if (has_mask)
        comp_alpha *= *mask;

      new_alpha = in[ALPHA] + (1.0 - in[ALPHA]) * comp_alpha;

      if (comp_alpha && new_alpha)
        {
          gfloat ratio = comp_alpha / new_alpha;
          gint   b;

          for (b = RED; b < ALPHA; b++)
            {
              gfloat comp;

              if (layer[b] > 0.5)
                {
                  comp = (1.0 - in[b]) * (1.0 - (layer[b] - 0.5) * 2.0);
                  comp = MIN (1 - comp, 1);
                }
              else
                {
                  comp = in[b] * (layer[b] * 2.0);
                  comp = MIN (comp, 1.0);
                }

              out[b] = comp * ratio + in[b] * (1.0 - ratio);
            }
        }
      else
        {
          gint b;

          for (b = RED; b < ALPHA; b++)
            {
              out[b] = in[b];
            }
        }

      out[ALPHA] = in[ALPHA];

      in    += 4;
      layer += 4;
      out   += 4;

      if (has_mask)
        mask ++;
    }

  return TRUE;
}
