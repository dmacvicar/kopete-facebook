/*
    Kopete Facebook Protocol implementation
    This code is not associated with Facebook in any way.

    Copyright (c) 2009 Duncan Mac-Vicar P. <duncan@kde.org>
    Kopete    (c) 2002-2009 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef FACEBOOKCONTACT_H
#define FACEBOOKCONTACT_H

#include <qmap.h>
//Added by qt3to4:
#include <QList>
#include "kopetecontact.h"
#include "kopetemessage.h"

class KAction;
class KActionCollection;
namespace Kopete { class Account; }
namespace Kopete { class ChatSession; }
namespace Kopete { class MetaContact; }

/**
@author Duncan Mac-Vicar P. <duncan@kde.org>
*/
class FacebookContact : public Kopete::Contact
{
    Q_OBJECT
public:
    /**
     * The range of possible contact types
     */
  enum Type { Null, Echo, Group };
  
  FacebookContact( Kopete::Account* _account, const QString &uniqueName, 
		   const QString &displayName, 
		   Kopete::MetaContact *parent );
  
  ~FacebookContact();
  
  virtual bool isReachable();
  /**
   * Serialize the contact's data into a key-value map
   * suitable for writing to a file
   */
  virtual void serialize(QMap< QString, QString >& serializedData,
			 QMap< QString, QString >& addressBookData);
  /**
   * Return the actions for this contact
   */
  virtual QList<KAction *> *customContextMenuActions(Kopete::ChatSession*);
  /**
   * Returns a Kopete::ChatSession associated with this contact
   */
  virtual Kopete::ChatSession *manager( CanCreateFlags canCreate = CannotCreate );
  
  /**
   * Set the Type of this contact
   */
  void setType( Type type );
public slots:
  /**
   * Transmits an outgoing message to the server 
   * Called when the chat window send button has been pressed
   * (in response to the relevant Kopete::ChatSession signal)
   */
  void sendMessage( Kopete::Message &message );
  /**
   * Called when an incoming message arrived
   * This displays it in the chatwindow
   */
  void receivedMessage( const QString &message );

  void setDisplayPicture( const QImage &image );
  
protected slots:

  /**
   * Show the user info dialog
   */
  void slotUserInfo();

  /**
   * opens the user profile in a browser
   */
  void slotShowProfile();

  /**
   * Notify the contact that its current Kopete::ChatSession was
   * destroyed - probably by the chatwindow being closed
   */
  void slotChatSessionDestroyed();

protected:
  Kopete::ChatSession* m_msgManager;
  KActionCollection* m_actionCollection;

  Type m_type;
  KAction* m_actionPrefs;
  KAction* m_actionShowProfile;
};

#endif
