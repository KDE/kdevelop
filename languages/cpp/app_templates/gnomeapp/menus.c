
/*** gnomehello-menus */

#include <config.h>
#include "menus.h"
#include "%{APPNAMELC}.h"

static void nothing_cb(GtkWidget* widget, gpointer data);
static void new_app_cb(GtkWidget* widget, gpointer data);
static void close_cb  (GtkWidget* widget, gpointer data);
static void exit_cb   (GtkWidget* widget, gpointer data);
static void about_cb  (GtkWidget* widget, gpointer data);


static GnomeUIInfo file_menu [] = {
  GNOMEUIINFO_MENU_NEW_ITEM(N_("_New %{APPNAME}"),
                            N_("Create a new %{APPNAME}"),
                            new_app_cb, NULL),

  GNOMEUIINFO_MENU_OPEN_ITEM(nothing_cb, NULL),

  GNOMEUIINFO_MENU_SAVE_ITEM(nothing_cb, NULL),

  GNOMEUIINFO_MENU_SAVE_AS_ITEM(nothing_cb, NULL),

  GNOMEUIINFO_SEPARATOR,

  GNOMEUIINFO_MENU_CLOSE_ITEM(close_cb, NULL),

  GNOMEUIINFO_MENU_EXIT_ITEM(exit_cb, NULL),

  GNOMEUIINFO_END
};

static GnomeUIInfo edit_menu [] = {
  GNOMEUIINFO_MENU_CUT_ITEM(nothing_cb, NULL), 
  GNOMEUIINFO_MENU_COPY_ITEM(nothing_cb, NULL),
  GNOMEUIINFO_MENU_PASTE_ITEM(nothing_cb, NULL),
  GNOMEUIINFO_MENU_SELECT_ALL_ITEM(nothing_cb, NULL), 
  GNOMEUIINFO_MENU_CLEAR_ITEM(nothing_cb, NULL),
  GNOMEUIINFO_MENU_UNDO_ITEM(nothing_cb, NULL),
  GNOMEUIINFO_MENU_REDO_ITEM(nothing_cb, NULL), 
  GNOMEUIINFO_MENU_FIND_ITEM(nothing_cb, NULL), 
  GNOMEUIINFO_MENU_FIND_AGAIN_ITEM(nothing_cb, NULL), 
  GNOMEUIINFO_MENU_REPLACE_ITEM(nothing_cb, NULL),
  GNOMEUIINFO_MENU_PROPERTIES_ITEM(nothing_cb, NULL),
  GNOMEUIINFO_END
};

static GnomeUIInfo help_menu [] = {
  GNOMEUIINFO_HELP ("gnome-hello"),
  
  GNOMEUIINFO_MENU_ABOUT_ITEM(about_cb, NULL),
  
  GNOMEUIINFO_END
};

static GnomeUIInfo menu [] = {
  GNOMEUIINFO_MENU_FILE_TREE(file_menu),
  GNOMEUIINFO_MENU_EDIT_TREE(edit_menu),
  GNOMEUIINFO_MENU_HELP_TREE(help_menu),
  GNOMEUIINFO_END
};

static GnomeUIInfo toolbar [] = {
  GNOMEUIINFO_ITEM_STOCK (N_("New"), N_("Create a new %{APPNAME}"), nothing_cb, GNOME_STOCK_PIXMAP_NEW),

  GNOMEUIINFO_SEPARATOR,

  GNOMEUIINFO_ITEM_STOCK (N_("Prev"), N_("Previous hello"), nothing_cb, GNOME_STOCK_PIXMAP_BACK),
  GNOMEUIINFO_ITEM_STOCK (N_("Next"), N_("Next hello"), nothing_cb, GNOME_STOCK_PIXMAP_FORWARD),

  GNOMEUIINFO_END
};


void 
hello_install_menus_and_toolbar(GtkWidget* app)
{
  gnome_app_create_toolbar_with_data(GNOME_APP(app), toolbar, app);
  gnome_app_create_menus_with_data(GNOME_APP(app), menu, app);
  gnome_app_install_menu_hints(GNOME_APP(app), menu);
}

static void 
nothing_cb(GtkWidget* widget, gpointer data)
{
  GtkWidget* dialog;
  GtkWidget* app;
  
  app = (GtkWidget*) data;

  dialog = gnome_ok_dialog_parented(
          _("This does nothing; it is only a demonstration."),
          GTK_WINDOW(app));
}

static void 
new_app_cb(GtkWidget* widget, gpointer data)
{
  GtkWidget* app;

  app = hello_app_new(_("Hello, World!"), NULL, NULL);

  gtk_widget_show_all(app);
}

static void 
close_cb(GtkWidget* widget, gpointer data)
{
  GtkWidget* app;

  app = (GtkWidget*) data;

  hello_app_close(app);
}

static void 
exit_cb(GtkWidget* widget, gpointer data)
{
  gtk_main_quit();
}

static void 
about_cb(GtkWidget* widget, gpointer data)
{
  static GtkWidget* dialog = NULL;
  GtkWidget* app;

  app = (GtkWidget*) data;

  if (dialog != NULL) 
    {
      g_assert(GTK_WIDGET_REALIZED(dialog));
      gdk_window_show(dialog->window);
      gdk_window_raise(dialog->window);
    }
  else
    {        
      const gchar *authors[] = {
        "%{AUTHOR} <%{EMAIL}>",
        NULL
      };

      gchar* logo = gnome_pixmap_file("%{APPNAMELC}.png");

      dialog = gnome_about_new (_("%{APPNAME}"), VERSION,
                                "(C) %{AUTHOR}",
                                authors,
                                _("A sample GNOME application."),
                                logo);

      g_free(logo);

      gtk_signal_connect(GTK_OBJECT(dialog),
                         "destroy",
                         GTK_SIGNAL_FUNC(gtk_widget_destroyed),
                         &dialog);

      gnome_dialog_set_parent(GNOME_DIALOG(dialog), GTK_WINDOW(app));

      gtk_widget_show(dialog);
    }
}


/* gnomehello-menus ***/
