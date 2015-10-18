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
#include "bar_comment.h"
#include "bar_rating.h"
#include "pixbuf_util.h"

#include "bar.h"
#include "metadata.h"
#include "filedata.h"
#include "ui_menu.h"
#include "ui_misc.h"
#include "rcfile.h"
#include "layout.h"

static void bar_pane_comment_changed(GtkTextBuffer *buffer, gpointer data);

/*
 *-------------------------------------------------------------------
 * keyword / comment utils
 *-------------------------------------------------------------------
 */



typedef struct _PaneCommentData PaneCommentData;
struct _PaneCommentData
{
	PaneData pane;
	GtkWidget *widget;
	GtkWidget *comment_view;
	FileData *fd;
	gchar *key;
	gint height;
  GtkWidget *rating_stars[6];
  int rating;
};


static void bar_pane_comment_write(PaneCommentData *pcd)
{
	gchar *comment;

	if (!pcd->fd) return;

	comment = text_widget_text_pull(pcd->comment_view);

	metadata_write_string(pcd->fd, pcd->key, comment);
	g_free(comment);
}


static void bar_pane_comment_update(PaneCommentData *pcd)
{
	gchar *comment = NULL;
	GtkTextBuffer *comment_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pcd->comment_view));

	g_signal_handlers_block_by_func(comment_buffer, bar_pane_comment_changed, pcd);

	comment = metadata_read_string(pcd->fd, pcd->key, METADATA_PLAIN);
	gtk_text_buffer_set_text(comment_buffer,
				 (comment) ? comment : "", -1);
	g_free(comment);
	
	g_signal_handlers_unblock_by_func(comment_buffer, bar_pane_comment_changed, pcd);

	gtk_widget_set_sensitive(pcd->comment_view, (pcd->fd != NULL));
}

static void bar_pane_comment_set_selection(PaneCommentData *pcd, gboolean append)
{
	GList *list = NULL;
	GList *work;
	gchar *comment = NULL;

	comment = text_widget_text_pull(pcd->comment_view);

	list = layout_selection_list(pcd->pane.lw);
	list = file_data_process_groups_in_selection(list, FALSE, NULL);
	
	work = list;
	while (work)
		{
		FileData *fd = work->data;
		work = work->next;
		if (fd == pcd->fd) continue;

		if (append)
			{
			metadata_append_string(fd, pcd->key, comment);
			}
		else
			{
			metadata_write_string(fd, pcd->key, comment);
			}
		}

	filelist_free(list);
	g_free(comment);
}

static void bar_pane_comment_sel_add_cb(GtkWidget *button, gpointer data)
{
	PaneCommentData *pcd = data;

	bar_pane_comment_set_selection(pcd, TRUE);
}

static void bar_pane_comment_sel_replace_cb(GtkWidget *button, gpointer data)
{
	PaneCommentData *pcd = data;

	bar_pane_comment_set_selection(pcd, FALSE);
}


static void bar_pane_comment_set_fd(GtkWidget *bar, FileData *fd)
{
	PaneCommentData *pcd;

	pcd = g_object_get_data(G_OBJECT(bar), "pane_data");
	if (!pcd) return;

	file_data_unref(pcd->fd);
	pcd->fd = file_data_ref(fd);

	bar_pane_comment_update(pcd);
}

static gint bar_pane_comment_event(GtkWidget *bar, GdkEvent *event)
{
	PaneCommentData *pcd;

	pcd = g_object_get_data(G_OBJECT(bar), "pane_data");
	if (!pcd) return FALSE;

#if GTK_CHECK_VERSION(2,20,0)
	if (gtk_widget_has_focus(pcd->comment_view)) return gtk_widget_event(pcd->comment_view, event);
#else
	if (GTK_WIDGET_HAS_FOCUS(pcd->comment_view)) return gtk_widget_event(pcd->comment_view, event);
#endif

	return FALSE;
}

static void bar_pane_comment_write_config(GtkWidget *pane, GString *outstr, gint indent)
{
	PaneCommentData *pcd;

	pcd = g_object_get_data(G_OBJECT(pane), "pane_data");
	if (!pcd) return;

	WRITE_NL(); WRITE_STRING("<pane_rating ");
	write_char_option(outstr, indent, "id", pcd->pane.id);
	write_char_option(outstr, indent, "title", gtk_label_get_text(GTK_LABEL(pcd->pane.title)));
	WRITE_BOOL(pcd->pane, expanded);
	WRITE_CHAR(*pcd, key);
	WRITE_INT(*pcd, height); 
	WRITE_STRING("/>");
}

static void bar_pane_comment_notify_cb(FileData *fd, NotifyType type, gpointer data)
{
	PaneCommentData *pcd = data;
	if ((type & (NOTIFY_REREAD | NOTIFY_CHANGE | NOTIFY_METADATA)) && fd == pcd->fd) 
		{
		DEBUG_1("Notify pane_comment: %s %04x", fd->path, type);

		bar_pane_comment_update(pcd);
		}
}

static void bar_pane_comment_changed(GtkTextBuffer *buffer, gpointer data)
{
	PaneCommentData *pcd = data;

	file_data_unregister_notify_func(bar_pane_comment_notify_cb, pcd);
	bar_pane_comment_write(pcd);
	file_data_register_notify_func(bar_pane_comment_notify_cb, pcd, NOTIFY_PRIORITY_LOW);
}


static void bar_pane_comment_populate_popup(GtkTextView *textview, GtkMenu *menu, gpointer data)
{
	PaneCommentData *pcd = data;

	menu_item_add_divider(GTK_WIDGET(menu));
	menu_item_add_stock(GTK_WIDGET(menu), _("Add text to selected files"), GTK_STOCK_ADD, G_CALLBACK(bar_pane_comment_sel_add_cb), pcd);
	menu_item_add_stock(GTK_WIDGET(menu), _("Replace existing text in selected files"), GTK_STOCK_CONVERT, G_CALLBACK(bar_pane_comment_sel_replace_cb), data);
}

#if 0
static void bar_pane_comment_close(GtkWidget *bar)
{
	PaneCommentData *pcd;

	pcd = g_object_get_data(G_OBJECT(bar), "pane_data");
	if (!pcd) return;

	gtk_widget_destroy(pcd->comment_view);
}
#endif

static void bar_pane_comment_destroy(GtkWidget *widget, gpointer data)
{
	PaneCommentData *pcd = data;

	file_data_unregister_notify_func(bar_pane_comment_notify_cb, pcd);

	file_data_unref(pcd->fd);
	g_free(pcd->key);

	g_free(pcd->pane.id);

	g_free(pcd);
}



enum {
    
  COL_DISPLAY_NAME,
  COL_PIXBUF,
  NUM_COLS
};

GtkTreeModel *init_model(void) {
    
  GtkListStore *list_store;
  GdkPixbuf *p1, *p2, *p3, *p4;
  GtkTreeIter iter;
  GError *err = NULL;

  p1 = pixbuf_inline(PIXBUF_INLINE_FOLDER_CLOSED); 
  p2 = pixbuf_inline(PIXBUF_INLINE_FOLDER_LOCKED);
  p3 = pixbuf_inline(PIXBUF_INLINE_FOLDER_OPEN);
  p4 = pixbuf_inline(PIXBUF_INLINE_FOLDER_UP);


  //p1 = gdk_pixbuf_new_from_file("ubuntu.png", &err); 
  //p2 = gdk_pixbuf_new_from_file("gnumeric.png", &err);
  //p3 = gdk_pixbuf_new_from_file("blender.png", &err);
  //p4 = gdk_pixbuf_new_from_file("inkscape.png", &err);

  //assert(err==NULL);    

  list_store = gtk_list_store_new(NUM_COLS, 
      G_TYPE_STRING, GDK_TYPE_PIXBUF);
      
  gtk_list_store_append(list_store, &iter);
  gtk_list_store_set(list_store, &iter, COL_DISPLAY_NAME, "Ubuntu", COL_PIXBUF, p1, -1);

  gtk_list_store_append(list_store, &iter);
  gtk_list_store_set(list_store, &iter, COL_DISPLAY_NAME, "Gnumeric", COL_PIXBUF, p2, -1);

  gtk_list_store_append(list_store, &iter);
  gtk_list_store_set(list_store, &iter, COL_DISPLAY_NAME, "Blender", COL_PIXBUF, p3, -1);

  gtk_list_store_append(list_store, &iter);
  gtk_list_store_set(list_store, &iter, COL_DISPLAY_NAME, "Inkscape", COL_PIXBUF, p4, -1);
  
  g_object_unref(p1);
  g_object_unref(p2);
  g_object_unref(p3);
  g_object_unref(p4);    

  return GTK_TREE_MODEL(list_store);
}


static set_rating_stars(PaneCommentData *pcd, int rating)
{
  GdkPixbuf *img_buf;
  int index, i;

  if (rating < -1) rating = -1;
  if (rating > 5) rating = 5;

  if (rating == -1)
  {
    img_buf = pixbuf_inline(PIXBUF_INLINE_FOLDER_LOCKED); 
    index = 1;
  }
  else
  {
    img_buf = pixbuf_inline(PIXBUF_INLINE_FOLDER_CLOSED); 
    index = rating + 1;
  }
  gtk_image_set_from_pixbuf(GTK_IMAGE(pcd->rating_stars[0]), img_buf);
  g_object_unref(img_buf);

  img_buf = pixbuf_inline(PIXBUF_INLINE_STAR_FULL); 
  for (i = 1; i < index; i++)
  {
    gtk_image_set_from_pixbuf(GTK_IMAGE(pcd->rating_stars[i]), img_buf);
  }

  
  log_printf("DBG PABLO index %d, i %d\n", index, i);

  g_object_unref(img_buf);

  img_buf = pixbuf_inline(PIXBUF_INLINE_STAR_EMPTY); 
  for ( ; i < 6; i++)
  {
    gtk_image_set_from_pixbuf(GTK_IMAGE(pcd->rating_stars[i]), img_buf);
  }
  g_object_unref(img_buf);
}

gboolean evt_box_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	//PaneCommentData *pcd = (PaneCommentData*)g_object_get_data(G_OBJECT(widget), "pane_data");
	PaneCommentData *pcd = g_object_get_data(G_OBJECT(widget), "pane_data");
  int rating, old_rating;

  old_rating = pcd->rating;

  log_printf("DBG PABLO evt_box_event, type :%d, button: %d, data %d\n", event->type, event->button, GPOINTER_TO_INT(data));
  log_printf("DBG PABLO pcd* 0x%x\n", pcd);
  //log_printf("DBG PABLO evt_box_event, x :%f, y: %f, x_root: %f, y_root: %f\n", event->x, event->y, event->x_root, event->y_root);


  //log_printf("GDK_BUTTON_PRESS %d, GDK_2BUTTON_PRESS %d, GDK_3BUTTON_PRESS %d, GDK_BUTTON_RELEASE %d\n", GDK_BUTTON_PRESS, GDK_2BUTTON_PRESS, GDK_3BUTTON_PRESS, GDK_BUTTON_RELEASE);
  rating = GPOINTER_TO_INT(data);
  if (rating == 0) rating = -1;
  if (old_rating == -1 && rating == -1) rating = 0;
  if (old_rating == rating) rating--;

  pcd->rating = rating;
  set_rating_stars(pcd, rating);
  

  return TRUE;
}


static GtkWidget *bar_pane_comment_new(const gchar *id, const gchar *title, const gchar *key, gboolean expanded, gint height)
{
	PaneCommentData *pcd;
	GtkWidget *scrolled;
	GtkWidget *hbox, *vbox, *evt_box[6];
	GtkTextBuffer *buffer;
	GtkWidget *temp_text;
  //GtkWidget *icon_view, *icon_view_1, *icon_view_2;
  GdkPixbuf *p1;
  GdkPixbuf *p2;
  int i;

	pcd = g_new0(PaneCommentData, 1);
	
	pcd->pane.pane_set_fd = bar_pane_comment_set_fd;
	pcd->pane.pane_event = bar_pane_comment_event;
	pcd->pane.pane_write_config = bar_pane_comment_write_config;
	pcd->pane.title = bar_pane_expander_title(title);
	pcd->pane.id = g_strdup(id);
	pcd->pane.type = PANE_RATING;

	pcd->pane.expanded = expanded;
	
	pcd->key = g_strdup(key);
	pcd->height = height;

	scrolled = gtk_scrolled_window_new(NULL, NULL);
	
  hbox = gtk_hbox_new(FALSE, 5);
  vbox = gtk_vbox_new(TRUE, 10);

	pcd->comment_view = gtk_text_view_new();
	temp_text = gtk_text_view_new();
 
  //icon_view = gtk_icon_view_new_with_model(init_model());
  //gtk_icon_view_set_text_column(GTK_ICON_VIEW(icon_view), COL_DISPLAY_NAME);
  //gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(icon_view), COL_PIXBUF);
  //gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(icon_view), GTK_SELECTION_MULTIPLE);
  p1 = pixbuf_inline(PIXBUF_INLINE_STAR_EMPTY); 
  p2 = pixbuf_inline(PIXBUF_INLINE_STAR_FULL); 
 
  for (i = 0; i < 6; i++)
  {
    pcd->rating_stars[i] = gtk_image_new_from_pixbuf(p1);
    evt_box[i] = gtk_event_box_new();
    g_object_set_data(G_OBJECT(evt_box[i]), "pane_data", pcd);

    gtk_container_add(GTK_CONTAINER(evt_box[i]), pcd->rating_stars[i]);

    gtk_box_pack_start(GTK_BOX(hbox), evt_box[i], FALSE, TRUE, 0);

    gtk_signal_connect (GTK_OBJECT(evt_box[i]), "button_press_event",
                        G_CALLBACK(evt_box_event), GINT_TO_POINTER(i));
  }
  
   
  
 // icon_view_1 = gtk_image_new_from_pixbuf(p1);
 // icon_view_2 = gtk_image_new_from_pixbuf(p2);
  g_object_unref(p1);
  g_object_unref(p2);

  pcd->rating = 0;
  set_rating_stars(pcd, pcd->rating);

  /* And bind an action to it */
  //gtk_widget_set_events (evt_box, GDK_BUTTON_PRESS_MASK /*|GDK_BUTTON_RELEASE_MASK*/);
  //gtk_signal_connect (GTK_OBJECT(evt_box), "button_release_event",
  //                    G_CALLBACK(evt_box_event), GINT_TO_POINTER(4567));

  //gtk_container_add(GTK_CONTAINER(evt_box), icon_view_2);
  //gtk_box_pack_start(GTK_BOX(hbox), icon_view, TRUE, TRUE, 0);
  //gtk_box_pack_start(GTK_BOX(hbox), evt_box, TRUE, TRUE, 0);
  //gtk_box_pack_start(GTK_BOX(hbox), icon_view_1, TRUE, TRUE, 0);

  //g_object_unref(icon_view);
  //g_object_unref(icon_view_1);
  //g_object_unref(evt_box);
	//gtk_widget_show(icon_view);
	//gtk_widget_show(evt_box);
	//gtk_widget_show(icon_view_1);
	//gtk_widget_show(icon_view_2);
	//gtk_widget_show(hbox);
	//gtk_widget_show_all(hbox);


	pcd->widget = vbox;
  gtk_box_pack_start(GTK_BOX(vbox), pcd->comment_view, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 10);
	gtk_widget_show_all(vbox);
	//pcd->widget = scrolled;
	g_object_set_data(G_OBJECT(pcd->widget), "pane_data", pcd);
	g_signal_connect(G_OBJECT(pcd->widget), "destroy",
			 G_CALLBACK(bar_pane_comment_destroy), pcd);
	
	//gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled), GTK_SHADOW_IN);
	//gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	//gtk_widget_set_size_request(pcd->widget, -1, height);
	//gtk_widget_show(scrolled);

	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(pcd->comment_view), GTK_WRAP_WORD);
	//gtk_container_add(GTK_CONTAINER(scrolled), pcd->comment_view);

	g_signal_connect(G_OBJECT(pcd->comment_view), "populate-popup",
			 G_CALLBACK(bar_pane_comment_populate_popup), pcd);
	gtk_widget_show(pcd->comment_view);

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pcd->comment_view));
	g_signal_connect(G_OBJECT(buffer), "changed",
			 G_CALLBACK(bar_pane_comment_changed), pcd);


	file_data_register_notify_func(bar_pane_comment_notify_cb, pcd, NOTIFY_PRIORITY_LOW);

  log_printf("DBG PABLO init pcd* 0x%x\n", pcd);

	return pcd->widget;
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
	ret = bar_pane_comment_new(id, title, key, expanded, height);
	g_free(title);
	g_free(key);
	g_free(id);
	return ret;
}

void bar_pane_rating_update_from_config(GtkWidget *pane, const gchar **attribute_names, const gchar **attribute_values)
{
	PaneCommentData *pcd;

	pcd = g_object_get_data(G_OBJECT(pane), "pane_data");
	if (!pcd) return;

	gchar *title = NULL;

	while (*attribute_names)
		{
		const gchar *option = *attribute_names++;
		const gchar *value = *attribute_values++;

		if (READ_CHAR_FULL("title", title)) continue;
		if (READ_CHAR_FULL("key", pcd->key)) continue;
		if (READ_BOOL_FULL("expanded", pcd->pane.expanded)) continue;
		if (READ_INT_FULL("height", pcd->height)) continue;
		if (READ_CHAR_FULL("id", pcd->pane.id)) continue;
		

		log_printf("unknown attribute %s = %s\n", option, value);
		}

  log_printf("DBG PABLO bar_pane_rating_update_from_config, title: %s\n", title);
	if (title)
		{
		bar_pane_translate_title(PANE_RATING, pcd->pane.id, &title);
		gtk_label_set_text(GTK_LABEL(pcd->pane.title), title);
		g_free(title);
		}
	gtk_widget_set_size_request(pcd->widget, -1, pcd->height);
	bar_update_expander(pane);
	bar_pane_comment_update(pcd);
}

/* vim: set shiftwidth=8 softtabstop=0 cindent cinoptions={1s: */
