
#ifndef FACEBOOK_BUDDYINFO
#define FACEBOOK_BUDDYINFO

#include <QDateTime>
#include <QVariant>
#include <QDebug>

namespace Facebook
{

/**
 * Represents information about a Facebook contact
 */
class BuddyInfo
{
public:
    BuddyInfo();
    ~BuddyInfo();
    
    /**
     * read data from a json parsed variant
     */
    bool readVariant( const QVariant &variant );

    QString buddyId() const;
    void setBuddyId( const QString &id );

    bool enableVC() const;
    void setEnableVC( bool enabled );

    QString firstName() const;
    void setFirstName( const QString &firstName );
    
    QString name() const;
    void setName( const QString &name );
    
    /**
     * text status of a contact
     * @note nothing to do with availability status
     */
    QString status() const;
    void setStatus( const QString &status );
    
    /**
     * When status was set
     */
    QDateTime statusTime() const;
    void setStatusTime( const QDateTime &statustime );

    /**
     * When status was set, in english form
     * ie: 3 days ago
     */
    QString statusTimeRel() const;
    void setStatusTimeRel( const QString &statusTime );

    /**
     * picture thumnail url
     */
    QString thumbSrc() const;
    void setThumbSrc( const QString &thumbSrc );

private:
    QString _buddyId;
    bool _enableVC;
    QString _firstName;
    QString _name;
    QString _status;
    QDateTime _statusTime;
    QString _statusTimeRel;
    QString _thumbSrc;
};

}

QDebug operator<<(QDebug dbg, const Facebook::BuddyInfo &buddy);


#endif
