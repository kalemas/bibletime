/*********
*
* In the name of the Father, and of the Son, and of the Holy Spirit.
*
* This file is part of BibleTime's source code, http://www.bibletime.info/.
*
* Copyright 1999-2014 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License version 2.0.
*
**********/

#ifndef BTCOLORWIDGET_H
#define BTCOLORWIDGET_H

#include <QFrame>


class QPaintEvent;

class BtColorWidget : public QFrame {
        Q_OBJECT

    public:
        BtColorWidget(QWidget* parent = 0);
        ~BtColorWidget();
        QSize sizeHint() const;

    public slots:
        void setColor(const QColor& color);

    protected:
        void mouseReleaseEvent(QMouseEvent* event);

    private:
        void showColorDialog();

        QColor m_color;

    signals:
        void changed(const QColor& color);
};

#endif


