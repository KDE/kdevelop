/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DOCUMENTATIONPART_H_
#define _DOCUMENTATIONPART_H_

#include <kparts/part.h>
#include <kurl.h>

class KToolBarPopupAction;
class HistoryItem;
class QLabel;
class QPopupMenu;
class KHTMLPart;


class DocumentationPart : public KParts::ReadOnlyPart
{
    Q_OBJECT
    
public:
    DocumentationPart( QWidget *parent=0, const char *name=0 );
    ~DocumentationPart();

    void gotoURL(KURL url);
    KURL browserURL() const;

signals:
    void contextMenu(QPopupMenu *popup, const QString &url, const QString &selection);
    
protected:
    void setManager(KParts::PartManager *manager);

private slots:
    void popupMenu(const QString &url);
    void openURLRequest(const KURL &url);
    void setWindowCaption(const QString &caption);
    void completed();
    void backActivated();
    void forwActivated();
    void backPopupActivated(int id);
    void forwPopupActivated(int id);
    void backShowing();
    void forwShowing();
    void find();
        
private:
    bool openFile();
    void saveState();
    void restoreState();
    void updateHistoryAction();
    
    KHTMLPart *htmlPart;
    QLabel *locationLabel;
    QString currentCaption;
    QList<HistoryItem> history;
    KToolBarPopupAction *backAction;
    KToolBarPopupAction *forwAction;
};

#endif
