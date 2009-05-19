
#ifndef FACEBOOKEDITACCOUNTWIDGET_H
#define FACEBOOKEDITACCOUNTWIDGET_H

#include <qwidget.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <editaccountwidget.h>

class QVBoxLayout;
namespace Kopete { class Account; }
namespace Ui { class FacebookAccountPreferences; }

/**
 * A widget for editing this protocol's accounts
 * @author Will Stephenson
*/
class FacebookEditAccountWidget : public QWidget, public KopeteEditAccountWidget
{
Q_OBJECT
public:
    FacebookEditAccountWidget( QWidget* parent, Kopete::Account* account);

    ~FacebookEditAccountWidget();

	/**
	 * Make an account out of the entered data
	 */
	virtual Kopete::Account* apply();
	/**
	 * Is the data correct?
	 */
	virtual bool validateData();
protected:
	Kopete::Account *m_account;
	Ui::FacebookAccountPreferences *m_preferencesWidget;
};

#endif
