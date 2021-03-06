/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#ifndef __TEMPLATE_OPTIONS_DIALOG_H__
#define __TEMPLATE_OPTIONS_DIALOG_H__


typedef struct _TemplateOptionsDialog TemplateOptionsDialog;

struct _TemplateOptionsDialog
{
  GtkWidget    *dialog;
  GtkWidget    *editor;

  Gimp         *gimp;
  GimpTemplate *template;
};


TemplateOptionsDialog * template_options_dialog_new (GimpTemplate *template,
                                                     GimpContext  *context,
                                                     GtkWidget    *parent,
                                                     const gchar  *title,
                                                     const gchar  *role,
                                                     const gchar  *stock_id,
                                                     const gchar  *desc,
                                                     const gchar  *help_id);


#endif /* __TEMPLATE_OPTIONS_DIALOG_H__ */
