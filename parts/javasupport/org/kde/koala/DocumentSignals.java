package org.kde.koala;
import org.kde.qt.*;


/** {@link Document} emits these signals */
public interface DocumentSignals {
	void destroyed( QObject obj );

/** 
 Same as the above signal except besides indicating that the data has
 been completely loaded it also informs the host, by setting the flag,
 that a pending action has been generated as a result of completing the
 requested task (loading the data).  An example of this is meta-refresh
 tags on HTML pages which result in the page either being refreshed or
 the viewer being redirected to another page.  By emitting this signal
 after appropriately setting the flag, the part can tell the host of the
 pending scheduled action inorder to give it a chance to accept or cancel
 that action.

 <code>pendingAction</code>  if true, a pending action exists (ex: a scheduled refresh)
   
*/
	void completed( boolean pendingAction );

/** 
 Emitted by the part, to set the caption of the window(s)
 hosting this part
     
*/
	void setWindowCaption( String caption );
	void saved( Document doc );

/** 
 Emit this if loading is canceled by the user or by an error.

*/
	void canceled( String errMsg );

/** 
 The part emits this when starting data.
 If using a KIO.Job, it sets the job in the signal, so that
 progress information can be shown. Otherwise, job is 0.

*/
	void started( Job arg1 );
	void loaded( Document doc );

/** 
 Emited by the part, to set a text in the statusbar of the window(s)
 hosting this part
     
*/
	void setStatusBarText( String text );
}

