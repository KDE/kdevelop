#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qtopia/qpeapplication.h>
#include <stdio.h>
#include <stdlib.h>

int main( int argc, char **argv )
{
	QPEApplication a( argc, argv );

	Main m;
	a.showMainWidget(&m);

	return a.exec();
}

