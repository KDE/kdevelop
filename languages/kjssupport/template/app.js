#!/usr/bin/env kjscmd

// Create main view
var mw = new KMainWindow();
var lv = new KListView( mw );
mw.setCentralWidget(lv);

lv.addColumn('One');
lv.addColumn('Two');
lv.addColumn('Three');

lv.insertItem( 'Something', "Nothing", "Thing" );
lv.insertItem( 'Something', "Nothing", "Thing" );
lv.insertItem( 'Something', "Nothing", "Thing" );
lv.insertItem( 'Something', "Nothing", "Thing" );

mw.show();

