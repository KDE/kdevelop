
#include <gtk--/main.h>

#include "main_window.hh"

int main(int argc, char **argv)
{  
   
   Gtk::Main m(&argc, &argv);

   main_window *main_window = new class main_window();
   m.run();
   delete main_window;
   return 0;
}
