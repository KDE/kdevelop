/***************************************************************************
                          defines_item.h  -  description
                             -------------------                                         
    begin                : Thu Mar 18 1999                                           
    copyright            : (C) 1999 by Pascal Krahmer
    email                : pascal@beast.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifdef WIDGETTYPE
#ifdef CLASSNAME
#ifdef CLASSDESC

#ifdef ISCONTAINER
protected:
    class MyWidget : public QFrame
    {
      public:
//        MyWidget(KDlgItem_QWidget* wid, QWidget* parent = 0, const char* name = 0);
        MyWidget(KDlgItem_QWidget *wid, bool ismain, QWidget *parent=0, const char *name = 0); 
      protected:
        KDlgItem_QWidget *parentObject;
        virtual void paintEvent ( QPaintEvent *e );
    };
#endif

public:
    CLASSNAME( KDlgEditWidget* editwid = 0, QWidget *parent = 0, const char* name = 0 );
    virtual void recreateItem();
    virtual ~CLASSNAME() {}
#ifdef ISCONTAINER
    virtual QString itemClass() { return QString("QWidget"); }
    virtual MyWidget* getItem() { return (MyWidget*)item; }
#else
    virtual QString itemClass() { return item->className(); }
    virtual WIDGETTYPE* getItem() { return (WIDGETTYPE*)item; }
#endif
    virtual void repaintItem(WIDGETTYPE *it = 0);
    virtual void addMyPropEntrys();

#undef CLASSDESC
#endif
#undef CLASSNAME
#endif
#undef WIDGETTYPE
#endif
