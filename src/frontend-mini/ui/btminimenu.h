/*********
*
* In the name of the Father, and of the Son, and of the Holy Spirit.
*
* This file is part of BibleTime Mini project. Visit 
* http://sourceforge.net/projects/bibletimemini for more information.
*
* This code is licensed under the GNU General Public License version 2.0.
*
**********/

#ifndef BTMINIMENU_H
#define BTMINIMENU_H

#include <QMouseEvent>
#include <QWidget>

class BtMiniMenuPrivate;

/** Menu. Close child widget will close this menu.
*/
class BtMiniMenu : public QWidget
{
	Q_OBJECT
	
public:
	BtMiniMenu();
	~BtMiniMenu();

    static BtMiniMenu * createQuery(QString text, QStringList actions = QStringList());
    
    static BtMiniMenu * createProgress(QString text);
    
	static int execQuery(QString text, QStringList actions = QStringList());
	
	static int execMenu(QStringList actions);
	
	QWidget * buttonAt(int id) const;

    bool wasCanceled();

	bool eventFilter(QObject *watched, QEvent *e);

signals:
    void canceled();

public slots:
	void exec();
    void show();
    void hide();

    void cancel();

    void setValue(int percent);
    void setText(QString text);
	
private slots:
    void buttonTrigger();

protected:
    /** Reimplemented from QWidget. */
	bool event(QEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void mouseDoubleClickEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *e);
	void hideEvent(QHideEvent *e);
	QSize minimumSizeHint () const;
	QSize sizeHint () const;

private:
	BtMiniMenuPrivate * const d_ptr;

};

#endif