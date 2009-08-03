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

#include "facebookaccount.h"

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kactionmenu.h>
#include <kmenu.h>
#include <kicon.h>

#include <kopetechatsession.h>
#include <kopetemessage.h>

#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetepassword.h"
#include "kopeteutils.h"

#include "facebookcontact.h"
#include "facebookprotocol.h"
#include "facebookchatsession.h"

using Facebook::ChatService;
using Facebook::ChatMessage;
using Facebook::BuddyInfo;

FacebookAccount::FacebookAccount( FacebookProtocol *parent, const QString& accountID )
  : Kopete::PasswordedAccount ( parent, accountID )
  , m_service(0L)
{
    setMyself( new FacebookContact( this, accountId(), accountId(), Kopete::ContactList::self()->myself() ) );
    myself()->setOnlineStatus( FacebookProtocol::protocol()->facebookOffline );

    m_service = new Facebook::ChatService(this);
}

FacebookAccount::~FacebookAccount()
{
        qDebug() << "destructing FacebookAccount";
}

FacebookContact * FacebookAccount::contact( const QString &id )
{
    return static_cast<FacebookContact *>(contacts().value(id));
}


void FacebookAccount::fillActionMenu( KActionMenu *actionMenu )
{
	Kopete::Account::fillActionMenu( actionMenu );

	//actionMenu->addSeparator();
	//KAction *action;
	//action = new KAction (KIcon("facebook_showvideo"), i18n ("Show my own video..."), actionMenu );
        //, "actionShowVideo");
	//QObject::connect( action, SIGNAL(triggered(bool)), this, SLOT(slotShowVideo()) );
	//actionMenu->addAction(action);
	//action->setEnabled( isConnected() );
}

bool FacebookAccount::createContact(const QString& contactId, Kopete::MetaContact* parentContact)
{
    if ( ! contact(contactId) )
    {
	FacebookContact* newContact = new FacebookContact( this, contactId, parentContact->displayName(), parentContact );
	return newContact != 0L;
    }
    return false;
}

void FacebookAccount::setAway( bool away, const QString & /* reason */ )
{
    if ( away )
        slotGoAway();
    else
        slotGoOnline();
}

void FacebookAccount::setOnlineStatus(const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason, const OnlineStatusOptions& options)
{
    Q_UNUSED(options);

    if ( status.status() == Kopete::OnlineStatus::Online &&
         myself()->onlineStatus().status() == Kopete::OnlineStatus::Offline )
        slotGoOnline();
    else if (status.status() == Kopete::OnlineStatus::Online &&
             myself()->onlineStatus().status() == Kopete::OnlineStatus::Away )
        setAway( false, reason.message() );
    else if ( status.status() == Kopete::OnlineStatus::Offline )
        slotGoOffline();
    else if ( status.status() == Kopete::OnlineStatus::Away )
        slotGoAway( /* reason */ );    
}

void FacebookAccount::setStatusMessage(const Kopete::StatusMessage& statusMessage)
{
        if ( ! statusMessage.isEmpty() )
            m_service->setStatusMessage(statusMessage.title());    
}

void FacebookAccount::connectWithPassword (const QString & pass)
{
    if (myself ()->onlineStatus () != FacebookProtocol::protocol ()->facebookOffline)
        return;

    if (pass.isEmpty ())
    {
        password ().setWrong (true);
        password ().setWrong (false);
        return;
    }

    password ().setWrong (false);

    QString login = accountId();
    QString pass1 = pass;

    m_service->setLoginInformation(login, pass1);

    myself()->setOnlineStatus( FacebookProtocol::protocol()->facebookConnecting );
    
    m_service->loginToService();

    QObject::connect(m_service, SIGNAL(loginToServiceFinished()), this, SLOT(slotLoginToServiceFinished()));
    QObject::connect(m_service, SIGNAL(loginToServiceError()), this, SLOT(slotLoginToServiceError()));

}

void FacebookAccount::slotLoginToServiceFinished()
{
    myself()->setOnlineStatus( FacebookProtocol::protocol()->facebookOnline );
    m_service->setVisibility(true);

    // connect changed status and buddy signals
    QObject::connect(m_service, SIGNAL(buddyAvailable(const BuddyInfo &, bool)), this, SLOT(slotBuddyAvailable(const BuddyInfo &, bool)));
    QObject::connect(m_service, SIGNAL(buddyNotAvailable(const BuddyInfo &)), this, SLOT(slotBuddyNotAvailable(const BuddyInfo &)));
    QObject::connect(m_service, SIGNAL(buddyInformation(const BuddyInfo &)), this, SLOT(slotBuddyInformation(const BuddyInfo &)));
    QObject::connect(m_service, SIGNAL(messageAvailable(const ChatMessage &)), this, SLOT(slotMessageAvailable(const ChatMessage &)));
    QObject::connect(m_service, SIGNAL(messageSendFinished(const ChatMessage &)), this, SLOT(slotMessageAckAvailable(const ChatMessage &)));
    QObject::connect(m_service, SIGNAL(messageSendError(const ChatMessage &)), this, SLOT(slotMessageErrorAvailable(const ChatMessage &)));
    QObject::connect(m_service, SIGNAL(buddyThumbAvailable( const QString &, const QImage & )), this, SLOT(slotBuddyThumbAvailable( const QString &, const QImage & )));
    QObject::connect(m_service, SIGNAL(typingEventAvailable(const QString &, const QString &)), this, SLOT(slotTypingEventAvailable(const QString &, const QString &)));
    QObject::connect(m_service, SIGNAL(error( int, const QString &)), this, SLOT(slotError(int, const QString &)));
}
    
void FacebookAccount::slotLoginToServiceError()
{
    kDebug (FBDBG) << k_funcinfo;
    myself ()->setOnlineStatus (FacebookProtocol::protocol ()->facebookOffline);
    Kopete::Utils::notifyCannotConnect(this);
}

void  FacebookAccount::slotLogoutFromServiceFinished()
{
    myself()->setOnlineStatus( FacebookProtocol::protocol()->facebookOffline );
    QObject::disconnect ( m_service, 0, 0, 0 );
}

void  FacebookAccount::slotLogoutFromServiceError()
{

}

void FacebookAccount::disconnect()
{
    kDebug ( FBDBG ) ;
    if ( m_service->isLoggedIn() )
    {                
        m_service->setVisibility(false);
        m_service->logoutFromService();
    }            
}

Facebook::ChatService * FacebookAccount::service()
{
    return m_service;
}

void FacebookAccount::slotGoOnline ()
{
	kDebug ( FBDBG ) ;

	if (!isConnected ())
		connect ();
}

void FacebookAccount::slotGoAway ()
{
	kDebug ( FBDBG ) ;

	if (!isConnected ())
		connect();

        m_service->setVisibility(true);
	myself()->setOnlineStatus( FacebookProtocol::protocol()->facebookAway );
}


void FacebookAccount::slotGoOffline ()
{
	kDebug ( FBDBG ) ;

	if (isConnected ())
            disconnect ();
}

void FacebookAccount::slotMessageAckAvailable( const ChatMessage &message )
{
    FacebookChatSession *mm = static_cast<FacebookChatSession *>(contact(message.to())->manager(Kopete::Contact::CanCreate));
    mm->slotMessageAck(message.messageId());                
}

void FacebookAccount::slotMessageErrorAvailable( const ChatMessage &message )
{
    FacebookChatSession *mm = static_cast<FacebookChatSession *>(contact(message.to())->manager(Kopete::Contact::CanCreate));
    mm->slotMessageError(message.messageId());                
}

void  FacebookAccount::slotMessageAvailable( const Facebook::ChatMessage &message )
{
    QFont msgFont;
    QDateTime msgDT;
    Kopete::ContactPtrList justMe;

    // outgoing or incoming
    if ( message.from() == m_service->userId() )
    {
        kDebug(FBDBG) << "got own sent message back (ack)";        
        // outgoing, we get our own messages back
        // we should use this for confirmation or something
        // like that
    }
    else if ( message.to() == m_service->userId() )
    {
        // incoming
        if( !contact( message.from() ) )
        {
            // this would be rare... receiving a message from unknown buddy
            kDebug(FBDBG) << "Adding contact " << message.from();
            addContact( message.from(), message.from(),  0L, Kopete::Account::Temporary );
        }
        if (message.time().toTime_t() == 0)
            msgDT = QDateTime( QDate::currentDate(), QTime::currentTime(), Qt::LocalTime );
        else
            msgDT = message.time();

        Kopete::ChatSession *mm = contact(message.from())->manager(Kopete::Contact::CanCreate);

        // Tell the message manager that the buddy is done typing
        mm->receivedTypingMsg(contact(message.from()), false);

        justMe.append(myself());

        Kopete::Message kmsg(contact(message.from()), justMe);
        kmsg.setTimestamp( msgDT );
        kmsg.setPlainBody( message.text() );
        kmsg.setDirection( Kopete::Message::Inbound );

        mm->appendMessage(kmsg);
    }
}

void  FacebookAccount::slotBuddyAvailable( const Facebook::BuddyInfo &buddy, bool idle )
{
    // server -> local
    if ( !contact( buddy.buddyId() ) )
    {
	kDebug(FBDBG) << "Contact " << buddy.buddyId() << " is not in the contact list. Ugh!";
        return;
    }
    
    if ( idle ) {
      qDebug() << "Contacting " << buddy.buddyId();
      contact( buddy.buddyId() )->setOnlineStatus( idle ? FacebookProtocol::protocol()->facebookAway : FacebookProtocol::protocol()->facebookOnline );
    }
}

void  FacebookAccount::slotBuddyNotAvailable( const Facebook::BuddyInfo &buddy )
{
    // server -> local
    if ( !contact( buddy.buddyId() ) )
    {
	kDebug(FBDBG) << "Contact " << buddy.buddyId() << " is not in the contact list. Ugh!";
        return;
    }
    contact( buddy.buddyId() )->setOnlineStatus( FacebookProtocol::protocol()->facebookOffline );
}

void  FacebookAccount::slotBuddyInformation( const Facebook::BuddyInfo &buddy )
{
    // server -> local
    if ( !contact( buddy.buddyId() ) )
    {
	kDebug(FBDBG) << "Contact " << buddy.buddyId() << " is not in the contact list. Adding...";
	Kopete::Group *g = Kopete::ContactList::self()->findGroup("Facebook");
	addContact(buddy.buddyId(), buddy.name().isEmpty() ? buddy.buddyId() : buddy.name() , g, Kopete::Account::ChangeKABC);
        contact( buddy.buddyId() )->setOnlineStatus( FacebookProtocol::protocol()->facebookOffline );
    }

    // set status message
    if ( ! buddy.status().isEmpty() )
        contact( buddy.buddyId() )->setStatusMessage(buddy.status());
    // set name
    if ( ! buddy.name().isEmpty() )
        contact( buddy.buddyId() )->setNickName(buddy.name());

    m_service->requestPicture(buddy.buddyId());
}

void FacebookAccount::slotBuddyThumbAvailable( const QString &buddyid, const QImage &image )
{
    if ( ! contact(buddyid) )
        return;
    
    contact(buddyid)->setDisplayPicture(image);
}

void FacebookAccount::slotTypingEventAvailable( const QString &from, const QString &to )
{
    Q_UNUSED(to);
    if ( ! contact(from) )
        return;
    
    Kopete::ChatSession *mm = contact(from)->manager(Kopete::Contact::CanCreate);
    // Tell the message manager that the buddy is typing
    mm->receivedTypingMsg(contact(from), true);
}

void FacebookAccount::slotError( int error, const QString& )
{
    switch ( error )
    {
    case ChatService::ErrorDisconnected:
        Kopete::Utils::notifyConnectionLost(this);
        break;
    default:
        break;        
    }    
}



#include "facebookaccount.moc"
