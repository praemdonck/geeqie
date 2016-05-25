/*
 * Geeqie
 * (C) 2004 John Ellis
 * Copyright (C) 2008 - 2010 The Geeqie Team
 *
 * Author: John Ellis
 *
 * This software is released under the GNU General Public License (GNU GPL).
 * Please read the included file COPYING for more information.
 * This software comes with no warranty of any kind, use at your own risk!
 */


#include "main.h"
#include "bar_rating.h"
#include "pixbuf_util.h"

#include "bar.h"
#include "metadata.h"
#include "filedata.h"
#include "ui_menu.h"
#include "ui_misc.h"
#include "rcfile.h"
#include "layout.h"


/*
 *-------------------------------------------------------------------
 * keyword / comment utils
 *-------------------------------------------------------------------
 */

typedef struct _PaneRatingData PaneRatingData;
struct _PaneRatingData
{
        PaneData pane;
        GtkWidget *widget;
        FileData *fd;
        gchar *key;
        gint height;
        GtkWidget *rating_stars[6];
        int rating;
};

static void bar_rating_set_stars(PaneRatingData *prd, int rating);

static void bar_pane_rating_update(PaneRatingData *prd)
{
	guint64 rating;

	rating = metadata_read_rating(prd->fd);

	prd->rating = rating;
	bar_rating_set_stars(prd, rating);
}

static void bar_pane_rating_set_selection(PaneRatingData *prd)
{
	GList *list = NULL;
	GList *work;

	list = layout_selection_list(prd->pane.lw);
	list = file_data_process_groups_in_selection(list, FALSE, NULL);
	
	work = list;
	while (work)
		{
		FileData *fd = work->data;
		work = work->next;
		if (fd == prd->fd) continue;
		metadata_write_rating(fd, prd->rating);
		}
	filelist_free(list);
}

static void bar_pane_rating_sel_set_cb(GtkWidget *button, gpointer data)
{
	PaneRatingData *prd = data;
	bar_pane_rating_set_selection(prd);
}

static void bar_pane_rating_set_fd(GtkWidget *bar, FileData *fd)
{
	PaneRatingData *prd;

	prd = g_object_get_data(G_OBJECT(bar), "pane_data");
	if (!prd) return;

	file_data_unref(prd->fd);
	prd->fd = file_data_ref(fd);

	bar_pane_rating_update(prd);
}

static void bar_pane_rating_write_config(GtkWidget *pane, GString *outstr, gint indent)
{
	PaneRatingData *prd;

	prd = g_object_get_data(G_OBJECT(pane), "pane_data");
	if (!prd) return;

	WRITE_NL(); WRITE_STRING("<pane_rating ");
	write_char_option(outstr, indent, "id", prd->pane.id);
	write_char_option(outstr, indent, "title", gtk_label_get_text(GTK_LABEL(prd->pane.title)));
	WRITE_BOOL(prd->pane, expanded);
	WRITE_CHAR(*prd, key);
	WRITE_INT(*prd, height); 
	WRITE_STRING("/>");
}

static void bar_pane_rating_notify_cb(FileData *fd, NotifyType type, gpointer data)
{
	PaneRatingData *prd = data;
	if ((type & (NOTIFY_REREAD | NOTIFY_CHANGE | NOTIFY_METADATA)) && fd == prd->fd) 
		{
		DEBUG_1("Notify pane_rating: %s %04x", fd->path, type);

		bar_pane_rating_update(prd);
		}
}

static void bar_pane_rating_destroy(GtkWidget *widget, gpointer data)
{
	PaneRatingData *prd = data;

	file_data_unregister_notify_func(bar_pane_rating_notify_cb, prd);

	file_data_unref(prd->fd);
	g_free(prd->key);

	g_free(prd->pane.id);

	g_free(prd);
}


static void bar_pane_rating_connect_marks_cb(GtkWidget *widget, gpointer data)
{
	int i;
	log_printf("DBG PABLO bar_pane_rating_connect_marks_cb %d\n", (int)data);
	for (i = 0; i < 5; i++) 
		{
		meta_data_connect_mark_with_rating(i + 1, i);
		}
	meta_data_connect_mark_with_rating(-1, i);
}

static void bar_pane_rating_disconnect_marks_cb(GtkWidget *widget, gpointer data)
{
	int i;
	log_printf("DBG PABLO bar_pane_rating_disconnect_marks_cb %d\n", (int)data);
	for (i = 0; i < 5; i++) 
		{
		meta_data_disconnect_mark_with_rating(i + 1, i);
		}
	meta_data_disconnect_mark_with_rating(-1, i);
}

static GtkWidget *bar_pane_rating_menu(PaneRatingData *prd)
{
	GtkWidget *menu;

	menu = popup_menu_short_lived();

	/* use the same strings as in layout_util.c */
	menu_item_add(menu, _("Connect marks 1 to 6 to Rating"), G_CALLBACK(bar_pane_rating_connect_marks_cb), GINT_TO_POINTER(128) );
	menu_item_add(menu, _("Disconnect marks from Rating"), G_CALLBACK(bar_pane_rating_disconnect_marks_cb), GINT_TO_POINTER(128) );
	menu_item_add_divider(GTK_WIDGET(menu));
	menu_item_add(GTK_WIDGET(menu), _("Set Rating to selected files"), G_CALLBACK(bar_pane_rating_sel_set_cb), prd);

	return menu;
}

static void bar_rating_set_stars(PaneRatingData *prd, int rating)
{
	GdkPixbuf *img_buf;

	int index, i;

	if (rating < -1) rating = -1;
	if (rating > 5) rating = 5;

	if (rating == -1)
		{
		img_buf = pixbuf_inline(PIXBUF_INLINE_CROSS_BLACK); 
		index = 1;
		}
	else
		{
		img_buf = pixbuf_inline(PIXBUF_INLINE_CROSS_GREY); 
		index = rating + 1;
		}

	gtk_image_set_from_pixbuf(GTK_IMAGE(prd->rating_stars[0]), img_buf);
	g_object_unref(img_buf);

	img_buf = pixbuf_inline(PIXBUF_INLINE_STAR_FULL); 
	for (i = 1; i < index; i++)
		{
		gtk_image_set_from_pixbuf(GTK_IMAGE(prd->rating_stars[i]), img_buf);
		}

	g_object_unref(img_buf);

	img_buf = pixbuf_inline(PIXBUF_INLINE_STAR_EMPTY); 
	for ( ; i < 6; i++)
		{
		gtk_image_set_from_pixbuf(GTK_IMAGE(prd->rating_stars[i]), img_buf);
		}
	g_object_unref(img_buf);
}

static void bar_rating_set_rating(PaneRatingData *prd, int rating)
{
	gchar *comment;

	if (!prd->fd) return;

	log_printf("DBG PABLO prd* 0x%x\n", GPOINTER_TO_INT(prd));

	if (rating < -1) rating = -1;
	if (rating > 5) rating = 5;

	prd->rating = rating;

	metadata_write_rating(prd->fd, prd->rating);
}

gboolean evt_box_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	PaneRatingData *prd = g_object_get_data(G_OBJECT(widget), "pane_data");
	if (event->button == MOUSE_BUTTON_LEFT)
		{    
		int rating, evt_index = GPOINTER_TO_INT(data);

		int old_rating;

    rating = evt_index;

		// Reject cross was clicked
		if (evt_index == 0) rating = -1;
		// If image is rejected and and rejected cross already selected, 
		// toggle rating to 0
		if (prd->rating == -1 && evt_index == 0) rating = 0;
		if (prd->rating == evt_index) rating--;


    log_printf("DBG PABLO prd->rating %d, evt_index %d, rating %d \n", prd->rating, evt_index, rating);

		bar_rating_set_rating(prd, rating);
		}

	else if (event->button == MOUSE_BUTTON_RIGHT)
		{
		GtkWidget *menu;

		menu = bar_pane_rating_menu(prd);
		gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event->button, event->time);
		return TRUE;
		}

	return TRUE;
}


static GtkWidget *bar_pane_rating_new(const gchar *id, const gchar *title, const gchar *key, gboolean expanded, gint height)
{
	PaneRatingData *prd;
	GtkWidget *hbox, *evt_box[6];
	GtkTextBuffer *buffer;
	int i;

	prd = g_new0(PaneRatingData, 1);

	prd->pane.pane_set_fd       = bar_pane_rating_set_fd;
	prd->pane.pane_event        = NULL;
	prd->pane.pane_write_config = bar_pane_rating_write_config;
	prd->pane.title             = bar_pane_expander_title(title);
	prd->pane.id                = g_strdup(id);
	prd->pane.type              = PANE_RATING;
	prd->pane.expanded          = expanded;

	prd->key = g_strdup(key);
	prd->height = height;

	hbox = gtk_hbox_new(FALSE, 5);

	for (i = 0; i < 6; i++)
		{
		prd->rating_stars[i] = gtk_image_new();
		evt_box[i] = gtk_event_box_new();
		g_object_set_data(G_OBJECT(evt_box[i]), "pane_data", prd);

		gtk_container_add(GTK_CONTAINER(evt_box[i]), prd->rating_stars[i]);

		gtk_box_pack_start(GTK_BOX(hbox), evt_box[i], FALSE, TRUE, 0);

		//gtk_signal_connect (GTK_OBJECT(evt_box[i]), "button_press_event",
		//		    G_CALLBACK(evt_box_event), GINT_TO_POINTER(i));
		g_signal_connect (G_OBJECT(evt_box[i]), "button_press_event",
				    G_CALLBACK(evt_box_event), GINT_TO_POINTER(i));
		}


	prd->rating = 0;
	bar_rating_set_stars(prd, prd->rating);

	prd->widget = hbox;
	gtk_widget_show_all(hbox);

	g_object_set_data(G_OBJECT(prd->widget), "pane_data", prd);
	g_signal_connect(G_OBJECT(prd->widget), "destroy",
			 G_CALLBACK(bar_pane_rating_destroy), prd);


	file_data_register_notify_func(bar_pane_rating_notify_cb, prd, NOTIFY_PRIORITY_LOW);

	log_printf("DBG PABLO init prd* 0x%x\n", prd);

	return prd->widget;
}

GtkWidget *bar_pane_rating_new_from_config(const gchar **attribute_names, const gchar **attribute_values)
{
	gchar *title = NULL;
	gchar *key = g_strdup(COMMENT_KEY);
	gboolean expanded = TRUE;
	gint height = 50;
	gchar *id = g_strdup("comment");
	GtkWidget *ret;

	while (*attribute_names)
		{
		const gchar *option = *attribute_names++;
		const gchar *value = *attribute_values++;

		if (READ_CHAR_FULL("title", title)) continue;
		if (READ_CHAR_FULL("key", key)) continue;
		if (READ_BOOL_FULL("expanded", expanded)) continue;
		if (READ_INT_FULL("height", height)) continue;
		if (READ_CHAR_FULL("id", id)) continue;
		

		log_printf("unknown attribute %s = %s\n", option, value);
		}

	bar_pane_translate_title(PANE_RATING, id, &title);
	log_printf("DBG PABLO bar_pane_rating_new_from_config\n");
	ret = bar_pane_rating_new(id, title, key, expanded, height);
	g_free(title);
	g_free(key);
	g_free(id);
	return ret;
}

void bar_pane_rating_update_from_config(GtkWidget *pane, const gchar **attribute_names, const gchar **attribute_values)
{
	PaneRatingData *prd;

	prd = g_object_get_data(G_OBJECT(pane), "pane_data");
	if (!prd) return;

	gchar *title = NULL;

	while (*attribute_names)
		{
		const gchar *option = *attribute_names++;
		const gchar *value = *attribute_values++;

		if (READ_CHAR_FULL("title", title)) continue;
		if (READ_CHAR_FULL("key", prd->key)) continue;
		if (READ_BOOL_FULL("expanded", prd->pane.expanded)) continue;
		if (READ_INT_FULL("height", prd->height)) continue;
		if (READ_CHAR_FULL("id", prd->pane.id)) continue;
		
		log_printf("unknown attribute %s = %s\n", option, value);
		}

	log_printf("DBG PABLO bar_pane_rating_update_from_config, title: %s\n", title);
	if (title)
		{
		bar_pane_translate_title(PANE_RATING, prd->pane.id, &title);
		gtk_label_set_text(GTK_LABEL(prd->pane.title), title);
		g_free(title);
		}
	gtk_widget_set_size_request(prd->widget, -1, prd->height);
	bar_update_expander(pane);
	bar_pane_rating_update(prd);
}

/* vim: set shiftwidth=8 softtabstop=0 cindent cinoptions={1s: tabstop=8 noexpandtab */
