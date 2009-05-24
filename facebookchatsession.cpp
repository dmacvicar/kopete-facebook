
#include <KComponentData>

#include "kopeteprotocol.h"
#include "kopetecontact.h"
#include "kopetechatsessionmanager.h"

#include "facebookchatsession.h"

FacebookChatSession::FacebookChatSession( Kopete::Protocol *protocol, const Kopete::Contact *user, Kopete::ContactPtrList others, Facebook::ChatService *service )
    : Kopete::ChatSession(user, others, protocol)
    , m_service(service)
{

    Kopete::ChatSessionManager::self ()->registerChatSession (this);
    setComponentData (protocol->componentData ());

    QObject::connect (this, SIGNAL(messageSent(Kopete::Message &,
                                                Kopete::ChatSession *)),
                      this, SLOT(slotMessageSent(Kopete::Message &,
                                                  Kopete::ChatSession *)));

    QObject::connect (this, SIGNAL(myselfTyping (bool)),
                      this, SLOT (sendTypingMsg (bool)));

}

FacebookChatSession::~FacebookChatSession()
{
}

void FacebookChatSession::slotSendTyping(bool typ)
{
    // send typing notification here
    Q_UNUSED(typ);   
}

void FacebookChatSession::slotMessageSent(Kopete::Message &message, Kopete::ChatSession *chat)
{
    Q_UNUSED(chat)
    // convert to the what the server wants
    // For this 'protocol', there's nothing to do
    // send it
    Facebook::ChatMessage fbmessage;
    fbmessage.setTo(message.to().first()->contactId());
    fbmessage.setText(message.plainBody());
    
    m_service->sendMessage( fbmessage );

    message.setState( Kopete::Message::StateSending );
    this->appendMessage(message);
    this->messageSucceeded();

    m_messagesSentQueue[fbmessage.messageId()] = message;
    m_messagesTimeoutQueue.append(fbmessage.messageId());
    QTimer::singleShot (60 * 1000, this,
                        SLOT (slotMessageTimeout()));
}

void FacebookChatSession::slotMessageTimeout()
{
        QString messageId = m_messagesTimeoutQueue.takeFirst();
        if(m_messagesSentQueue.contains(messageId))
            this->receivedMessageState(m_messagesSentQueue[messageId].id(), Kopete::Message::StateError );
}

void FacebookChatSession::slotMessageAck( const QString &messageId )
{
    this->receivedMessageState(m_messagesSentQueue[messageId].id(), Kopete::Message::StateSent );

    m_messagesSentQueue.remove(messageId);
    // remove the blinking icon when there are no messages
    // waiting for delivery
    if (m_messagesSentQueue.empty ())
        messageSucceeded ();
}
