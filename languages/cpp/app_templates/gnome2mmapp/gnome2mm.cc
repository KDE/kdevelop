%{CC_TEMPLATE}

#include <config.h>
#include <gnome--/main.h>
#include <libgnome/gnome-i18n.h>

#include "main_window.hh"

int main(int argc, char **argv)
{  
#if defined(ENABLE_NLS)
   bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
   textdomain (GETTEXT_PACKAGE);
#endif //ENABLE_NLS
   
   Gnome::Main m(PACKAGE, VERSION, argc, argv);

   main_window *main_window = new class main_window();
   m.run();
   delete main_window;
   return 0;
}
