/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpdisplayshell-style.c
 * Copyright (C) 2010  Michael Natterer <mitch@gimp.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gegl.h>
#include <gtk/gtk.h>

#include "libgimpcolor/gimpcolor.h"
#include "libgimpwidgets/gimpwidgets.h"

#include "display-types.h"

#include "core/gimpgrid.h"
#include "core/gimplayer.h"
#include "core/gimplayermask.h"

#include "widgets/gimpcairo.h"

#include "gimpdisplayshell.h"
#include "gimpdisplayshell-style.h"


static const GimpRGB guide_normal_fg     = { 0.0, 0.0, 0.0, 1.0 };
static const GimpRGB guide_normal_bg     = { 0.0, 0.5, 1.0, 1.0 };
static const GimpRGB guide_active_fg     = { 0.0, 0.0, 0.0, 1.0 };
static const GimpRGB guide_active_bg     = { 1.0, 0.0, 0.0, 1.0 };

static const GimpRGB sample_point_normal = { 0.0, 0.5, 1.0, 1.0 };
static const GimpRGB sample_point_active = { 1.0, 0.0, 0.0, 1.0 };

static const GimpRGB layer_fg            = { 0.0, 0.0, 0.0, 1.0 };
static const GimpRGB layer_bg            = { 1.0, 1.0, 0.0, 1.0 };

static const GimpRGB layer_group_fg      = { 0.0, 0.0, 0.0, 1.0 };
static const GimpRGB layer_group_bg      = { 0.0, 1.0, 1.0, 1.0 };

static const GimpRGB layer_mask_fg       = { 0.0, 0.0, 0.0, 1.0 };
static const GimpRGB layer_mask_bg       = { 0.0, 1.0, 0.0, 1.0 };

static const GimpRGB selection_out_fg    = { 1.0, 1.0, 1.0, 1.0 };
static const GimpRGB selection_out_bg    = { 0.5, 0.5, 0.5, 1.0 };

static const GimpRGB selection_in_fg     = { 0.0, 0.0, 0.0, 1.0 };
static const GimpRGB selection_in_bg     = { 1.0, 1.0, 1.0, 1.0 };

static const GimpRGB vectors_normal_bg   = { 1.0, 1.0, 1.0, 0.6 };
static const GimpRGB vectors_normal_fg   = { 0.0, 0.0, 1.0, 0.8 };

static const GimpRGB vectors_active_bg   = { 1.0, 1.0, 1.0, 0.6 };
static const GimpRGB vectors_active_fg   = { 1.0, 0.0, 0.0, 0.8 };

static const GimpRGB dim                 = { 0.0, 0.0, 0.0, 0.5 };

static const GimpRGB tool_bg             = { 0.0, 0.0, 0.0, 0.4 };
static const GimpRGB tool_fg             = { 1.0, 1.0, 1.0, 0.8 };
static const GimpRGB tool_fg_highlight   = { 1.0, 0.8, 0.2, 0.8 };


/*  public functions  */

void
gimp_display_shell_set_guide_style (GimpDisplayShell *shell,
                                    cairo_t          *cr,
                                    gboolean          active)
{
  cairo_pattern_t *pattern;

  g_return_if_fail (GIMP_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (cr != NULL);

  cairo_set_line_width (cr, 1.0);

  if (active)
    pattern = gimp_cairo_stipple_pattern_create (&guide_active_fg,
                                                 &guide_active_bg,
                                                 0);
  else
    pattern = gimp_cairo_stipple_pattern_create (&guide_normal_fg,
                                                 &guide_normal_bg,
                                                 0);

  cairo_set_source (cr, pattern);
  cairo_pattern_destroy (pattern);
}

void
gimp_display_shell_set_sample_point_style (GimpDisplayShell *shell,
                                           cairo_t          *cr,
                                           gboolean          active)
{
  g_return_if_fail (GIMP_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (cr != NULL);

  cairo_set_line_width (cr, 1.0);

  if (active)
    cairo_set_source_rgb (cr,
                          sample_point_active.r,
                          sample_point_active.g,
                          sample_point_active.b);
  else
    cairo_set_source_rgb (cr,
                          sample_point_normal.r,
                          sample_point_normal.g,
                          sample_point_normal.b);
}

void
gimp_display_shell_set_grid_style (GimpDisplayShell *shell,
                                   cairo_t          *cr,
                                   GimpGrid         *grid)
{
  g_return_if_fail (GIMP_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (cr != NULL);
  g_return_if_fail (GIMP_IS_GRID (grid));

  cairo_set_line_width (cr, 1.0);

  switch (grid->style)
    {
      cairo_pattern_t *pattern;

    case GIMP_GRID_ON_OFF_DASH:
    case GIMP_GRID_DOUBLE_DASH:
      if (grid->style == GIMP_GRID_DOUBLE_DASH)
        {
          pattern = gimp_cairo_stipple_pattern_create (&grid->fgcolor,
                                                       &grid->bgcolor,
                                                       0);
        }
      else
        {
          GimpRGB bg = { 0.0, 0.0, 0.0, 0.0 };

          pattern = gimp_cairo_stipple_pattern_create (&grid->fgcolor,
                                                       &bg,
                                                       0);
        }

      cairo_set_source (cr, pattern);
      cairo_pattern_destroy (pattern);
      break;

    case GIMP_GRID_DOTS:
    case GIMP_GRID_INTERSECTIONS:
    case GIMP_GRID_SOLID:
      cairo_set_source_rgb (cr,
                            grid->fgcolor.r,
                            grid->fgcolor.g,
                            grid->fgcolor.b);
      break;
    }
}

void
gimp_display_shell_set_pen_style (GimpDisplayShell *shell,
                                  cairo_t          *cr,
                                  const GimpRGB    *color,
                                  gint              width)
{
  g_return_if_fail (GIMP_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (cr != NULL);
  g_return_if_fail (color != NULL);

  cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
  cairo_set_line_width (cr, width);
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);

  gimp_cairo_set_source_rgb (cr, color);
}

void
gimp_display_shell_set_layer_style (GimpDisplayShell *shell,
                                    cairo_t          *cr,
                                    GimpLayer        *layer)
{
  cairo_pattern_t *pattern;

  g_return_if_fail (GIMP_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (cr != NULL);
  g_return_if_fail (GIMP_IS_LAYER (layer));

  cairo_set_line_width (cr, 1.0);
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);

  if (gimp_layer_get_mask (layer) &&
      gimp_layer_mask_get_edit (gimp_layer_get_mask (layer)))
    {
      pattern = gimp_cairo_stipple_pattern_create (&layer_mask_fg,
                                                   &layer_mask_bg,
                                                   0);
    }
  else if (gimp_viewable_get_children (GIMP_VIEWABLE (layer)))
    {
      pattern = gimp_cairo_stipple_pattern_create (&layer_group_fg,
                                                   &layer_group_bg,
                                                   0);
    }
  else
    {
      pattern = gimp_cairo_stipple_pattern_create (&layer_fg,
                                                   &layer_bg,
                                                   0);
    }

  cairo_set_source (cr, pattern);
  cairo_pattern_destroy (pattern);
}

void
gimp_display_shell_set_selection_out_style (GimpDisplayShell *shell,
                                            cairo_t          *cr)
{
  cairo_pattern_t *pattern;

  g_return_if_fail (GIMP_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (cr != NULL);

  cairo_set_line_width (cr, 1.0);
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);

  pattern = gimp_cairo_stipple_pattern_create (&selection_out_fg,
                                               &selection_out_bg,
                                               0);
  cairo_set_source (cr, pattern);
  cairo_pattern_destroy (pattern);
}

void
gimp_display_shell_set_selection_in_style (GimpDisplayShell *shell,
                                           cairo_t          *cr,
                                           gint              index)
{
  cairo_pattern_t *pattern;

  g_return_if_fail (GIMP_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (cr != NULL);

  cairo_set_line_width (cr, 1.0);
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);

  pattern = gimp_cairo_stipple_pattern_create (&selection_in_fg,
                                               &selection_in_bg,
                                               index);
  cairo_set_source (cr, pattern);
  cairo_pattern_destroy (pattern);
}

void
gimp_display_shell_set_vectors_bg_style (GimpDisplayShell *shell,
                                         cairo_t          *cr,
                                         gdouble           width,
                                         gboolean          active)
{
  g_return_if_fail (GIMP_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (cr != NULL);

  cairo_set_line_width (cr, width * 3.0);

  if (active)
    gimp_cairo_set_source_rgba (cr, &vectors_active_bg);
  else
    gimp_cairo_set_source_rgba (cr, &vectors_normal_bg);
}

void
gimp_display_shell_set_vectors_fg_style (GimpDisplayShell *shell,
                                         cairo_t          *cr,
                                         gdouble           width,
                                         gboolean          active)
{
  g_return_if_fail (GIMP_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (cr != NULL);

  cairo_set_line_width (cr, width);

  if (active)
    gimp_cairo_set_source_rgba (cr, &vectors_active_fg);
  else
    gimp_cairo_set_source_rgba (cr, &vectors_normal_fg);
}

void
gimp_display_shell_set_dim_style (GimpDisplayShell *shell,
                                  cairo_t          *cr)
{
  g_return_if_fail (GIMP_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (cr != NULL);

  gimp_cairo_set_source_rgba (cr, &dim);
}

void
gimp_display_shell_set_tool_bg_style (GimpDisplayShell *shell,
                                      cairo_t          *cr)
{
  g_return_if_fail (GIMP_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (cr != NULL);

  cairo_set_line_width (cr, 3.0);
  cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);

  gimp_cairo_set_source_rgba (cr, &tool_bg);
}

void
gimp_display_shell_set_tool_fg_style (GimpDisplayShell *shell,
                                      cairo_t          *cr,
                                      gboolean          highlight)
{
  g_return_if_fail (cr != NULL);

  cairo_set_line_width (cr, 1.0);
  cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);

  if (highlight)
    gimp_cairo_set_source_rgba (cr, &tool_fg_highlight);
  else
    gimp_cairo_set_source_rgba (cr, &tool_fg);
}