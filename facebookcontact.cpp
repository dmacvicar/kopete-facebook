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

#include <KToolInvocation>
#include <KDialog>
#include <KAction>
#include <KDebug>
#include <KLocale>

#include "kopeteavatarmanager.h"
#include "kopeteaccount.h"
#include "kopetechatsessionmanager.h"
#include "kopetechatsession.h"
#include "kopetemetacontact.h"

#include "facebookaccount.h"
#include "facebookprotocol.h"
#include "facebookchatsession.h"

#include "ui_facebookinfo.h"

FacebookContact::FacebookContact( Kopete::Account* _account, const QString &uniqueName,
				  const QString &displayName, Kopete::MetaContact *parent )
    : Kopete::Contact( _account, uniqueName, parent )
    , m_actionShowProfile(0L)
{
    kDebug( FBDBG ) << " uniqueName: " << uniqueName << ", displayName: " << displayName;
    m_type = FacebookContact::Null;
    // FIXME: ? setDisplayName( displayName );
    m_msgManager = 0L;
    
    setOnlineStatus( FacebookProtocol::protocol()->facebookOffline );

    m_actionShowProfile = new KAction(i18n("Show Profile"), this);
    QObject::connect(m_actionShowProfile, SIGNAL(triggered(bool)), this, SLOT(slotShowProfile()));

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

QList<KAction *> *FacebookContact::customContextMenuActions(Kopete::ChatSession*)
{
    //FIXME!!!  this function is obsolete, we should use XMLGUI instead
    QList<KAction*> *actions = new QList<KAction*>();
    actions->append(m_actionShowProfile);
    return actions;
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

void
FacebookContact::slotShowProfile()
{
    KToolInvocation::invokeBrowser(QString::fromLatin1("http://www.facebook.com/profile.php?id=") + contactId()) ;
}

void FacebookContact::slotUserInfo()
{
    KDialog infoDialog;
    infoDialog.setButtons( KDialog::Close);
    infoDialog.setDefaultButton(KDialog::Close);
    Ui::FacebookInfo info;
    info.setupUi(infoDialog.mainWidget());
    info.m_displayName->setText(nickName());
    info.m_personalMessage->setPlainText(statusMessage().message());
    QVariant picture(property(Kopete::Global::Properties::self()->photo()).value());    
    info.m_photo->setPixmap(picture.value<QPixmap>());    

    infoDialog.setCaption(nickName());
    infoDialog.exec();
}

#include "facebookcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

