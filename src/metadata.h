/*
 * Geeqie
 * (C) 2004 John Ellis
 * Copyright (C) 2008 - 2012 The Geeqie Team
 *
 * Author: John Ellis, Laurent Monin
 *
 * This software is released under the GNU General Public License (GNU GPL).
 * Please read the included file COPYING for more information.
 * This software comes with no warranty of any kind, use at your own risk!
 */


#ifndef METADATA_H
#define METADATA_H

#define COMMENT_KEY "Xmp.dc.description"
#define KEYWORD_KEY "Xmp.dc.subject"
#define KEYWORD_RATING "Xmp.xmp.Rating"
#define ORIENTATION_KEY "Xmp.tiff.Orientation"

void metadata_cache_free(FileData *fd);

gboolean metadata_write_queue_remove(FileData *fd);
gboolean metadata_write_queue_remove_list(GList *list);
gboolean metadata_write_perform(FileData *fd);
gboolean metadata_write_queue_confirm(gboolean force_dialog, FileUtilDoneFunc done_func, gpointer done_data);
void metadata_notify_cb(FileData *fd, NotifyType type, gpointer data);

gint metadata_queue_length(void);

gboolean metadata_write_revert(FileData *fd, const gchar *key);
gboolean metadata_write_list(FileData *fd, const gchar *key, const GList *values);
gboolean metadata_write_string(FileData *fd, const gchar *key, const char *value);
gboolean metadata_write_int(FileData *fd, const gchar *key, guint64 value);
gboolean metadata_write_rating(FileData *fd, gint value);

GList *metadata_read_list(FileData *fd, const gchar *key, MetadataFormat format);
gchar *metadata_read_string(FileData *fd, const gchar *key, MetadataFormat format);
guint64 metadata_read_int(FileData *fd, const gchar *key, guint64 fallback);
gdouble metadata_read_GPS_coord(FileData *fd, const gchar *key, gdouble fallback);
gint metadata_read_rating(FileData *fd);

gboolean metadata_append_string(FileData *fd, const gchar *key, const char *value);
gboolean metadata_append_list(FileData *fd, const gchar *key, const GList *values);

GList *string_to_keywords_list(const gchar *text);

gboolean meta_data_get_keyword_mark(FileData *fd, gint n, gpointer data);
gboolean meta_data_set_keyword_mark(FileData *fd, gint n, gboolean value, gpointer data);


enum {
	KEYWORD_COLUMN_MARK,
	KEYWORD_COLUMN_NAME,
	KEYWORD_COLUMN_CASEFOLD,
	KEYWORD_COLUMN_IS_KEYWORD,
	KEYWORD_COLUMN_HIDE_IN,
	KEYWORD_COLUMN_COUNT
};

extern GtkTreeStore *keyword_tree;

void meta_data_connect_mark_with_keyword(GtkTreeModel *keyword_tree, GtkTreeIter *kw_iter, gint mark);
void meta_data_connect_mark_with_rating(gint rating, gint mark);


gchar *keyword_get_name(GtkTreeModel *keyword_tree, GtkTreeIter *iter);
gchar *keyword_get_casefold(GtkTreeModel *keyword_tree, GtkTreeIter *iter);
gboolean keyword_get_is_keyword(GtkTreeModel *keyword_tree, GtkTreeIter *iter);

gboolean keyword_compare(GtkTreeModel *keyword_tree, GtkTreeIter *a, GtkTreeIter *b);
gboolean keyword_same_parent(GtkTreeModel *keyword_tree, GtkTreeIter *a, GtkTreeIter *b);
gboolean keyword_exists(GtkTreeModel *keyword_tree, GtkTreeIter *parent_ptr, GtkTreeIter *sibling, const gchar *name, gboolean exclude_sibling, GtkTreeIter *result);

void keyword_copy(GtkTreeStore *keyword_tree, GtkTreeIter *to, GtkTreeIter *from);
void keyword_copy_recursive(GtkTreeStore *keyword_tree, GtkTreeIter *to, GtkTreeIter *from);
void keyword_move_recursive(GtkTreeStore *keyword_tree, GtkTreeIter *to, GtkTreeIter *from);

GList *keyword_tree_get_path(GtkTreeModel *keyword_tree, GtkTreeIter *iter_ptr);
gboolean keyword_tree_get_iter(GtkTreeModel *keyword_tree, GtkTreeIter *iter_ptr, GList *path);

void keyword_set(GtkTreeStore *keyword_tree, GtkTreeIter *iter, const gchar *name, gboolean is_keyword);
gboolean keyword_tree_is_set(GtkTreeModel *keyword_tree, GtkTreeIter *iter, GList *kw_list);
void keyword_tree_set(GtkTreeModel *keyword_tree, GtkTreeIter *iter_ptr, GList **kw_list);
GList *keyword_tree_get(GtkTreeModel *keyword_tree, GtkTreeIter *iter_ptr);
void keyword_tree_reset(GtkTreeModel *keyword_tree, GtkTreeIter *iter_ptr, GList **kw_list);

void keyword_delete(GtkTreeStore *keyword_tree, GtkTreeIter *iter_ptr);


void keyword_hide_in(GtkTreeStore *keyword_tree, GtkTreeIter *iter, gpointer id);
void keyword_show_in(GtkTreeStore *keyword_tree, GtkTreeIter *iter, gpointer id);
gboolean keyword_is_hidden_in(GtkTreeModel *keyword_tree, GtkTreeIter *iter, gpointer id);
void keyword_show_all_in(GtkTreeStore *keyword_tree, gpointer id);
void keyword_hide_unset_in(GtkTreeStore *keyword_tree, gpointer id, GList *keywords);
void keyword_show_set_in(GtkTreeStore *keyword_tree, gpointer id, GList *keywords);

void keyword_tree_new_default(void);
void keyword_tree_new(void);

void keyword_tree_write_config(GString *outstr, gint indent);
GtkTreeIter *keyword_add_from_config(GtkTreeStore *keyword_tree, GtkTreeIter *parent, const gchar **attribute_names, const gchar **attribute_values);

#endif
/* vim: set shiftwidth=8 softtabstop=0 cindent cinoptions={1s: */
