
#ifndef FACEBOOK_CHATMESSAGE
#define FACEBOOK_CHATMESSAGE

#include <QVariant>
#include <QDateTime>
#include <QDataStream>
#include <QDebug>

namespace Facebook
{

/**
 * Represents information about a message exchanged
 * via the chat service
 */
class ChatMessage
{
public:
    /**
     * creates a emtpy message with a given id
     */
    ChatMessage( const QString &messageId );
    /**
     * creates a new message with a random id.
     */
    ChatMessage();

    ~ChatMessage();

    /**
     * reads a message from a json parsed variant
     * return false if data is not valid
     */
    bool readVariant( const QVariant &variant );

    /**
     * unique id of the message
     */
    QString messageId() const;

    /**
     * text of the message
     */
    QString text() const;
    /**
     * sets the text of the message
     */
    void setText( const QString &text );

    /**
     * timestamp of the message at the service side
     */
    QDateTime time() const;

    /**
     * Sets timestamp of the message at the service side
     */
    void setTime( const QDateTime &time );

    /**
     * timestamp of the message at the client side
     */
    QDateTime clientTime() const;

    /**
     * sets timestamp of the message at the client side
     */
    void setClientTime( const QDateTime &time );

    /**
     * facebook id of the sender of the message
     */
    QString from() const;
    /**
     * sets facebook id of the sender of the message
     */
    void setFrom( const QString &id );
    
    /**
     * name of the sender of the message
     */
    QString fromName() const;

    /**
     * sets name of the sender of the message
     */
    void setFromName( const QString &name );
    
    /**
     * first name of the message sender
     */
    QString fromFirstName() const;

    /**
     * sets the first name of the message sender
     */
    void setFromFirstName( const QString &firstName );

    /**
     * facebook id of the message recipiend
     */
    QString to() const;

    /**
     * sets the facebook id of the message recipient
     */
    void setTo( const QString &id );

    /**
     * name of the message recipient
     */    
    QString toName() const;

    /**
     * sets the name of the message recipient
     */    
    void setToName( const QString &name );
    
    /**
     * first name of the message recipient
     */
    QString toFirstName() const;

    /**
     * sets the first name of the message recipient
     */
    void setToFirstName( const QString &firstName );

private:
    QString _id;
    QString _text;
    QDateTime _time;
    QDateTime _clientTime;
    QString _from;
    QString _to;
    QString _toName;
    QString _toFirstName;
    QString _fromName;
    QString _fromFirstName;

    static unsigned int _idCounter;
};

}

QDataStream & operator<< ( QDataStream & stream, const Facebook::ChatMessage &message );

QDebug operator<<(QDebug dbg, const Facebook::ChatMessage &message);

#endif
