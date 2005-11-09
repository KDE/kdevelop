%{HH_TEMPLATE}

// newer (non customized) versions of this file go to main_window.hh_new

// you might replace
//    class foo : public foo_glade { ... };
// by
//    typedef foo_glade foo;
// if you didn't make any modifications to the widget

#ifndef _MAIN_WINDOW_HH
#  include "main_window_glade.hh"
#  define _MAIN_WINDOW_HH
class main_window : public main_window_glade
{

        bool quit(GdkEventAny *ev);
};
#endif
