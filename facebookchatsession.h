
#ifndef FACEBOOKCHATSESSION_H
#define FACEBOOKCHATSESSION_H

#include <QLinkedList>
#include <QMap>
#include <QTimer>

#include <KAction>
#include <KActionMenu>
#include <KMenu>

#include "kopetechatsession.h"
#include "facebook/chatservice.h"

class FacebookContact;

class FacebookChatSession : public Kopete::ChatSession
{
    Q_OBJECT

public:
    FacebookChatSession( Kopete::Protocol *protocol, const Kopete::Contact *user, Kopete::ContactPtrList others, Facebook::ChatService *service );
    virtual ~FacebookChatSession();
public slots:
    void slotMessageAck( const QString &messageId );
protected slots:
    void slotSendTyping(bool typ);
    void slotMessageSent(Kopete::Message &, Kopete::ChatSession *);
    void slotMessageTimeout();
private:
    QLinkedList<Kopete::Message> m_messagesQueue;
    QMap<QString, Kopete::Message> m_messagesSentQueue;
    QLinkedList<QString> m_messagesTimeoutQueue;
    Facebook::ChatService *m_service;
};

#endif
