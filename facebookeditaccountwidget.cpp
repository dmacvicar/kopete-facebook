

#include "facebookeditaccountwidget.h"

#include <qlayout.h>
#include <qlineedit.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <kdebug.h>
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "ui_facebookaccountpreferences.h"
#include "facebookaccount.h"
#include "facebookprotocol.h"

FacebookEditAccountWidget::FacebookEditAccountWidget( QWidget* parent, Kopete::Account* account)
  : QWidget( parent ), KopeteEditAccountWidget( account )
{
    QVBoxLayout *layout = new QVBoxLayout( this );
    kDebug(14210) ;
    QWidget *widget = new QWidget( this );
    m_preferencesWidget = new Ui::FacebookAccountPreferences();
    m_preferencesWidget->setupUi( widget );
    layout->addWidget( widget );
}

FacebookEditAccountWidget::~FacebookEditAccountWidget()
{
    delete m_preferencesWidget;
}

Kopete::Account* FacebookEditAccountWidget::apply()
{
    QString accountName;
    if ( m_preferencesWidget->m_acctName->text().isEmpty() )
        accountName = "Facebook Account";
    else
        accountName = m_preferencesWidget->m_acctName->text();
    
    if ( account() )
        // FIXME: ? account()->setAccountLabel(accountName);
        account()->myself()->setProperty( Kopete::Global::Properties::self()->nickName(), accountName );
    else
        setAccount( new FacebookAccount( FacebookProtocol::protocol(), accountName ) );
    
    return account();
}

bool FacebookEditAccountWidget::validateData()
{
    //return !( m_preferencesWidget->m_acctName->text().isEmpty() );
    return true;
}

#include "facebookeditaccountwidget.moc"
