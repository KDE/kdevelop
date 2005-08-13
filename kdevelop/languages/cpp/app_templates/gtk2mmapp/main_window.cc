%{CC_TEMPLATE}

// newer (non customized) versions of this file go to main_window.cc_new

// This file is for your program, I won't touch it again!

#include "config.h"
#include "main_window.hh"

#include <gtk--/main.h>

gint main_window::quit(GdkEventAny *ev)
{
    Gtk::Main::quit();
}
