/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpcanvaspath.c
 * Copyright (C) 2010 Michael Natterer <mitch@gimp.org>
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

#include "libgimpbase/gimpbase.h"
#include "libgimpmath/gimpmath.h"

#include "display-types.h"

#include "core/gimpparamspecs.h"

#include "vectors/gimpbezierdesc.h"

#include "gimpcanvaspath.h"
#include "gimpdisplayshell.h"
#include "gimpdisplayshell-style.h"
#include "gimpdisplayshell-transform.h"


enum
{
  PROP_0,
  PROP_PATH,
  PROP_FILLED,
  PROP_PATH_STYLE
};


typedef struct _GimpCanvasPathPrivate GimpCanvasPathPrivate;

struct _GimpCanvasPathPrivate
{
  cairo_path_t *path;
  gboolean      filled;
  gboolean      path_style;
};

#define GET_PRIVATE(path) \
        G_TYPE_INSTANCE_GET_PRIVATE (path, \
                                     GIMP_TYPE_CANVAS_PATH, \
                                     GimpCanvasPathPrivate)


/*  local function prototypes  */

static void        gimp_canvas_path_finalize     (GObject          *object);
static void        gimp_canvas_path_set_property (GObject          *object,
                                                  guint             property_id,
                                                  const GValue     *value,
                                                  GParamSpec       *pspec);
static void        gimp_canvas_path_get_property (GObject          *object,
                                                  guint             property_id,
                                                  GValue           *value,
                                                  GParamSpec       *pspec);
static void        gimp_canvas_path_draw         (GimpCanvasItem   *item,
                                                  GimpDisplayShell *shell,
                                                  cairo_t          *cr);
static GdkRegion * gimp_canvas_path_get_extents  (GimpCanvasItem   *item,
                                                  GimpDisplayShell *shell);
static void        gimp_canvas_path_stroke       (GimpCanvasItem   *item,
                                                  GimpDisplayShell *shell,
                                                  cairo_t          *cr);


G_DEFINE_TYPE (GimpCanvasPath, gimp_canvas_path,
               GIMP_TYPE_CANVAS_ITEM)

#define parent_class gimp_canvas_path_parent_class


static void
gimp_canvas_path_class_init (GimpCanvasPathClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  GimpCanvasItemClass *item_class   = GIMP_CANVAS_ITEM_CLASS (klass);

  object_class->finalize     = gimp_canvas_path_finalize;
  object_class->set_property = gimp_canvas_path_set_property;
  object_class->get_property = gimp_canvas_path_get_property;

  item_class->draw           = gimp_canvas_path_draw;
  item_class->get_extents    = gimp_canvas_path_get_extents;
  item_class->stroke         = gimp_canvas_path_stroke;

  g_object_class_install_property (object_class, PROP_PATH,
                                   g_param_spec_pointer ("path", NULL, NULL,
                                                         GIMP_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_FILLED,
                                   g_param_spec_boolean ("filled", NULL, NULL,
                                                         FALSE,
                                                         GIMP_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_PATH_STYLE,
                                   g_param_spec_boolean ("path-style",
                                                         NULL, NULL,
                                                         FALSE,
                                                         GIMP_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (GimpCanvasPathPrivate));
}

static void
gimp_canvas_path_init (GimpCanvasPath *path)
{
}

static void
gimp_canvas_path_finalize (GObject *object)
{
  GimpCanvasPathPrivate *private = GET_PRIVATE (object);

  if (private->path)
    {
      gimp_bezier_desc_free (private->path, TRUE);
      private->path = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gimp_canvas_path_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  GimpCanvasPathPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_PATH:
      {
        cairo_path_t *path = g_value_get_pointer (value);

        if (private->path)
          {
            gimp_bezier_desc_free (private->path, TRUE);
            private->path = NULL;
          }

        if (path)
          private->path = gimp_bezier_desc_copy (path);
      }
      break;
    case PROP_FILLED:
      private->filled = g_value_get_boolean (value);
      break;
    case PROP_PATH_STYLE:
      private->path_style = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gimp_canvas_path_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  GimpCanvasPathPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_PATH:
      g_value_set_pointer (value, private->path);
      break;
    case PROP_FILLED:
      g_value_set_boolean (value, private->filled);
      break;
    case PROP_PATH_STYLE:
      g_value_set_boolean (value, private->path_style);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gimp_canvas_path_draw (GimpCanvasItem   *item,
                       GimpDisplayShell *shell,
                       cairo_t          *cr)
{
  GimpCanvasPathPrivate *private = GET_PRIVATE (item);

  if (private->path)
    {
      cairo_save (cr);
      cairo_translate (cr, - shell->offset_x, - shell->offset_y);
      cairo_scale (cr, shell->scale_x, shell->scale_y);

      cairo_append_path (cr, private->path);
      cairo_restore (cr);

      if (private->filled)
        _gimp_canvas_item_fill (item, cr);
      else
        _gimp_canvas_item_stroke (item, cr);
    }
}

static GdkRegion *
gimp_canvas_path_get_extents (GimpCanvasItem   *item,
                              GimpDisplayShell *shell)
{
  GimpCanvasPathPrivate *private = GET_PRIVATE (item);

  if (private->path)
    {
      cairo_t      *cr;
      GdkRectangle  rectangle;
      gdouble       x1, y1, x2, y2;

      cr = gdk_cairo_create (gtk_widget_get_window (shell->canvas));

      cairo_save (cr);
      cairo_translate (cr, - shell->offset_x, - shell->offset_y);
      cairo_scale (cr, shell->scale_x, shell->scale_y);

      cairo_append_path (cr, private->path);
      cairo_restore (cr);

      cairo_path_extents (cr, &x1, &y1, &x2, &y2);

      cairo_destroy (cr);

      if (private->filled)
        {
          rectangle.x      = floor (x1 - 1.0);
          rectangle.y      = floor (y1 - 1.0);
          rectangle.width  = ceil (x2 + 1.0) - rectangle.x;
          rectangle.height = ceil (y2 + 1.0) - rectangle.y;
        }
      else
        {
          rectangle.x      = floor (x1 - 1.5);
          rectangle.y      = floor (y1 - 1.5);
          rectangle.width  = ceil (x2 + 1.5) - rectangle.x;
          rectangle.height = ceil (y2 + 1.5) - rectangle.y;
        }

      return gdk_region_rectangle (&rectangle);
    }

  return NULL;
}

static void
gimp_canvas_path_stroke (GimpCanvasItem   *item,
                         GimpDisplayShell *shell,
                         cairo_t          *cr)
{
  GimpCanvasPathPrivate *private = GET_PRIVATE (item);

  if (private->path_style)
    {
      gboolean active = gimp_canvas_item_get_highlight (item);

      gimp_display_shell_set_vectors_bg_style (shell, cr, active);
      cairo_stroke_preserve (cr);

      gimp_display_shell_set_vectors_fg_style (shell, cr, active);
      cairo_stroke (cr);
    }
  else
    {
      GIMP_CANVAS_ITEM_CLASS (parent_class)->stroke (item, shell, cr);
    }
}

GimpCanvasItem *
gimp_canvas_path_new (GimpDisplayShell     *shell,
                      const GimpBezierDesc *path,
                      gboolean              filled,
                      gboolean              path_style)
{
  g_return_val_if_fail (GIMP_IS_DISPLAY_SHELL (shell), NULL);

  return g_object_new (GIMP_TYPE_CANVAS_PATH,
                       "shell",      shell,
                       "path",       path,
                       "filled",     filled,
                       "path-style", path_style,
                       NULL);
}