/*
 * Geeqie
 * (C) 2004 John Ellis
 * Copyright (C) 2008 - 2010 The Geeqie Team
 *
 * Author: Vladimir Nadvornik
 *
 * This software is released under the GNU General Public License (GNU GPL).
 * Please read the included file COPYING for more information.
 * This software comes with no warranty of any kind, use at your own risk!
 */


#ifndef BAR_RATING_H
#define BAR_RATING_H

GtkWidget *bar_pane_rating_new_from_config(const gchar **attribute_names, const gchar **attribute_values);
void bar_pane_rating_update_from_config(GtkWidget *pane, const gchar **attribute_names, const gchar **attribute_values);

#endif
/* vim: set shiftwidth=8 softtabstop=0 cindent cinoptions={1s: */
