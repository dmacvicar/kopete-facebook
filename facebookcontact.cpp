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

#include "facebookcontact.h"

#include <QList>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include "kopeteavatarmanager.h"
#include "kopeteaccount.h"
#include "kopetechatsessionmanager.h"
#include "kopetechatsession.h"
#include "kopetemetacontact.h"

#include "facebookaccount.h"
#include "facebookprotocol.h"
#include "facebookchatsession.h"

FacebookContact::FacebookContact( Kopete::Account* _account, const QString &uniqueName,
				  const QString &displayName, Kopete::MetaContact *parent )
    : Kopete::Contact( _account, uniqueName, parent )
{
    kDebug( FBDBG ) << " uniqueName: " << uniqueName << ", displayName: " << displayName;
    m_type = FacebookContact::Null;
    // FIXME: ? setDisplayName( displayName );
    m_msgManager = 0L;
    
    setOnlineStatus( FacebookProtocol::protocol()->facebookOffline );

}

FacebookContact::~FacebookContact()
{
}

void FacebookContact::setType( FacebookContact::Type type )
{
    m_type = type;
}

bool FacebookContact::isReachable()
{
    return true;
}

void FacebookContact::serialize( QMap< QString, QString > &serializedData, QMap< QString, QString > & /* addressBookData */ )
{
    QString value;
    switch ( m_type )
    {
    case Null:
	value = QLatin1String("null");
    case Echo:
	value = QLatin1String("echo");
    case Group:
	value = QLatin1String("group");
    }
    serializedData[ "contactType" ] = value;
}

Kopete::ChatSession* FacebookContact::manager( CanCreateFlags canCreateFlags )
{
    kDebug( FBDBG ) ;

    Kopete::ContactPtrList chatmembers;
    chatmembers.append (this);

    Kopete::ChatSession * m_msgManager = Kopete::ChatSessionManager::self()->findChatSession (account()->myself(), chatmembers, protocol());

    FacebookChatSession *manager = qobject_cast <FacebookChatSession *>(m_msgManager);
    if (!manager && canCreateFlags == Kopete::Contact::CanCreate)
    {        
        //Kopete::ChatSession::Form form = ( m_type == Group ?
	//				   Kopete::ChatSession::Chatroom : Kopete::ChatSession::Small );
        //m_msgManager = Kopete::ChatSessionManager::self()->create(account()->myself(), contacts, protocol(), form );
        m_msgManager = new FacebookChatSession(protocol(), account()->myself(),
                                                  chatmembers, static_cast<FacebookAccount *>( account() )->service() );
        connect(m_msgManager, SIGNAL(destroyed()), this, SLOT(slotChatSessionDestroyed()));
    }
    return m_msgManager;
}

QList<KAction *> *FacebookContact::customContextMenuActions() //OBSOLETE
{
    //FIXME!!!  this function is obsolete, we should use XMLGUI instead
    /*m_actionCollection = new KActionCollection( this, "userColl" );
      m_actionPrefs = new KAction(i18n( "&Contact Settings" ), 0, this,
      SLOT( showContactSettings( )), m_actionCollection, "contactSettings" );
      
      return m_actionCollection;*/
    return 0L;
}

void FacebookContact::showContactSettings()
{
    //FacebookContactSettings* p = new FacebookContactSettings( this );
    //p->show();
}

void FacebookContact::sendMessage( Kopete::Message &message )
{
    kDebug( FBDBG ) ;    
    // give it back to the manager to display
    manager()->appendMessage( message );
    // tell the manager it was sent successfully
    manager()->messageSucceeded();
}

void FacebookContact::receivedMessage( const QString &message )
{
    Kopete::ContactPtrList contactList;
    contactList.append( account()->myself() );
    // Create a Kopete::Message
    Kopete::Message newMessage( this, contactList );
    newMessage.setPlainBody( message );
    newMessage.setDirection( Kopete::Message::Inbound );
   
    // Add it to the manager
    manager(CanCreate)->appendMessage(newMessage);
}

void FacebookContact::slotChatSessionDestroyed()
{
    //FIXME: the chat window was closed?  Take appropriate steps.
    m_msgManager = 0L;
}

void FacebookContact::setDisplayPicture( const QImage &image )
{
    Kopete::AvatarManager::AvatarEntry entry;
    entry.name = contactId();
    entry.category = Kopete::AvatarManager::Contact;
    entry.contact = this;
    entry.image = image;
    entry = Kopete::AvatarManager::self()->add(entry);
    removeProperty (Kopete::Global::Properties::self()->photo ());
    setProperty (Kopete::Global::Properties::self()->photo (), image);
    //emit displayPictureChanged();
}

#include "facebookcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

