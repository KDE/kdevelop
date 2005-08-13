%{H_TEMPLATE}

/*** gnomehello-apph */

#ifndef GNOMEHELLO_APP_H
#define GNOMEHELLO_APP_H

#include <gnome.h>

GtkWidget* hello_app_new(const gchar* message, 
                         const gchar* geometry,
                         GSList* greet);

void       hello_app_close(GtkWidget* app);
                         
#endif

/* gnomehello-apph ***/
