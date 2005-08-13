/*
 * Copyright (C) %{YEAR} %{AUTHOR} <%{EMAIL}>
 */

import java.util.*;
import org.kde.qt.*;
import org.kde.koala.*;

/**
 * This is the main view class for %{APPNAME}.  Most of the non-menu,
 * non-toolbar, and non-statusbar (e.g., non frame) GUI code should go
 * here.
 *
 * This %{APPNAMELC} uses an HTML component as an example.
 *
 * @short Main view
 * @author %{AUTHOR} <%{EMAIL}>
 * @version %{VERSION}
 */
public class %{APPNAME}View extends QWidget
{

    QHBoxLayout top_layout;

public %{APPNAME}View(QWidget parent)
{
    super(parent, null);
    // setup our layout manager to automatically add our widgets
    top_layout = new QHBoxLayout(this);
    top_layout.setAutoAdd(true);

    // we want to look for all components that satisfy our needs.  the
    // trader will actually search through *all* registered KDE
    // applications and components -- not just KParts.  So we have to
    // specify two things: a service type and a constraint
    //
    // the service type is like a mime type.  we say that we want all
    // applications and components that can handle HTML -- 'text/html'
    // 
    // however, by itself, this will return such things as Netscape..
    // not what we wanted.  so we constrain it by saying that the
    // string 'KParts/ReadOnlyPart' must be found in the ServiceTypes
    // field.  with this, only components of the type we want will be
    // returned.
    ArrayList offers = KTrader.self().query("text/html", "'KParts/ReadOnlyPart' in ServiceTypes");

    KLibFactory factory = null;
    // in theory, we only care about the first one.. but let's try all
    // offers just in case the first can't be loaded for some reason
    Iterator it = offers.iterator();
    while(it.hasNext())
    {
        KService ptr = (KService) it.next();

        // we now know that our offer can handle HTML and is a part.
        // since it is a part, it must also have a library... let's try to
        // load that now
        factory = KLibLoader.self().factory( ptr.library() );
        if (factory != null)
        {
            m_html = (ReadOnlyPart) factory.create(this, ptr.name(), "KParts::ReadOnlyPart");
            break;
        }
    }

    // if our factory is invalid, then we never found our component
    // and we might as well just exit now
    if (factory == null)
    {
        KMessageBox.error(this, "Could not find a suitable HTML component");
        return;
    }

    connect(m_html, SIGNAL("setWindowCaption(String)"),
            this,   SLOT("slotSetTitle(String)"));
    connect(m_html, SIGNAL("setStatusBarText(String)"),
            this,   SLOT("slotOnURL(String)"));

}


public void print(QPainter p, int height, int width)
{
    // do the actual printing, here
    // p.drawText(etc..)
}

public String currentURL()
{
    return m_html.url().url();
}

public void openURL(String url)
{
    openURL(new KURL(url));
}

public void openURL(KURL url)
{
    m_html.openURL(url);
}

private void slotOnURL(String url)
{
    emit("signalChangeStatusbar", url);
}

private void slotSetTitle(String title)
{
    emit("signalChangeCaption", title);
}

private ReadOnlyPart m_html;

}
