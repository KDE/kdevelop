program %{APPNAMELC};

{$mode objfpc}

uses
 glib,gdk,gtk;

procedure hello(widget : pGtkWidget ; data: pgpointer ); cdecl;
begin
  writeln('Hello World');
end;

function delete_event (widget : pGtkWidget ; event: pGdkEvent; data: pgpointer ): integer; cdecl;
begin
  writeln('Delete Event Occurred');
  delete_event := ord(true);
end;

procedure destroy(widget : pGtkWidget ; data: pgpointer ); cdecl;
begin
  gtk_main_quit();
end;

var
  window, button :  pGtkWidget;//GtkWidget is the storage type for widgets


begin
  // This is called in all GTK applications. Arguments are parsed
  // from the command line and are returned to the application.
  gtk_init (@argc, @argv);

  // create a new window
  window := gtk_window_new (GTK_WINDOW_TOPLEVEL);

  // When the window is given the "delete_event" signal (this is given
  // by the window manager, usually by the 'close' option, or on the
  // titlebar), we ask it to call the delete_event () function
  // as defined above. The data passed to the callback
  // function is NULL and is ignored in the callback function.
  gtk_signal_connect (pGTKOBJECT (window), 'delete_event',
                      GTK_SIGNAL_FUNC (@delete_event), NIL);


  // Here we connect the "destroy" event to a signal handler.
  // This event occurs when we call gtk_widget_destroy() on the window,
  // or if we return 'FALSE' in the "delete_event" callback.
  gtk_signal_connect (pGTKOBJECT (window), 'destroy',
                    GTK_SIGNAL_FUNC (@destroy), NULL);

  // Sets the border width of the window.
  gtk_container_set_border_width (GTK_CONTAINER (window), 10);

  // Creates a new button with the label "Hello World".
  button := gtk_button_new_with_label ('Hello_World');

  // When the button receives the "clicked" signal, it will call the
  // function hello() passing it NULL as its argument.  The hello()
  // function is defined above. */
  gtk_signal_connect (pGTKOBJECT (button), 'clicked',
                              GTK_SIGNAL_FUNC (@hello), NULL);

  // This will cause the window to be destroyed by calling
  // gtk_widget_destroy(window) when "clicked".  Again, the destroy
  // signal could come from here, or the window manager
  gtk_signal_connect_object (pGTKOBJECT (button), 'clicked',
                      GTK_SIGNAL_FUNC (@gtk_widget_destroy),
                      pGTKOBJECT(window));

  // This packs the button into the window (a gtk container).
  gtk_container_add (GTK_CONTAINER (window), button);

  // The final step is to display this newly created widget.
  gtk_widget_show (button);

  // and the window
  gtk_widget_show (window);

  // All GTK applications must have a gtk_main(). Control ends here
  // and waits for an event to occur (like a key press or
  // mouse event).
  gtk_main ();

end.
