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

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QElapsedTimer>
#include <QMargins>
#include <QMouseEvent>
#include <QMutexLocker>
#include <QPainter>
#include <QScrollBar>
//#include <QStaticText>
#include <QTextDocument>
#include <QThread>
#include <QVariant>
#include <QWaitCondition>
#include <QtCore/qmath.h>
#include <QtDebug>

#include "btminilayoutdelegate.h"
#include "btminiview.h"

#include "text/btstatictext.h"

#ifndef QT_NO_WEBKIT
// actually not supported
//#define USE_WEBKIT
#endif

#ifdef USE_WEBKIT
#include <QWebElement>
#include <QWebFrame>
#include <QWebHitTestResult>
#include <QWebPage>
#include <QWebSettings>
#endif

// vibration
#ifdef Q_OS_WINCE
#include  <windows.h>
#include  <nled.h>

extern "C"
{
    BOOL WINAPI NLedSetDevice(UINT nDeviceId, void *pInput);
};

void __leds(bool on, int id)
{
    NLED_SETTINGS_INFO settings;
    settings.LedNum = id;
    settings.OffOnBlink = on ? 1 : 0;
    NLedSetDevice(NLED_SETTINGS_INFO_ID, &settings);
}
#endif

#define TURN_OFF_CACHED_ITEMS
#define SHORT_PRESS_DELAY        13
#define LONG_PRESS_DELAY         32
#define SCROLL_SNAPPING_SPEED    0.3f
#define SCROLL_ATTENUATION       0.93
#define CACHED_SURFACE_OVERLAP   0.5
#define SCROLL_BACK_ATTENUATION  13.285713041 // qreal v = 1.0, t = 0.0; while(v > 0.0000001) t += v *= SCROLL_ATTENUATION; return t;

/**
	Basic item to hold data and render text for view.
	There is no ploymorphysm, because for one object we may change
	implementation after object was created, so we use pointers to implementations.
*/
class BtMiniViewItem
{
private:
	class TextItem
	{
	public:
		TextItem(const QString &text, QFont font)
		{
			QVector<QPair<QString,Data>> stack;
			stack << QPair<QString,Data>(QString(),Data());

			stack.last().second.font = font;
			_data.append(stack.last().second);

			bool wordBreaks = false;
			bool newLine = false;

			// break on parts
			for(int c = 0; c < text.size();)
			{
				if(text[c] == '<')
				{
					bool end = false;
					
					if(text[c + 1] == '/')
						c++, end = true;

					unsigned int f = text.indexOf('>', c);
					QString tag = text.mid(c + 1, qMin(f, (unsigned int)text.indexOf(' ', c)) - c - 1);

					 // if have text, start new part
					if(_data.last().text.size() > 0)
						_data.append(stack.last().second);

					if(!end)
					{
						if(tag[tag.size() - 1] == '/')
							tag.resize(tag.size() - 1);
						else
							stack.append(QPair<QString,Data>(tag, stack.last().second));

						// modify data by tag names
						if(tag == "center")
							stack.last().second.center = true;
						else if(tag == "b")
							stack.last().second.font.setWeight(QFont::Bold);
						else if(tag == "font")
						{
							int e = text.indexOf('>', c);
							int l = text.indexOf("color", c);
							int s = text.indexOf("size", c);

							if(l < e && l >= 0)
							{
								QString v = text.mid(l + 7, 7);
								stack.last().second.color = QColor(v);
							}

							if(s < e && s >= 0)
							{
								int ss = s + 6;
								QString v = text.mid(ss, qMin((unsigned int)text.indexOf('\"', ss),
									(unsigned int)text.indexOf('\'', ss)) - ss);

								if(v == "1")
									stack.last().second.font.setPixelSize(font.pixelSize() / 2);
								else if(v == "+1")
									stack.last().second.font.setPixelSize(stack.last().second.font.pixelSize() * 1.2);
								else if(v[v.size() - 1] == '%')
									stack.last().second.font.setPixelSize(stack.last().second.font.pixelSize() /
										100.0 * v.left(v.size() - 1).toInt());
								else
									qDebug() << "Unable to resolve font size" << v;
							}
						}
						else if(tag == "word-breaks")
								wordBreaks = true;
						
						// set parameters from stack to empty part
						_data[_data.size() - 1] = stack.last().second;

						if(tag == "br" || tag == "p")
							newLine = true;
					}
					else
					{
						if(tag != stack.last().first)
							qDebug() << "TextItem, mismatched tags" << tag << stack.last().first << text;
						
						if(tag == "word-breaks")
							wordBreaks = false;

						stack.erase(stack.end() - 1);
					}

					c = f + 1;
					continue;
				}

				if(newLine && _data.last().text.size() == 0)
					_data.last().newLine = true, newLine = false;
					
				_data.last().text.append(text[c]);

				if(wordBreaks && text[c] == ' ')
					_data.append(stack.last().second);

				++c;
			}

			// strip last empty part
			if(_data.last().text.size() == 0)
				_data.erase(_data.end() - 1);

			// replace new line
			for(int i = 0; i < _data.size(); ++i)
			{
				if(_data[i].text == "&nbsp;")
				{
					_data[i].text = " ";
					if(i < _data.size() - 1)
						_data[i + 1].newLine = true;
				}
				Q_ASSERT(_data[i].text.indexOf("&nbsp;") == -1);
			}
		}

		~TextItem()
		{
			;
		}

		inline void paint(QPainter *painter, const QPoint &point, const QRect &clipping) const
		{
			painter->save();
			painter->setClipping(true);
			painter->setClipRect(clipping.translated(point));

			painter->eraseRect(clipping.translated(point));

			for(int i=0; i < _data.size(); ++i)
			{
				painter->setFont(_data[i].font);
				painter->setPen(_data[i].color);
				painter->translate(0, _data[i].size.height());
				painter->drawText(point + _data[i].pos, _data[i].text);
				painter->translate(0, -_data[i].size.height());

			}
			painter->restore();
		}

		/** Set width and prepare to display. */
		inline void resize(int width, int height)
		{
			_size.setWidth(width);
			layout();
		}

		inline QSize size() const
		{
			return _size;
		}

	private:
		void layout()
		{
			if(_data.size() == 0)
				return;

			const int ident = _data[0].font.pixelSize() * 0.5;
			const int width = _size.width();

			// sizes
			for(int i = 0; i < _data.size(); ++i)
			{
				QFontMetrics fm(_data[i].font);
				_data[i].size = QSize(fm.width(_data[i].text), _data[i].font.pixelSize());
				_data[i].pos = QPoint(0, ident * 0.75);
			}

			// lines and centering
			int h = _data[0].size.height();
			int w = _data[0].size.width();
			for(int i = 1; i < _data.size(); ++i)
			{
				_data[i].pos = QPoint(_data[i - 1].pos.x() + _data[i - 1].size.width(), _data[i - 1].pos.y());
				//_data[i].pos.setX(_data[i - 1].pos.x() + _data[i - 1].size.width());
				
				h = qMax(h, _data[i].size.height());

				// todo break part at particular character

				bool nl = w + _data[i].size.width() > width || _data[i].newLine;

				// center previous line, if width is exceeded
				if(nl && _data[i - 1].center)
				{
					int d = (width - w) / 2;
					for(int ii = i - 1; ii >= 0; --ii)
					{
						bool start = _data[ii].pos.x() == 0;
						_data[ii].pos.setX(qMax(ident, _data[ii].pos.x() + d));
						if(start)
							break;
					}
				}

				// move part to new line
				if(nl)
				{
					_data[i].pos = QPoint(0, _data[i].pos.y() + h);
					h = _data[i].size.height();
					w = _data[i].size.width();
				}
				else
					w += _data[i].size.width();
			}

			// finish last line
			if(_data.last().center)
			{
				int d = (width - w) / 2;
				for(int ii = _data.size() - 1; ii >= 0; --ii)
				{
					bool start = _data[ii].pos.x() == 0;
					_data[ii].pos.setX(qMax(ident, _data[ii].pos.x() + d));
					if(start)
						break;
				}
			}

			// set height
			_size.setHeight(_data.last().pos.y() + h + (ident * 1.5));
		}

		struct Data
		{
			Data()
			{
				font    = QFont();
				font.setStyleStrategy(QFont::NoAntialias);
				color   = Qt::black;
				center  = false;
				newLine = false;
			}

			QString  text;
			QFont    font;
			QPoint   pos;
			QSize    size;
			QColor   color;

			char     center:1;
			char     newLine:1; // part should be placed on new line
		};

		QVector<Data>  _data;
		QSize          _size;
	};

public:
    BtMiniViewItem()
    {
#ifdef USE_WEBKIT
        _wp              = 0;
#endif
        _selected        = false;
        _interactive     = false;
        _active          = true;
        _newLine         = true;
		_allowStaticText = true;
        _width           = 0;
        _height          = 0;
        _doc             = 0;
		_ti              = 0;
		_st              = 0;
    }
    
    virtual ~BtMiniViewItem()
    {
		clear();
    }

	/** Clear item text. */
	void clear()
	{
#ifdef USE_WEBKIT
		if(_wp)
			delete _wp, _wp = 0;
#endif
		if(_doc)
			delete _doc, _doc = 0;
		if(_ti)
			delete _ti, _ti = 0;
		if(_st)
			delete _st, _st = 0;
	}


	/** Detect whether text use allowed tags or unallowed if exclusive is true. */
	static bool isTextAcceptable(const QString &text, QStringList tags, bool exclusive = false)
	{
		QString tag;
		bool skip = true;
		
		for(int i = 0; i < text.size(); ++i)
		{
			if(text[i] == '<')
			{
				skip = false;
				tag = QString();
			}
			else if(skip == false)
			{
				if(text[i] == '>' || text[i] == ' ')
				{
					if((tags.indexOf(tag) == -1 && !exclusive) || 
						(tags.indexOf(tag) != -1 && exclusive))
						return false;
						
					skip = true;
				}
				else if(text[i] != '/')
					tag.append(text[i]);
			}
		}

		return true;
	}
    
    void setText(const QString &text, QWidget *widget = 0)
    {
		_scale = widget == 0 ? QApplication::font().pixelSize() : widget->font().pixelSize();

		clear();

		if(isTextAcceptable(text, QStringList() << "b" << "center" << "font" << "br" << "p" << "word-breaks"))
		{
			_ti = new TextItem(text, widget->font());
			resize(size());
			return;
		}

#ifdef USE_WEBKIT
        if(text.size() > 64)
        {
            QWebPage *wp = new QWebPage(widget);
            
            if(wp == 0)
            {
                qDebug("BtMiniViewItem::setText Can't allocate QWebPage");
                return;
            }

            wp->settings()->setFontFamily(QWebSettings::StandardFont, widget->font().family());
            wp->settings()->setFontSize(QWebSettings::DefaultFontSize, widget->font().pixelSize());
            wp->settings()->setFontSize(QWebSettings::DefaultFixedFontSize, widget->font().pixelSize());
            
            wp->setPreferredContentsSize(QSize(_width, 1));
            wp->mainFrame()->setHtml(text);

            wp->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
            wp->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

            _height = wp->mainFrame()->contentsSize().height();

            wp->setPreferredContentsSize(QSize(_width, _height));
            wp->setViewportSize(QSize(_width, _height));

            wp->moveToThread(QApplication::instance()->thread());

            qSwap(_wp, wp);

            if(wp)
                delete wp;

            return;
        }
#endif

        QString ct = text;
		
		if(!text.isEmpty())
        {
            Q_ASSERT(_width > 0);

            // HACK correct css to work with QTextDocument
            int cssStart = ct.indexOf("<style type=\"text/css\">");

            if(cssStart >= 0)
            {
                int contentStart = ct.indexOf("#content", cssStart);
                int cssEnd = ct.indexOf("</style>");

                // fix default font size
                if(contentStart >= 0 && contentStart < cssEnd)
                {
                    int fontSize = ct.indexOf("font-size:", contentStart);

                    if(fontSize >= 0 && fontSize < ct.indexOf("}", contentStart))
                    {
                        int column = ct.indexOf(":", fontSize) + 1;
                        ct.replace(column, ct.indexOf(";", fontSize) - column,
                            QString("%1px").arg(widget->font().pixelSize()));
                    }
                }

				// fix percent font-size
				for(int i = cssStart; i < cssEnd; )
				{
					int fontSize = ct.indexOf("font-size:", i);
					
					if(fontSize == -1)
						break;

					for(int ii = fontSize + 10; ; ++ii)
					{
						if(ct[ii] == '\n')
							break;
						else if(ct[ii] == '%')
						{
							int fs = fontSize + 10;
							int v = ct.mid(fs, ii - fs).toInt();
							ct = ct.replace(fs, ii - fs + 1, QString("%1px").arg(widget->font().pixelSize() * v / 100));
						}
					}

					i = fontSize + 10;

				}
            }
        }

		if(_allowStaticText && isTextAcceptable(ct, QStringList() << "img" << "table", true))
		{
			_st = new BtStaticText();
			_st->setText(ct);
			_st->prepare(QTransform(), widget->font());
			
			resize(size());
			return;
		}

		if(!_doc)
			_doc = new QTextDocument(widget);
        
		_doc->setDefaultFont(widget->font());
		_doc->setHtml(ct);
		
        resize(size());
    }

    /** Add icon to item. */
    void setIcon(QIcon &icon)
    {
        _icon = icon;
        resize(size());
    }

    /** Size Routines. */
    inline quint16 width() const { return _width; }
    inline quint16 height() const { return _height; }

    inline QSize size() const { return QSize(_width, _height); }

    inline void resize(QSize &newSize) { resize(newSize.width(), newSize.height()); }

    QSize contentsSize() const
    {
#ifdef USE_WEBKIT
        if(_wp)
            return _wp->mainFrame()->contentsSize();
#endif
        if(_doc)
            return QSize(_doc->idealWidth(), _doc->size().height());
		if(_st)
			return _st->size().toSize();
        return QSize();
    }

    void resize(const int width, const int height)
    {
        int w = _width = width;
		int h = 0;

        if(!_icon.isNull())
		{
            w -= _scale * 1.66;
			h = qMax(h, (int)(_scale * 1.66));
		}

		if(_ti)
		{
			_ti->resize(w, height);
			h = qMax(h, _ti->size().height());
		}

		if(_st)
		{
			_st->setTextWidth(w);
			h = qMax(h, (int)_st->size().height());
		}

#ifdef USE_WEBKIT
        if(_wp)
        {
            _wp->setPreferredContentsSize(QSize(w, 1));
            _height = _wp->mainFrame()->contentsSize().height();

            _wp->setPreferredContentsSize(QSize(w, _height));
            _wp->setViewportSize(QSize(w, _height));

            return;
        }
#endif
        if(_doc)
        {
            _doc->setTextWidth(w);
            h = qMax(h, (int)_doc->size().height());
        }

		_height = h;
    }

    /** Rendering. */
    virtual void paint(QPainter *painter, const QPoint &point, const QRect &clipping)
    {
        QPoint p = point;

        if(!_icon.isNull())
        {
            QRect rect(0, 0, _scale * 1.08, _scale * 1.08);
			rect.translate(_scale * 0.26, _scale * 0.33);

            if(!rect.intersected(clipping).isEmpty())
            {
                rect.translate(point);
                _icon.paint(painter, rect);
            }
            p.rx() += _scale * 1.66;
        }

#ifdef USE_WEBKIT
        if(_wp)
        {
            painter->save();
            painter->translate(p);
            _wp->mainFrame()->render(painter, clipping);
            painter->restore();
        }
#endif
        if(_doc)
        {
            painter->translate(p);
			_doc->drawContents(painter, clipping.translated(point.x() - p.x(), 0));
			painter->translate(-p);
        }

		if(_ti)
			_ti->paint(painter, p, clipping.translated(point.x() - p.x(), 0).intersected(
				QRect(0, 0, _width, _height)));

		if(_st)
		{
			QRect b(clipping.intersected(QRect(p.x() - point.x(), 0, _width, _height)).translated(point));

			painter->save();

			painter->setFont(_st->defaultFont());
			painter->setClipping(true);
			painter->setClipRect(clipping.translated(point));
			//painter->eraseRect(b);

			painter->drawStaticText(p, *((QStaticText*)_st));

			painter->restore();
		}
    }

public:
    char             _selected:1;
    char             _interactive:1;
    char             _active:1;

    /** Item is placed at the beginning of line. All line items have same height. */
    char             _newLine:1;

	char             _allowStaticText:1;
    
    qint32           _row;
    
    quint16          _width;
    quint16          _height;


    QIcon            _icon;
    quint16          _scale;

#ifdef USE_WEBKIT
    QWebPage        *_wp;
#endif
	QTextDocument   *_doc;
	TextItem        *_ti;
	BtStaticText    *_st;

};

class BtMiniSubView
{
public:
    BtMiniSubView()
    {
        clear();
    }
    
    virtual ~BtMiniSubView()
    {
        clear();
    }

    /** Render subview to screen or cache.
        \param rect is screen area, that needs to be painted.
        \param clipping is in local subview coordinates. */
    void paint(QPainter *painter, const QRect &rect, const QRect &clipping)
    {
#ifdef TURN_OFF_CACHED_ITEMS
        QPair<int, int> _cachedTopItem;
#endif

        for(int n = _cachedTopItem.first, x = 0, y = 0,
            y2 = _cachedTopItem.second; n < _items.size(); ++n)
        {
            if(y > clipping.bottom())
                break;
    
            BtMiniViewItem *i = _items[n];

            if(i->_newLine)
                y = y2, y2 += i->size().height(), x = 0;

            if(y2 >= clipping.top())
            {
                QPoint p(x, y);
                QRect r(QPoint(), i->size());
                r &= clipping.translated(-p);
                if(!r.isEmpty())
                    i->paint(painter, rect.topLeft() - clipping.topLeft() + p, r);
            }

            x += i->size().width();
        }
    }

    void clear()
    {
        _cachedLastItem = _cachedTopItem = _cachedBottomItem = qMakePair(0, 0);
        _resizeCursor   = -1;

        _rect.setHeight(0);
        _rect.moveTop(0);

        foreach(BtMiniViewItem *i, _items)
		{
            delete i;
		}

        _items.clear();

		_viualCenter = 0.0;
    }

    /** Work with model. */
    void setModelIndex(const QModelIndex &index)
    {
        _index = index;
        
        if(index.isValid())
        {
            _parentIndex = index.parent();
            _rowCount = index.model()->rowCount(_parentIndex);
            
            //Q_ASSERT(_rowCount > 0);
        }
        else
        {
            _parentIndex = index;
            _rowCount = 0;
        }

        clear();
    }

    void updateModelIndex(const QModelIndex &index)
    {
        Q_ASSERT(_index.parent() == index.parent());

		//qDebug() << "updateModelIndex" << index;

        _index = index;
    }

    inline const QModelIndex & modelIndex() const
    {
        return _index;
    }

    QModelIndex modelIndex(const int row) const
    {
        return _index.model()->index(row, 0, _parentIndex);
    }

    inline const QModelIndex & modelParentIndex() const
    {
        return _parentIndex;
    }

    const int & rowCount() const
    {
        Q_ASSERT(_items.size() <= _rowCount);

        return _rowCount;
    }
    
    /** Get/set contents rectangle. Setting top value receives new absolute
        position of contents. */
    void setContentsTop(float vertical)
    {
        int ot = _rect.top();

        _rect.moveTop(vertical);

        if(ot == (int)_rect.top())
            return;

        //qDebug() << "setContentsTop" << vertical;
        
        updateCachedItems();
    }
    void setContentsLeft(int horizontal)
    {
        _rect.moveLeft(horizontal);
    }
    void setContentsSize(int w, int h)
    {
        _rect.setWidth(w);
        _rect.setHeight(h);
    }
    inline const QRectF & contentsRect() const
    {
        return _rect;
    }

    /** Get/set base subview size. */
    void setBaseSize(QSize &size)
    {
        //qDebug() << "setBaseSize" << size << contentsRect();

        _size = size;

        if(_items.size() > 0)
            _resizeCursor = 0;

        setContentsSize(baseSize().width(), contentsRect().height());
    }
    inline const QSize & baseSize() const
    {
        return _size;
    }


    /** */
    void updateCachedItems()
    {
        if(_items.size() == 0)
            return;

#ifdef QT_DEBUG
        {
            int height = 0;
            for(int i = 0; i < _items.size(); ++i)
                if(_items[i]->_newLine)
                    height += _items[i]->size().height();
            Q_ASSERT(height == contentsRect().height());
        }
#endif

#ifdef TURN_OFF_CACHED_ITEMS
        return;
#endif

        // last new line item
        _cachedLastItem.first = _items.size() - 1;
        while(!_items[_cachedLastItem.first]->_newLine)
            --_cachedLastItem.first;
        _cachedLastItem.second = _rect.height() - _items[_cachedLastItem.first]->size().height();

        // first new line item on screen
        for(int i = _cachedTopItem.first - 1; i >= 0; --i)
        {
            if(_rect.top() + _cachedTopItem.second <= 0)
                break;

            if(_items[i]->_newLine)
            {
                qDebug() << "_cachedTopItem --" << _cachedTopItem;

                _cachedTopItem.first = i;
                _cachedTopItem.second -= _items[i]->size().height();

                qDebug() << "\t" << _cachedTopItem;
            }
        }

        for(int i = _cachedTopItem.first + 1; i <= _cachedLastItem.first; ++i)
        {
            if(_rect.top() + _cachedTopItem.second + _items[_cachedTopItem.first]->size().height() > 0)
                break;

            if(_items[i]->_newLine)
            {
                qDebug() << "_cachedTopItem ++" << _cachedTopItem;

                _cachedTopItem.second += _items[_cachedTopItem.first]->size().height();
                _cachedTopItem.first = i;

                qDebug() << "\t" << _cachedTopItem;

            }
        }

        // last new line item on screen
        _cachedBottomItem = _cachedTopItem;

        for(int i = _cachedTopItem.first + 1; i <= _cachedLastItem.first; ++i)
        {
            if(_rect.top() + _cachedBottomItem.second + 
                _items[_cachedBottomItem.first]->size().height() > _size.height())
                break;

            if(_items[i]->_newLine)
            {
                _cachedBottomItem.second += _items[_cachedBottomItem.first]->size().height();
                _cachedBottomItem.first = i;
            }
        }

        Q_ASSERT(_cachedTopItem.first == 0 ? _cachedTopItem.second == 0 : true);
        Q_ASSERT(_cachedTopItem.first == _cachedLastItem.first ? _cachedTopItem.second ==
            _cachedLastItem.second : true);
        Q_ASSERT(_cachedBottomItem.first == _cachedLastItem.first ? _cachedBottomItem.second ==
            _cachedLastItem.second : true);
    }

    /** */
    QPoint itemXy(const int pos) const
    {
        QPair<int, int> p;

#ifndef TURN_OFF_CACHED_ITEMS
        if(pos == 0)
            return QPoint();
        else if(pos == _cachedTopItem.first)
            return QPoint(0, _cachedTopItem.second);
        else if(pos == _cachedBottomItem.first)
            return QPoint(0, _cachedBottomItem.second);
        else if(pos < _cachedTopItem.first)
            ;
        else if(pos < _cachedBottomItem.first)
            p = _cachedTopItem;
        else if(pos >= _cachedLastItem.first)
            p = _cachedLastItem;
        else
            p = _cachedBottomItem;
#endif

        for(int i = p.first, x = 0, y = p.second, y2 = y; i < _items.size(); ++i)
        {
            if(_items[i]->_newLine)
                x = 0, y = y2, y2 += _items[i]->size().height();
            else
                x += _items[i - 1]->size().width();

            if(i == pos)
                return QPoint(x, y);
        }

        Q_ASSERT(false);
        return QPoint();
    }

    /** */
    int xyItem(const QPoint &point) const
    {
#ifdef TURN_OFF_CACHED_ITEMS
        QPair<int,int> _cachedTopItem;
#endif

        QRect r(0, 0, baseSize().width(), _cachedTopItem.second);

        Q_ASSERT(!r.contains(point));

        for(int i = _cachedTopItem.first, x = 0, y = 0,
            y2 = _cachedTopItem.second; i < _items.size(); ++i)
        {
            if(_items[i]->_newLine)
                r.moveTo(0, r.bottom() + 1);

            r.setSize(_items[i]->size());

            if(r.contains(point))
                return i;

            r.moveLeft(r.right() + 1);
        }

        return -1;
    }

    /** Return item index of given model index. If index not created return -1. */
    int indexItem(const QModelIndex &index) const
    {
        Q_ASSERT(index.parent() == _parentIndex);

        if(_items.size() == 0 || _items[0]->_row > index.row() ||
            _items[_items.size() - 1]->_row < index.row())
            return -1;

        return index.row() - _items[0]->_row;
    }

    /** Return rect for given model index. */
    QRect indexRect(const QModelIndex &index) const
    {
        Q_ASSERT(index.parent() == _parentIndex);

        for(int n = 0, r = index.row(); n < _items.size(); ++n)
        {
            if(_items[n]->_row == r)
                return QRect(itemXy(n), _items[n]->size());
        }

        return QRect();
    }

    /** Return model index at given point. Point is in local subview 
        coordinates. */
    QModelIndex indexAt(const QPoint &point) const
    {
        int i = xyItem(point);

        return i == -1 ? QModelIndex() : modelIndex(_items[i]->_row);
    }

public:
    /** Items. */
    QList<BtMiniViewItem*>  _items;
    
    /** Value represents current progress of resize. Default value is -1 mean
        that view doesn't need to be resized. */
    int                     _resizeCursor;
    
    /** Top and bottom screen item, first is index, second is vertical displacement.
        It is always calculated with _size property. Those items are always placed
        on new line, so last items is first item on last line. */
    QPair<int, int>         _cachedTopItem;
    QPair<int, int>         _cachedBottomItem;
    QPair<int, int>         _cachedLastItem;
	
	/** If user just opened view or scrolled to an index, necessary to determine when resize
		thread computed items from center or top. */
	float                   _viualCenter;


private:
    /** Contents rectangle, top left point used by parent to hold its 
		position in subviews structure. X right, Y down. When scroll
		this value moves up/down, so top can be negative value. */
    QRectF                  _rect;

    /** Base subview size, it is equal to the size of parent's viewport. 
		Deprecated. */
    QSize                   _size;

    /** Subview base index in the model used for this view. Index and all 
    items on it's level will be used, not children indexes. */
    QPersistentModelIndex   _index;
    QPersistentModelIndex   _parentIndex;
    int                     _rowCount;

};

class BtMiniViewPrivate
{
public:
    BtMiniViewPrivate(BtMiniView *q) : q_ptr(q)
    {
        _vx              = 0.0f;
        _vt              = 0;
        _currentSubView  = 0;
        _mouseDown       = false;
        _interactive     = false;
        _enableTopShadow = false;
		_needScroll      = false;

		_needEmitCurrentChanged = false;
        
        _limitHeightTop      = 0;
        _limitHeightBottom   = 0;
        
        _timer.start();

        _ld = new BtMiniLayoutDelegate(q);

		_useRenderCaching = false;
		_cachedSurface    = 0;

		_sleep            = false;
    }

    ~BtMiniViewPrivate()
    {
		Q_Q(BtMiniView);

        clear();

		if(_cachedSurface)
			delete _cachedSurface;

		foreach(BtMiniViewThread *t, _threads)
		{
			t->stop();
			delete t;
		}
    }

    inline BtMiniSubView * currentSubView()
    {
        Q_ASSERT(_currentSubView < _subViews.size() && _currentSubView >= 0);
        return _subViews[_currentSubView];
    }
    inline const BtMiniSubView * currentSubView() const
    {
        Q_ASSERT(_currentSubView < _subViews.size() && _currentSubView >= 0);
        return _subViews[_currentSubView];
    }

    void clear()
	{
		_mutex.lock();
		_modelWork.clear();
		_mutex.unlock();

        foreach(BtMiniSubView* v, _subViews)
            delete v;

        _subViews.clear();
    }

    /** After this function subview will contain \param index. */
    void checkIndexCreated(int subView, const QModelIndex &index)
    {
		Q_Q(BtMiniView);

        BtMiniSubView *v = _subViews[subView];

		Q_ASSERT(index.model() == q->model());

        if(v->indexItem(index) == -1)
        {
            Q_ASSERT(_ld->levelOption(subView).perCycle > 0);
            
            v->clear();
            layoutItems(subView, _ld->levelOption(subView).perLine);
            
            Q_ASSERT(v->indexItem(index) >= 0);
        }
    }

    /** Only create subviews if silent mode and invalid index is passed. */
    void activateIndex(const QModelIndex &index, bool silent,
        QAbstractItemView::ScrollHint hint = QAbstractItemView::EnsureVisible)
    {
        Q_Q(BtMiniView);

        //qDebug() << "activateIndex" << index << silent << hint;

        QVector<QModelIndex> parents;

        Q_CHECK_PTR(q->model());

        if(!index.isValid())
            parents << q->model()->index(0, 0);
        else
            parents << index;

        while(parents.front().isValid())
            parents.prepend(parents.front().parent());

        int id = 0;

        if(_ld->plainMode())
        {
            if(parents.size() == 2)
                parents.append(q->model()->index(0, 0, parents[1]));

            Q_ASSERT(parents.size() == 3);

            // create subviews
            int rc = q->model()->rowCount();
            if(_subViews.size() == 0)
            {
                for(int r = 0; r < rc; ++r)
                    q->makeSubView(r, q->model()->index(0, 0, q->model()->index(r, 0)));
            }
            else
            {
                Q_ASSERT(rc == _subViews.size());
            }

            // find subview
            for(; id < _subViews.size(); ++id)
			{
                if(_subViews[id]->modelParentIndex() == parents[1])
                    break;
			}
        }
        else
        {
            for(; id < parents.size() - 1; ++id)
			{
                if(_subViews.size() <= id || _subViews[id]->modelParentIndex() != parents[id])
                    q->makeSubView(id, parents[id + 1]);

				if(index.isValid())
					_subViews[id]->updateModelIndex(parents[id + 1]);
			}
            
			id -= 1;
        }

        // if there is nothing
        if(_subViews.size() == 0)
        {
            Q_ASSERT(parents.size() == 2);
            q->makeSubView(0, parents.back());
		}

		if(index.isValid())
			_subViews[id]->updateModelIndex(parents.back());

        if(!silent)
        {
            q->activateSubView(id);
			q->scroll(_vx - currentSubView()->contentsRect().left(), 0.0f);

			_needScroll = true;
			_scrollHint = hint;
        }
    }

    /** Update views geometry. */
    void updateViews()
    {
        Q_Q(BtMiniView);

        //qDebug() << "updateViews";

		if(_subViews.size() > 0)
        {
			q->setVerticalScrollBarPolicy(_ld->levelOption(_currentSubView).scrollBarPolicy);

            // rearrange subviews
            for(int i = 0; i < _subViews.size(); ++i)
            {
                if(i > 0)
                {
                    const int d = _subViews[i - 1]->contentsRect().right() + 1 - _subViews[i]->contentsRect().left();
                    if(d != 0)
                    {
                        _subViews[i]->setContentsLeft(_subViews[i - 1]->contentsRect().right() + 1);
                        if(i == _currentSubView)
                            _vx += d;
                    }
                }
                else
                {
                    const int d = -_subViews[0]->contentsRect().left();
                    if(d != 0)
                    {
                        _subViews[i]->setContentsLeft(0);
                        if(i == _currentSubView)
                            _vx += d;
                    }
                }
            }

			updateScrollBars();
		}
    }

    /** General callback on area change. This will only scroll subview, should be called after 
		items are changed. Parameters are subview local. */
    void areaChanged(int subView, const int from, const int to, const int height, float resizeFactor = 0.0)
    {
        BtMiniSubView *v = _subViews[subView];
        const int d = (from - to) + height;

		// needs to update view
		if(from + v->contentsRect().top() < v->baseSize().height() &&
			to + v->contentsRect().top() > 0)
			q_func()->viewport()->update();

		//qDebug() << "updateHeight" << from << to << d << height << v->contentsRect() << _cachedRect;

		// update render cache
		if(_useRenderCaching && !_cachedRect.isEmpty())
		{
			if(to < _cachedRect.top())
				_cachedRect.translate(0, d);
			else if(from < _cachedRect.bottom())
				_cachedRect = QRect();
		}

		if(to - from == d)
			return;

		const int oldHeight = v->contentsRect().height();

		v->setContentsSize(v->contentsRect().width(), oldHeight + d);

		if(from == to)       // add
		{
			if(oldHeight == 0)
				v->setContentsTop(0);
			else if(from == 0)
				v->setContentsTop(v->contentsRect().top() - d);
		}
		else if(height == 0) // remove
		{
			if(from == 0)
				v->setContentsTop(v->contentsRect().top() - d);
		}
		else                 // resize
		{
			if(v->contentsRect().top() + to <= v->baseSize().height() * resizeFactor)
				v->setContentsTop(v->contentsRect().top() - d);
		}

		updateScrollBars();
    }

    /** Update items sizes on line at \param pos. Assures that line of items has 
        view width and all items has same height. */
    void updateLine(const int subView, const int pos)
    {
        BtMiniSubView *v = _subViews[subView];
        
        //qDebug() << "updateLine" << pos << v->contentsRect();

        Q_ASSERT(v->_items[0]->_newLine);

        int start = pos;
        int end = pos + 1;
        
        bool multiItem = _ld->levelOption(subView).perLine > 1;
        
        if(multiItem)
        {
            while(start >= 1 && !v->_items[start]->_newLine)
                --start;
            while(end < v->_items.size() && !v->_items[end]->_newLine)
                ++end;
        }

        int contentsWidth = 0;
        int width = 0;
        for(int i = start; i < end; ++i)
            contentsWidth += v->_items[i]->contentsSize().width(), 
            width         += v->_items[i]->size().width();

        bool b = width != v->contentsRect().width();
        
        if(multiItem)
        {
            // check height
            for(int i = start; i < end; ++i)
                if(v->_items[i]->height() != v->_items[start]->height())
                    b = true;
        }

        if(b)
        {
            int maxHeight = 0;
            const int oldHeight = v->_items[start]->size().height();
            width = v->contentsRect().width();

            // resize width
            for(int i = start; i < end; ++i)
            {
                if(i < end - 1)
                {
                    const int newWidth = qFloor(v->_items[i]->contentsSize().width() /
                        (qreal)contentsWidth * v->contentsRect().width());

                    v->_items[i]->resize(newWidth, maxHeight);

                    width -= newWidth;
                }
                else
                    v->_items[i]->resize(width, maxHeight);

                maxHeight = qMax(maxHeight, v->_items[i]->size().height());
            }

            // resize height
            for(int i = start; i < end; ++i)
                v->_items[i]->resize(v->_items[i]->size().width(), maxHeight);

            // finally update on viewport if necessary
            const int p = v->itemXy(start).y();
            areaChanged(subView, p, p + oldHeight, maxHeight);
        }

#ifdef QT_DEBUG
        {
            // after this procedure every items line must be width of subview
            int width = 0;
            for(int i = start; i < end; ++i)
                width += v->_items[i]->size().width();
            Q_ASSERT(width == v->contentsRect().width());
        }
#endif

        // no need to resize this line again, move resize pointer
        if(v->_resizeCursor >= start &&  v->_resizeCursor < end)
            v->_resizeCursor = end - 1;
    }
    
    /** Create item for subview, change subview contents size and 
        do necessary scrolling, shift render cache if necessary. */
    void addItem(const int subView, const QModelIndex &index, const int insertAt, const int width)
    {
        Q_Q(BtMiniView);
        
        BtMiniSubView  *v  = _subViews[subView];

        // insert at beginning or end supported for now
        Q_ASSERT(insertAt == 0 || insertAt == v->_items.size());

		const BtMiniLayoutOption &o = _ld->levelOption(subView);

		BtMiniViewItem *item = new BtMiniViewItem;

		item->_row = index.row();
		item->resize(width, 0);
		item->_allowStaticText = o.allowStaticText;

		// Icon
		QVariant decoration = index.data(Qt::DecorationRole);
		QIcon icon;
		if(decoration.canConvert(QVariant::Icon))
			icon = decoration.value<QIcon>();

		if(!icon.isNull())
			item->setIcon(icon);

		// text
		item->setText(o.preText + index.data(o.useThread ? BtMini::PreviewRole :
			Qt::DisplayRole).toString() + o.postText, q);

		// threaded processing
		if(o.useThread)
		{
			QMutexLocker locker(&_mutex);
			_modelWork.append(index);
		}

        // newline
        bool newLine = true;

        if(o.perLine > 1)
        {
            int width = 0;
            
            if(insertAt > 0)
            {
                int i = qMin(insertAt - 1, v->_items.size() - 1);
                for(; i >= 0; --i)
                {
                    width += v->_items[i]->contentsSize().width();
                    if(v->_items[i]->_newLine)
                        break;
                }

                if(insertAt - i >= o.perLine)
                    newLine = true;
                else
                    newLine = v->_items.size() == 0 || v->contentsRect().width() <
                        (width + item->contentsSize().width());

                item->_newLine = newLine;
            }
            else if(v->_items.size() > 0)
            {
                int i = 1;
                width = v->_items[0]->contentsSize().width();

                for(; i < v->_items.size(); ++i)
                {
                    if(v->_items[i]->_newLine)
                        break;
                    width += v->_items[i]->contentsSize().width();
                }

                item->_newLine = true;

                if(v->contentsRect().width() > (width + item->contentsSize().width()) && i < o.perLine)
                {
                    newLine = v->_items[0]->_newLine = false;
                    item->resize(item->width(), v->_items[0]->height());
                }
            }
        }

        v->_items.insert(insertAt, item);

        // Update subview height
        if(newLine)
        {
            const int p = insertAt == 0 ? 0 : v->contentsRect().height();
            areaChanged(subView, p, p, item->height());
        }
        else
        {
            updateLine(subView, insertAt);
        }
    }
    
    /** Remove item from subview, change subview contents size and do necessary 
        scrolling. If multiline mode enabled, whole line will be deleted. */
    void removeItem(const int subView, const int pos)
    {
        BtMiniSubView  *v  = _subViews[subView];

        int start = pos;
        int end = pos + 1;

        while(start >= 1 && !v->_items[start]->_newLine)
            --start;
        while(end < v->_items.size() && !v->_items[end]->_newLine)
            ++end;
    
        //qDebug() << "removeItem" << pos << start << end;

        const int y = v->itemXy(start).y();
        const int h = v->_items[start]->size().height();

#ifndef TURN_OFF_CACHED_ITEMS
        // pre update cached items
        if(v->_cachedTopItem.first >= end)
        {
            qDebug() << "change cached top" << v->_cachedTopItem << "minus" << end - start << h;

            v->_cachedTopItem.first  -= end - start;
            v->_cachedTopItem.second -= h;
        }
        else
            Q_ASSERT(v->_cachedTopItem.first < start);
#endif

#ifdef QT_DEBUG
        for(int i = start; i < end; ++i)
            Q_ASSERT(i == start ? v->_items[i]->_newLine : !v->_items[i]->_newLine);
        Q_ASSERT(end == v->_items.size() || v->_items[end]->_newLine);
#endif

		// erase items
        for(int i = start; i < end; ++i)
        {
            //qDebug() << "erase" << v->_items[start]->_doc->toPlainText();

			// remove from threaded processing
			_mutex.lock();
			int ii = _modelWork.indexOf(v->modelIndex(v->_items[start]->_row));
			if(ii >= 0)
				_modelWork.removeAt(ii);
			_mutex.unlock();

            delete v->_items[start];
            v->_items.erase(v->_items.begin() + start);
        }

        areaChanged(subView, y, y + h, 0);

        Q_ASSERT(v->_items[0]->_newLine);
    }
    
    /** */
    void layoutItemsFull(const int subView)
    {
        BtMiniSubView *v = _subViews[subView];

        v->clear();
		
		for(int row = 0, count = v->rowCount(); row < count; ++row)
            addItem(subView, v->modelIndex(row), v->_items.size(), v->baseSize().width());
    }
    
    /** Layout one item, detect where it is necessary. */
    void layoutItems(const int subView, int amount)
    {
        BtMiniSubView *v = _subViews[subView];

        if(v->rowCount() <= v->_items.size())
            return;

        const BtMiniLayoutOption &o = _ld->levelOption(subView);

        for(int l = 0; l < amount; ++l)
        {
            bool t = -v->contentsRect().top() - _limitHeightTop < 
                v->contentsRect().bottom() - v->baseSize().height() - _limitHeightBottom;
            bool ct = v->_items.size() == 0 || v->_items[0]->_row > 0;
            bool cb = v->_items.size() == 0 || v->_items[v->_items.size() - 1]->_row < v->rowCount() - 1;

            if(v->_items.size() < o.perLine && o.perLine > 1)
                t = false;

            if(o.limitItems)
            {
                if(-v->contentsRect().top() - _limitHeightTop > 0)
                    ct = false;
                if(v->contentsRect().bottom() - v->baseSize().height() - _limitHeightBottom > 0)
                    cb = false;
            }

            if(t && !ct)
                t = false;
            if(!t && !cb)
                t = true;

            if(t && !ct || !t && !cb)
                return;

            int r = v->modelIndex().row();

            if(v->_items.size() > 0)
                r = t ? v->_items[0]->_row - 1 : v->_items[v->_items.size() - 1]->_row + 1;
            else if(o.perLine > 1)
                r = r - (r % o.perLine);

            addItem(subView, v->modelIndex(r), t ? 0 : v->_items.size(), v->baseSize().width());
        }
    }
    
    /** Return displacement of borders of given subview relative to view rect.
        All margins are positive if subview is inside of view. */
    inline QMargins subViewMargins(const int subView) const
    {
        Q_Q(const BtMiniView);

        const QRect rect(q->viewport()->rect().translated(_vx, 0));
        const QRect cont(_subViews[subView]->contentsRect().toRect());

        return QMargins(cont.left() - rect.left(), cont.top(),
                        rect.right() - cont.right(), rect.bottom() - cont.bottom());
    }

    /** Update scrollbars and viewport. */
    void updateScrollBars()
    {
        Q_Q(BtMiniView);

        q->horizontalScrollBar()->setMaximum(_subViews[_subViews.size()-1]->contentsRect().left());

        q->horizontalScrollBar()->setValue(qMax((qreal)q->horizontalScrollBar()->minimum(),
            qMin((qreal)q->horizontalScrollBar()->maximum(), _vx)));

		if(_ld->levelOption(_currentSubView).scrollPerItem)
		{
			q->verticalScrollBar()->setMaximum(currentSubView()->rowCount() - 1);
			q->verticalScrollBar()->setValue(currentSubView()->modelIndex().row());
		}
		else
		{
			q->verticalScrollBar()->setMaximum(qMax((qreal)0,
				currentSubView()->contentsRect().height() - q->viewport()->height()));

			q->verticalScrollBar()->setValue(qMax((qreal)q->verticalScrollBar()->minimum(),
				qMin((qreal)q->verticalScrollBar()->maximum(), -currentSubView()->contentsRect().top())));
		}

        _vt = q->verticalScrollBar()->value();
    }

    /** Cyclic procedure. */
    void updateSubView(int subView)
	{
		Q_Q(BtMiniView);

        BtMiniSubView *v = _subViews[subView];

		// resize items
        if(v->_resizeCursor >= 0)
        {
            for(int i = 0, c = qMax(1, _ld->levelOption(subView).perCycle), s = v->_resizeCursor; i < c; ++i)
            {
                if(v->_resizeCursor >= v->_items.size())
                {
                    v->_resizeCursor = -1;
                    break;
                }
                else if(v->_resizeCursor - s > c)
                    break;
                else
                    updateLine(subView, v->_resizeCursor++);
            }
        }

        const BtMiniLayoutOption &o = _ld->levelOption(subView);

        // remove unnecessary items
        if(o.limitItems && v->_items.size() > 0)
        {
            if(_limitHeightTop > 0)
            {
                int i = v->xyItem(QPoint(0, -v->contentsRect().top() - _limitHeightTop));

                if(i > 3)
                    while(--i > 0)
                        removeItem(subView, 0);
            }
            if(_limitHeightBottom)
            {
                int i = v->xyItem(QPoint(0, -v->contentsRect().top() + v->baseSize().height() +
                    _limitHeightBottom));

                if(i >= 0)
                    i = v->_items.size() - 1 - i;

                if(i > 3)
                    while(--i > 0)
                        removeItem(subView, v->_items.size() - 1);
            }
        }

		// update threading
		bool workDone = false;
		bool simulate = true;

		// start threads
		if(o.useThread && !simulate)
		{
			if(_threads.size() == 0)
				_threads.append(new BtMiniViewThread(this));

			for(int i = 0; i < _threads.size(); ++i)
				if(!_threads[i]->isRunning())
					_threads[i]->start(QThread::LowPriority);
		}

		// calculate priority
		if(_modelWork.size() > 0)
		{
			_mutex.lock();
			QVector<int> distance(_modelWork.size(), INT_MAX);

			for(int i = 0; i < _modelWork.size(); ++i)
			{
				// calculate rating
				if(v->modelParentIndex() == _modelWork[i].parent())
				{
					int ii = v->indexItem(_modelWork[i]);
					if(ii >= 0)
					{
						int y = v->itemXy(ii).y() + v->contentsRect().top();
							
						if(v->_viualCenter != 0.0)
							distance[i] = qAbs(y + (v->_items[ii]->height() / 2) - (v->baseSize().height() * v->_viualCenter));
						else if(y <= 0 && y + v->_items[ii]->height() > 0)
							distance[i] = qAbs(y);
						else
							distance[i] = y > 0 ? y : qAbs(y) + v->baseSize().height();
					}
				}

				// sort array
				for(int ii = i - 1; ii >= 0; --ii)
				{
					if(distance[ii] > distance[ii + 1])
					{
						qSwap(distance[ii], distance[ii + 1]);
						qSwap(_modelWork[ii], _modelWork[ii + 1]);
					}
				}
			}

			if(distance[0] == INT_MAX)
				simulate = false;
			
			_mutex.unlock();
		}

		// paste calculated data
		while(_modelDone.size() > 0)
		{
			QPair<QModelIndex, QString> done = _modelDone.takeAt(0);
			QModelIndex parent = done.first.parent();

			for(int v = 0; v < _subViews.size(); ++v)
			{
				if(parent == _subViews[v]->modelParentIndex())
				{
					int i = _subViews[v]->indexItem(done.first);
					if(i != -1)
					{
						QRect r(_subViews[v]->itemXy(i), _subViews[v]->_items[i]->size());

						// TODO obtain subView options

						_subViews[v]->_items[i]->setText(done.second, q);
						areaChanged(v, r.top(), r.bottom() + 1, _subViews[v]->_items[i]->height(), 
							_subViews[v]->_viualCenter);

						workDone = true;
					}
				}
			}
		}

		if(!workDone && simulate && _modelWork.size() > 0)
		{
			_modelDone.append(QPair<QModelIndex, QString>(_modelWork[0], _modelWork[0].data().toString()));
			_modelWork.erase(_modelWork.begin());
			workDone = true;
		}

        if(o.perCycle > 0 && !workDone)
            layoutItems(subView, o.perCycle);
    }

public:
	class BtMiniViewThread : public QThread
	{
	public:
		BtMiniViewThread(BtMiniViewPrivate *view)
		{
			_stop  = false;
			_view = view;
		}

		~BtMiniViewThread()
		{
			Q_ASSERT(!isRunning());
		}

		void stop()
		{
			_stop = true;

			// wait before thread stops execution
			QMutex mutex;
			mutex.lock();

			QWaitCondition waitCondition;

			for(int i = 0; i < 100 && isRunning(); ++i)
				waitCondition.wait(&mutex, 20);

			mutex.unlock();

			// thread won't stop
			if(isRunning())
			{
				qDebug() << "Termination of thread.";
				terminate();
			}
		}

		void run()
		{
			qDebug() << "thread" << this << "started";

			_stop = false;

			while(!_stop)
			{
				_view->_mutex.lock();
				
				if(_view->_modelWork.size() == 0)
				{
					_view->_mutex.unlock();
					msleep(200);
					continue;
				}

				QModelIndex index = _view->_modelWork.takeAt(0);
				_view->_mutex.unlock();

				//msleep(2000);
				QString text(index.data().toString());

				_view->_modelDone.append(QPair<QModelIndex, QString>(index, text));
			}

			qDebug() << "thread" << this << "stoped";
		}

		bool                        _stop;
		BtMiniViewPrivate          *_view;
	};

public:
    /** User feedback. */
    QPoint                        _mouseLast;
    QPoint                        _mouseStart;
    bool                          _mouseLeaveZone;
    bool                          _mouseDown;
    int                           _mouseTapping;
    QPair<QPoint, qint64>         _mouseScrolling[2];
    QPointF                       _mousePower;
	
	int                           _sizeFactor;
    
    /** Snapping. */
    int                           _snappingValue;
    int                           _snappingLeft;
    int                           _snappingRight;
    
    QElapsedTimer                 _timer;
    int                           _eventTimer;

    /** Viewport position. Float values necessary for smooth scrolling.
        _vx is horizontal viewport position, vertical position is always 0. */
    qreal                         _vx;

    /** Tracking for vertical scrollbar. */
    int                           _vt;

    /** Subviews. */
    QList<BtMiniSubView*>         _subViews;
    int                           _currentSubView;
    
    /** Layout items. */
    int                           _limitHeightTop;
    int                           _limitHeightBottom;
    
    /** Thread for background calculations launched. Should be set true in 
	main thread and set false in created thread. */
	QList<QModelIndex>                 _modelWork;
	QList<QPair<QModelIndex, QString>> _modelDone;
	QVector<BtMiniViewThread*>         _threads;
	QMutex                             _mutex;
	bool                               _sleep;
    
    /** Item and subview layout delegate. */
    BtMiniLayoutDelegate         *_ld;

    /** */
    bool                          _interactive;

    /** */
    bool                          _enableTopShadow;

	QAbstractItemView::ScrollHint _scrollHint;
	bool                          _needScroll;
	bool                          _needEmitCurrentChanged;

	// we cache current subview only, in two parts
	bool                          _useRenderCaching;
	QPixmap                      *_cachedSurface;
	QRect                         _cachedRect;

    Q_DECLARE_PUBLIC(BtMiniView);
    BtMiniView * const             q_ptr;
};

BtMiniView::BtMiniView(QWidget *parent) : QAbstractItemView(parent), d_ptr(new BtMiniViewPrivate(this))
{
    Q_D(BtMiniView);

	// Setup font / local size
	QFont f(parent ? parent->font() : font());
	f.setPixelSize(f.pixelSize() * 1.1);
	setFont(f);
		
	// Get global size factor
    {
        QWidget *w = this;
        while(w->parentWidget())
            w = w->parentWidget();

		d->_sizeFactor = w->font().pixelSize();

        const int b = style()->pixelMetric(QStyle::PM_MenuPanelWidth) * 2;
        
        if(w->width() < w->height())
        {
            setMinimumWidth(qMin(w->width() - b, (int)(w->width() * 0.8)));
            setMinimumHeight(w->height()*0.7);
        }
        else
        {
            setMinimumWidth(w->width()*0.7);
            setMinimumHeight(qMin(w->width() - b, (int)(w->width() * 0.8)));
        }
    }

	setAttribute(Qt::WA_InputMethodEnabled, false);

    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::SingleSelection);
    
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setFrameStyle(QFrame::NoFrame);

    d->_eventTimer = startTimer(1000/30);

    setFocusPolicy(Qt::WheelFocus);
}

BtMiniView::~BtMiniView()
{
    Q_D(BtMiniView);
    delete d;
}

void BtMiniView::mousePressEvent(QMouseEvent *e)
{
    Q_D(BtMiniView);

    if(e->button() != Qt::LeftButton)
        return;

    d->_mouseLast = d->_mouseStart = e->pos();

    d->_mouseLeaveZone = false;
    d->_mouseDown      = true;

    d->_mouseTapping             = 0;
    d->_mouseScrolling[0].first  = e->pos();
    d->_mouseScrolling[0].second = d->_timer.elapsed();

    // calculate snapping
    QMargins m(d->subViewMargins(d->_currentSubView));
    d->_snappingLeft  = qMin(0, m.left());
    d->_snappingRight = qMin(0, m.right());

    // activate subview that is currently pressed
	bool activated = false;
    for(int i = 0; i < d->_subViews.size(); ++i)
    {
        if(d->_currentSubView != i && d->_subViews[i]->contentsRect().left() < e->x() + d->_vx &&
            d->_subViews[i]->contentsRect().right() > e->x() + d->_vx)
        {
            activateSubView(i);

			if(!activated)
				activated = true;
			else
				Q_ASSERT(false);
        }
    }

    QPersistentModelIndex index = indexAt(e->pos());

    if(index.isValid())
    {
        d->currentSubView()->updateModelIndex(index);
        emit currentChanged(index);
    }
}

void BtMiniView::mouseReleaseEvent(QMouseEvent *e)
{
    Q_D(BtMiniView);

    if(e->button() != Qt::LeftButton || !d->_mouseDown)
        return;

    d->_mouseScrolling[1].second = d->_timer.elapsed();
    d->_mouseScrolling[1].first  = e->pos();

    if(d->_mouseScrolling[1].second-d->_mouseScrolling[0].second > 200)
    {
        d->_mouseScrolling[0].first  = d->_mouseScrolling[1].first+((d->_mouseScrolling[0].first-\
            d->_mouseScrolling[1].first) * (200.0f/(d->_mouseScrolling[1].second-d->_mouseScrolling[0].second)));
        d->_mouseScrolling[0].second = d->_mouseScrolling[1].second - 200;
    }

    QPointF speed(QPointF(d->_mouseScrolling[1].first - d->_mouseScrolling[0].first) /
        qMax((d->_mouseScrolling[1].second - d->_mouseScrolling[0].second), (qint64)120));

    d->_mousePower.rx() *= qMin(qAbs(speed.x()) * 200.0f / d->_sizeFactor, (qreal)1.0);
    d->_mousePower.ry() *= qMin(qAbs(speed.y()) * 200.0f / d->_sizeFactor, (qreal)1.0);
    d->_mousePower += speed * 50.0f;

	int tapping = d->_mouseTapping;

	// cut down horizontal kinetic power, for a while
	d->_mousePower.rx() = 0.0;

	d->_mouseTapping = 0;
	d->_mouseDown = false;

    if(!d->_mouseLeaveZone)
    {
        QPersistentModelIndex index = indexAt(e->pos());

        if(selectionModel())
            selectionModel()->select(index, selectionCommand(index, e));

        if(tapping >= LONG_PRESS_DELAY)
        {
            emit longPressed(index);
        }
        else if(tapping >= SHORT_PRESS_DELAY)
        {
            emit shortPressed(index);
        }
        else
        {
            emit clicked(index);

			if(index.isValid())
			{
				if(model()->hasChildren(index))
				{
					Q_ASSERT(!d->_ld->plainMode() &&
						"Model for plain layout must not have children");

					// switch to left subview with children items
					makeSubView(d->_currentSubView + 1, index.child(0, 0));
					activateSubView(d->_currentSubView + 1);
				}
				else if(d->_interactive)
				{
					close();
				}
			}
        }
	}
    else
    {
        // switch subview if passed horizontal snapping
        const int xr = d->_mouseStart.x() - e->pos().x() + d->_snappingRight;
        const int xl = e->pos().x() - d->_mouseStart.x() + d->_snappingLeft;

        if(xr > d->_snappingValue)
            slideRight();
        if(xl > d->_snappingValue)
            slideLeft();
    }

    viewport()->update();
}

void BtMiniView::mouseMoveEvent(QMouseEvent *e)
{
    Q_D(BtMiniView);

    if(!d->_mouseDown)
        return;

    // Tapping / clicking
    if(qAbs(d->_mouseStart.x() - e->x()) > d->_sizeFactor ||
		qAbs(d->_mouseStart.y() - e->y()) > d->_sizeFactor)
    {
        d->_mouseLeaveZone = true;
        d->_mouseTapping   = 0;
		
		d->currentSubView()->_viualCenter = 0.45f;
    }

    // Kinetic scrolling
    d->_mouseScrolling[1].second = d->_timer.elapsed();
    d->_mouseScrolling[1].first  = e->pos();

    if(d->_mouseScrolling[1].second-d->_mouseScrolling[0].second > 200)
    {
        d->_mouseScrolling[0].first  = d->_mouseScrolling[1].first + ((d->_mouseScrolling[0].first -
            d->_mouseScrolling[1].first) * (200.0f / (d->_mouseScrolling[1].second - d->_mouseScrolling[0].second)));
        d->_mouseScrolling[0].second = d->_mouseScrolling[1].second - 200;
    }

    // Make snapping feeling
    QPoint p = e->pos();
    const int xr = d->_mouseStart.x() - p.x() + d->_snappingRight;
    const int xl = p.x() - d->_mouseStart.x() + d->_snappingLeft;

    if(xr > 0 && xr <= d->_snappingValue)
        p.rx() += xr;
    if(xl > 0 && xl <= d->_snappingValue)
        p.rx() -= xl;

    QPoint delta = p - d->_mouseLast;

    scroll(static_cast<float>(delta.x()), static_cast<float>(delta.y()));

    d->_mouseLast = p;
}


void BtMiniView::timerEvent(QTimerEvent *e)
{
    Q_D(BtMiniView);

    if(d->_eventTimer != e->timerId() || d->_currentSubView >= d->_subViews.size())
	{
		QAbstractItemView::timerEvent(e);
        return;
	}

	if(d->_sleep)
		return;

    // update kinetic scrolling
    if(!d->_mouseDown && (qAbs(d->_mousePower.x()) > 0.1f || qAbs(d->_mousePower.y()) > 0.1f))
    {
        d->_mousePower *= SCROLL_ATTENUATION;
        scroll(d->_mousePower.x(), d->_mousePower.y());
    }

    // update long press
    if(d->_mouseDown && !d->_mouseLeaveZone)
    {
        // if there is any connection to longPressed or shortPressed signal, vibrate
        if(d->_mouseTapping == LONG_PRESS_DELAY)
        {
            if(receivers(SIGNAL(longPressed(const QModelIndex &))) > 0)
            {
#ifdef Q_OS_WINCE
                __leds(true, 1), Sleep(30), __leds(false, 1);
#endif
            }
        }
        else if(d->_mouseTapping == SHORT_PRESS_DELAY)
        {
            if(receivers(SIGNAL(shortPressed(const QModelIndex &))) > 0)
            {
#ifdef Q_OS_WINCE
                __leds(true, 1), Sleep(30), __leds(false, 1);
#endif
            }
        }

        d->_mouseTapping++;
        viewport()->update();
    }

    bool calmly = false;
        
    // snap to borders of active subview
    if(!d->_mouseDown)
    {
        const QMargins m(d->subViewMargins(d->_currentSubView));

        if(m.top() > 0)
        {
            d->_mousePower.setY(qMin(d->_mousePower.y() * 0.75f, d->_mousePower.y()));

            scroll(0.0f, -m.top() * SCROLL_SNAPPING_SPEED);
        }
        else if(m.top() < 0 && m.bottom() > 0)
        {
            if(d->currentSubView()->contentsRect().height() < rect().height())
                scroll(0.0f, -m.top() * SCROLL_SNAPPING_SPEED);
            else
            {
                d->_mousePower.setY(qMax(d->_mousePower.y() * 0.75f, d->_mousePower.y()));

                scroll(0.0f, m.bottom() * SCROLL_SNAPPING_SPEED);
            }
        }
        
        if(d->currentSubView()->contentsRect().width() < rect().width())
        {
            if (m.left() >= 0 && d->_currentSubView == 0)
            {
                if(m.left() > 0)
                    scroll(-m.left() * SCROLL_SNAPPING_SPEED, 0.0f);
                else
                    calmly = true;
            }
            else if(m.right() > 0 && d->_currentSubView == d->_subViews.size() - 1)
                scroll(m.right() * SCROLL_SNAPPING_SPEED, 0.0f);
            else if(m.left() < 0)
                scroll(-m.left() * SCROLL_SNAPPING_SPEED, 0.0f);
            else if(m.right() < 0)
                scroll(m.right() * SCROLL_SNAPPING_SPEED, 0.0f);
            else
                calmly = true;
        }
        else
        {
            if(m.right() > 0)
                scroll(m.right() * SCROLL_SNAPPING_SPEED, 0.0f);
            else if(m.left() > 0)
                scroll(-m.left() * SCROLL_SNAPPING_SPEED, 0.0f);
            else
                calmly = true;
        }
    }

    if(calmly)
        d->updateSubView(d->_currentSubView);
}

void BtMiniView::scroll(float horizontal, float vertical)
{
    Q_D(BtMiniView);

    const int oy = d->currentSubView()->contentsRect().top();
    const int ox = d->_vx;

    d->_vx -= horizontal;
    d->currentSubView()->setContentsTop(d->currentSubView()->contentsRect().top() + vertical);
    
    if(oy != (int)d->currentSubView()->contentsRect().top() || ox != (int)d->_vx)
    {
        d->updateScrollBars();
        viewport()->update();
    }
}

void BtMiniView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    Q_D(BtMiniView);
    
    d->activateIndex(index, false, hint);

    scheduleDelayedItemsLayout();

	if(index.isValid())
		emit currentChanged(index);
}

void BtMiniView::scrollTo(QVariant data)
{
    Q_D(BtMiniView);

    QModelIndexList list = model()->match(model()->index(0, 0, d->currentSubView()->modelParentIndex()),
        d->_ld->levelOption(d->_currentSubView).searchRole, data);

    if(list.size() == 1)
        scrollTo(list[0]);
}

QModelIndex BtMiniView::indexAt(const QPoint &point) const
{
    Q_D(const BtMiniView);

    // Parse all subviews
    foreach(BtMiniSubView *v, d->_subViews)
    {
        const QPoint gp(point + QPoint(d->_vx, 0));
        if(v->contentsRect().contains(gp))
			return v->indexAt(gp - v->contentsRect().topLeft().toPoint());
    }

    return QModelIndex();
}

QModelIndex BtMiniView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    return QModelIndex();
}

int BtMiniView::horizontalOffset() const
{
    return 0;
}

int BtMiniView::verticalOffset() const
{
    return 0;
}

bool BtMiniView::isIndexHidden(const QModelIndex &index) const
{
    return false;
}

void BtMiniView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_D(BtMiniView);

    foreach(BtMiniSubView *v, d->_subViews)
    {
		// module changed
        if(v->modelParentIndex() == topLeft)
        {
			// stop threads and clear affected indexes
			//foreach(BtMiniViewPrivate::BtMiniViewThread *t, d->_threads)
			//	t->stop();

			//foreach(BtMiniViewPrivate::BtMiniViewThread *t, d->_threads)
			//	foreach(const QModelIndex &i, t->_done.keys())
			//		if(i.parent() == v->modelParentIndex())
			//		{
			//			qDebug() << "Erased on module change" << i.data(BtMini::ModuleRole) << i.data(BtMini::PlaceRole);
			//			t->_done.take(i);
			//		}

			//for(int i = 0; i < d->_toCalculate.size(); ++i)
			//	if(d->_toCalculate[i].parent() == v->modelParentIndex())
			//		d->_toCalculate.erase(d->_toCalculate.begin() + i--);

			// reset subview
            v->clear();

			QModelIndex i = topLeft.child(qMin(model()->rowCount(topLeft) - 1, 
				v->modelIndex().row()), 0);

            v->setModelIndex(i);

			if(d->currentSubView() == v)
				emit currentChanged(i);

			scheduleDelayedItemsLayout();

            return;
        }
    }

    Q_ASSERT(false);
}

void BtMiniView::doItemsLayout()
{
	Q_D(BtMiniView);

	//qDebug() << "doItemsLayout";

    // need to stop delayed layout first, so code below can schedule layout again
    QAbstractItemView::doItemsLayout();

	// if there is no model, it is not time to create subviews...
	if(!model()) return;

    // create views if them not created yet
	d->activateIndex(QModelIndex(), true);

    // resize new subviews
    foreach(BtMiniSubView *v, d->_subViews)
        if(v->baseSize().isEmpty())
            v->setBaseSize(viewport()->size());

    // check if size is not equal to viewport size
    if(d->currentSubView()->baseSize() != viewport()->size())
        d->currentSubView()->setBaseSize(viewport()->size());

    d->updateViews();

	Q_ASSERT(d->_currentSubView < d->_subViews.size());
	
    // check if we need to update layout
	bool needLayout = false;
	bool layoutMaked = false;

	for(int s = 0; s < 2; ++s)
	{
		for(int i = 0; i < d->_subViews.size(); ++i)
		{
			if(s == 0 && i != d->_currentSubView)
				continue;

			BtMiniSubView *v = d->_subViews[i];
			const BtMiniLayoutOption &o = d->_ld->levelOption(i);

			if(v->_items.size() == 0)
			{
				if(!layoutMaked)
				{
					if(o.perCycle == 0)
					{
						d->layoutItemsFull(i);

						if(v->contentsRect().height() > v->baseSize().height())
						{
							QRect r(v->indexRect(v->modelIndex()));
							v->setContentsTop(-r.top());

							if(v->contentsRect().bottom() + 1 < v->baseSize().height())
								v->setContentsTop(v->contentsRect().top() + v->baseSize().height() -
									v->contentsRect().bottom() + 1);
						}
					}
					else
					{
						d->layoutItems(i, 1);
					}

					layoutMaked = true;
				}
				else
					needLayout = true;
			}
		}
	}

	if(d->_needScroll)
	{
		BtMiniSubView *v = d->_subViews[d->_currentSubView];

		d->checkIndexCreated(d->_currentSubView, v->modelIndex());

		if(v->_items.size() > 1)
		{
			QRect r(v->indexRect(v->modelIndex()));

			Q_ASSERT(r.width() > 0);

			int l = v->contentsRect().top() + r.top();

			switch(d->_scrollHint)
			{
			case QAbstractItemView::EnsureVisible:
				{
					const int middle = l + (r.height() / 2);
					if(middle > v->baseSize().height())
						l -= qMax(v->baseSize().height() - r.height(), 0);
					else if(middle > 0)
						l = 0;
				}
				break;
				//case QAbstractItemView::EnsureVisible:
				//    l = v->contentsRect().top() + r.top() + (qMin(r.height(),
				//        v->baseSize().height()) / 2) - (v->baseSize().height() / 2);
				//    break;
			default:
				Q_ASSERT(false && "Not implemented.");
			}

			d->_mousePower.setY(-l / SCROLL_BACK_ATTENUATION);
			d->_needScroll = false;
		}
	}

	if(needLayout)
		scheduleDelayedItemsLayout();

	if(layoutMaked)
		d->updateScrollBars();

	if(d->_needEmitCurrentChanged)
	{
		d->_needEmitCurrentChanged = false;
		emit currentChanged(currentIndex());
	}
}

const QModelIndex BtMiniView::currentIndex() const
{
    Q_D(const BtMiniView);

    if(d->_subViews.size() == 0)
        return QAbstractItemView::currentIndex();
    else
        return d->currentSubView()->modelIndex();
}

const QModelIndexList BtMiniView::currentIndexes() const
{
    Q_D(const BtMiniView);

    QModelIndexList list;

    foreach(const BtMiniSubView* v, d->_subViews)
        list.append(v->modelIndex());

    return list;
}

int BtMiniView::currentLevel() const
{
    Q_D(const BtMiniView);
    return d->_currentSubView;
}

void BtMiniView::makeSubView(int id, const QModelIndex &index)
{
    Q_D(BtMiniView);

    //qDebug() << "BtMiniView::makeSubView" << id << index;

    Q_ASSERT(model());
    Q_ASSERT(id <= d->_subViews.size());
    
    if(id == d->_subViews.size())
        d->_subViews.append(0);
    
    if(!d->_subViews[id])
    {
        d->_subViews[id] = new BtMiniSubView;
        //d->_subViews[id]->setBaseSize(viewport()->size());
        
        if(id > 0)
            d->_subViews[id]->setContentsLeft(d->_subViews[id - 1]->contentsRect().right() + 1);
    }

    d->_subViews[id]->setModelIndex(index);

    if(!d->_ld->plainMode())
    {
        // clear all subviews after changed subview
        while(d->_subViews.size() > id + 1)
        {
            delete d->_subViews[id + 1];
            d->_subViews.erase(d->_subViews.begin() + id + 1);
        }
    }

	scheduleDelayedItemsLayout();
}

void BtMiniView::activateSubView(int id)
{
    Q_D(BtMiniView);

    Q_ASSERT(id < d->_subViews.size());

    if(d->_currentSubView != id)
    {
        // remove unnecessary items of previous view to release memory
        BtMiniSubView *v = d->currentSubView();
        if(layoutDelegate()->levelOption(d->_currentSubView).perCycle > 0)
        {
            while(v->_items.size() > 0 && v->contentsRect().bottom() -
                v->_items[v->_items.size() - 1]->height() > height())
                d->removeItem(d->_currentSubView, v->_items.size() - 1);
            while(v->_items.size() > 0 && v->contentsRect().top() + v->_items[0]->height() < 0)
                d->removeItem(d->_currentSubView, 0);
        }

		// free render cache
		if(d->_useRenderCaching)
			d->_cachedRect = QRect();

        d->_currentSubView = id;
        d->_mousePower     = QPointF();

        d->updateViews();
		scheduleDelayedItemsLayout();
        emit currentChanged(currentIndex());
    }
}

void BtMiniView::reset()
{
    Q_D(BtMiniView);

    //qDebug() << "BtMiniView::reset";
	
    d->clear();
    QAbstractItemView::reset();
}

void BtMiniView::setRootIndex(const QModelIndex &index)
{
    Q_D(BtMiniView);

    BtMiniLayoutDelegate *ld = model()->findChild<BtMiniLayoutDelegate*>();
    if(ld)
        setLayoutDelegate(ld);

    setVerticalScrollBarPolicy(d->_ld->levelOption().scrollBarPolicy);
    d->activateIndex(index, true);

	scheduleDelayedItemsLayout();
}

void BtMiniView::slideLeft()
{
    Q_D(BtMiniView);
    activateSubView(qMax(d->_currentSubView - 1, 0));
}

void BtMiniView::slideRight()
{
    Q_D(BtMiniView);
    activateSubView(qMin(d->_currentSubView + 1, d->_subViews.size() - 1));
}

void BtMiniView::setLevelOptions(const int level, int itemPerLine, QString itemPreText, QString itemPostText)
{
    Q_D(BtMiniView);

    for(int i = 0; i < d->_ld->levelOptionsCount(); ++i)
    {
        if(!(level == -1 || level == i))
            continue;

        BtMiniLayoutOption o(d->_ld->levelOption(i));

        o.perLine  = itemPerLine;
        o.preText  = itemPreText;
        o.postText = itemPostText;

        d->_ld->setLevelOption(i, o);
    }
}

void BtMiniView::setSearchRole(int searchRole, int level)
{
    Q_D(BtMiniView);

    for(int i = 0; i < d->_ld->levelOptionsCount(); ++i)
    {
        if(!(level == -1 || level == i))
            continue;

        BtMiniLayoutOption o(d->_ld->levelOption(i));

        o.searchRole = searchRole;

        d->_ld->setLevelOption(i, o);
    }
}

void BtMiniView::showEvent(QShowEvent *e)
{
    Q_D(BtMiniView);
    //qDebug() << "BtMiniView::showEvent" << rect();
    //d->updateViews();
}

void BtMiniView::setCurrentIndex(const QModelIndex &index)
{
    Q_D(BtMiniView);
    //qDebug() << "BtMiniView::setCurrentIndex" << index;
    Q_ASSERT(index.isValid());
    d->activateIndex(index, true);
}

void BtMiniView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    qDebug() << "BtMiniView::setSelection" << rect << command;
    selectionModel()->select(indexAt(rect.topLeft()), command);
}

void BtMiniView::paintEvent(QPaintEvent *e)
{
    Q_D(BtMiniView);

	// update according vertical scrollbar
	if(d->_vt != verticalScrollBar()->value())
	{
		if(d->_ld->levelOption(d->_currentSubView).scrollPerItem)
		{
			scrollTo(d->currentSubView()->modelIndex(verticalScrollBar()->value()));
		}
		else
		{
			scroll(0.0f, d->_vt - verticalScrollBar()->value());
			d->_vt = verticalScrollBar()->value();
			d->_mousePower.ry() = 0.0f;
		}
	}


    QPainter painter(viewport());

	const bool debugClipping = false;
	const bool debugCache = false;
    
	if(debugClipping || debugCache)
	{
		painter.scale(0.5, 0.5);
		painter.translate(100, 100);
	}

    // draw subviews
    foreach(BtMiniSubView *v, d->_subViews)
    {
        const QRect area(viewport()->rect() &
            QRect(v->contentsRect().toRect()).translated(-d->_vx, 0));

        if(area.isNull())
            continue;

		// subview-related clipping
        const QRect clip(QRect(QPoint(), v->contentsRect().size().toSize()) &
            viewport()->rect().translated(QPoint(d->_vx, 0) -
            v->contentsRect().topLeft().toPoint()));

        if(!clip.isNull())
		{
			if(d->_useRenderCaching && d->_subViews[d->_currentSubView] == v && 
				v->contentsRect().width() == d->_cachedSurface->width())
			{
				Q_CHECK_PTR(d->_cachedSurface);
				Q_ASSERT(d->_cachedSurface->size() == d->_cachedSurface->size().expandedTo(clip.size()));

				const int overlap = (d->_cachedSurface->height() * CACHED_SURFACE_OVERLAP) / 2;

				// renew
				if(d->_cachedRect == QRect() || !d->_cachedRect.intersects(clip))
				{
					d->_cachedRect = QRect(QPoint(0, qMax(0, clip.top() - overlap)), d->_cachedSurface->size());

					d->_cachedSurface->fill(this, QPoint());
					QPainter p(d->_cachedSurface);
					v->paint(&p, QRect(QPoint(), d->_cachedSurface->size()), d->_cachedRect);
					if(debugCache)
					{
						p.setPen(QPen(Qt::red, 2));
						p.drawRect(d->_cachedSurface->rect());
					}
				}

				// shift cache
				if(!d->_cachedRect.contains(clip))
				{
					int vd = clip.bottom() > d->_cachedRect.bottom() ? clip.bottom() - d->_cachedRect.bottom() + 
						(overlap * 0.75) : clip.top() - d->_cachedRect.top() - (overlap * 0.75);

					QRect rs, rd;

					if(vd > 0)
					{
						rs = d->_cachedRect;
						rs.setTop(rs.bottom());
						rs.setBottom(rs.bottom() + vd);
						rd = QRect(0, d->_cachedSurface->height() - vd, d->_cachedSurface->width(), d->_cachedSurface->height());
					}
					else
					{
						rs = d->_cachedRect;
						rs.setBottom(rs.top());
						rs.setTop(rs.top() + vd);
						rd = QRect(0, 0, d->_cachedSurface->width(), -vd);
					}

					d->_cachedSurface->scroll(0, -vd, d->_cachedSurface->rect());
					d->_cachedRect.translate(0, vd);

					QPainter p(d->_cachedSurface);
					v->paint(&p, rd, rs);

					if(debugCache)
					{
						p.setPen(QPen(Qt::yellow, 2));
						p.drawRect(rd);

						qDebug() << rs << rd << d->_cachedRect << vd;
					}
				}

				Q_ASSERT(area.size() == clip.size());
				Q_ASSERT(d->_cachedRect.contains(clip));

				// flush cache to window
				painter.drawPixmap(area, *d->_cachedSurface, clip.translated(-d->_cachedRect.topLeft()));

				if(debugCache)
					painter.drawPixmap(QPoint(d->_cachedSurface->size().width(), 0), *d->_cachedSurface);
			}
			else
	            v->paint(&painter, area, clip);
		}
    }

	if(debugClipping)
	{
		painter.setPen(QPen(Qt::green, 2));
		painter.drawRect(QRect(QPoint(), viewport()->size()));
	}
    
    // draw shadow
    if(d->_enableTopShadow)
    {
		QRect r(viewport()->rect().adjusted(0, 0, 0, d->_sizeFactor - viewport()->rect().height()));
            
        QLinearGradient g(r.topLeft(), r.bottomLeft());
        
        g.setColorAt(0.0, QColor(0, 0, 0, 100));
        g.setColorAt(1.0, QColor(0, 0, 0, 0));
        
        painter.setPen(Qt::NoPen);
        painter.setBrush(g);
        painter.drawRect(r);
    }

    // draw tapping
    if(d->_mouseDown && d->_mouseTapping > 0 && d->_mouseTapping <= LONG_PRESS_DELAY * 1.4)
    {
        QRect z(d->_mouseLast, QSize());
        const int m = d->_sizeFactor * 2;
        z.adjust(-m, -m, m, m);

        painter.setPen(Qt::NoPen);

		int mt = d->_mouseTapping - 6;
        
        for(int i = qMax(0, mt - 3), e = mt; i <= e; ++i)
        {
            QPoint p(z.center() + (QPointF(qCos(i / 1.7), qSin(i / 1.7)) * (z.width() / 2)).toPoint());
            QRect r(p.x()-(m*0.25), p.y()-(m*0.25), m*0.5, m*0.5);

            painter.setBrush(QColor(0, 0, 0, (255 * 0.8) / (mt - i + 1)));
            painter.drawEllipse(r);
        }
    }
}

void BtMiniView::resizeEvent(QResizeEvent *e)
{
    Q_D(BtMiniView);

    //qDebug() << "BtMiniView::resizeEvent" << e->oldSize() << e->size();

    d->_snappingValue = qMin(e->size().width(), e->size().height()) / 4;

    verticalScrollBar()->setPageStep(e->size().height());
    verticalScrollBar()->setSingleStep(e->size().height() / 5);

    // update layout limits
    d->_limitHeightTop = e->size().height() * 1.5;
    d->_limitHeightBottom = e->size().height() * 2.5;

	scheduleDelayedItemsLayout();

	if(d->_useRenderCaching)
	{
		setRenderCaching(false);
		setRenderCaching(true);
	}

    QAbstractItemView::resizeEvent(e);
}

void BtMiniView::setLayoutDelegate(BtMiniLayoutDelegate *ld)
{
    Q_D(BtMiniView);
    d->_ld = ld == 0 ? findChild<BtMiniLayoutDelegate *>() : ld;
    Q_CHECK_PTR(d->_ld);
}

void BtMiniView::setInteractive(bool mode)
{
    Q_D(BtMiniView);

    d->_interactive = mode;
}

bool BtMiniView::viewportEvent(QEvent *e)
{
    switch(e->type())
    {
    case QEvent::ToolTip:
    case QEvent::QueryWhatsThis:
    case QEvent::WhatsThis:
        return true;
    }
    return QAbstractItemView::viewportEvent(e);
}

QRect BtMiniView::visualRect(const QModelIndex &index) const
{
    Q_D(const BtMiniView);

    //qDebug() << "BtMiniView::visualRect" << index;

    //if(d->_currentSubView < d->_subViews.size())
    //    return d->currentSubView()->indexRect(index).translated(0, 
    //        d->currentSubView()->contentsRect().top());

    return QRect();
}

QRegion BtMiniView::visualRegionForSelection(const QItemSelection &selection) const
{
    qDebug() << "BtMiniView::visualRegionForSelection" << selection;
    return QRegion();
}

void BtMiniView::setTopShadowEnabled(bool mode)
{
    Q_D(BtMiniView);
    d->_enableTopShadow = mode;
}

void BtMiniView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_D(BtMiniView);

    qDebug() << "BtMiniView::rowsAboutToBeRemoved" << parent << start << end;

    Q_ASSERT(start == end);
    Q_ASSERT(d->_ld->plainMode());

    if(!parent.isValid() && d->_ld->plainMode())
    {
        delete d->_subViews[start];
        d->_subViews.erase(d->_subViews.begin() + start);

		Q_ASSERT(start == d->_currentSubView);

        if(d->_currentSubView > 0)
		    d->_currentSubView--;

		scheduleDelayedItemsLayout();

		d->_needEmitCurrentChanged = true;
    }
}

void BtMiniView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_D(BtMiniView);

    qDebug() << "BtMiniView::rowsInserted" << parent << start << end;

    Q_ASSERT(start == end);
	Q_ASSERT(d->_ld->plainMode());

    if(!parent.isValid())
    {
		d->_subViews.insert(start, 0);

        QModelIndex p = model()->index(start, 0);
        QModelIndex i;

        if(model()->rowCount(p) > 0)
            i = model()->index(0, 0, p);
        
		makeSubView(start, i);

		if(start == d->_currentSubView + 1)
        {
			slideRight();
        }
		else if(start == d->_currentSubView)
        {
            d->_currentSubView++;
            d->updateViews();
			slideLeft();
        }
		else
		{
            Q_ASSERT(false);

			d->updateViews();
			emit currentChanged(currentIndex());
		}
    }
}

QString BtMiniView::currentContents() const
{
    Q_D(const BtMiniView);

    foreach(BtMiniSubView *v, d->_subViews)
    {
        const QPoint gp(d->_mouseLast + QPoint(d->_vx, 0));
        if(v->contentsRect().contains(gp))
        {
            const QPoint vp(gp - v->contentsRect().topLeft().toPoint());
            int i = v->xyItem(vp);

            if(i >= 0)
            {
                const QPoint pp(vp - v->itemXy(i));

#ifdef USE_WEBKIT
                QWebPage *wp(v->_items[i]->_wp);
                if(wp)
                {
                    //QWebHitTestResult r(wp->mainFrame()->hitTestContent(pp));

                    //qDebug() << r.alternateText() << r.element().attributeNames() << r.linkText();
                    //qDebug() << r.element().classes() << r.element().toPlainText();
                    //qDebug() << r.isNull() /*<< r.element().toInnerXml() << r.element().toOuterXml()*/;
                    //qDebug() << r.linkElement().attributeNames() << r.linkElement().classes() << r.linkElement().toPlainText();
                    //qDebug() << r.element().parent().classes() << r.element().parent().toPlainText();

                    //qDebug() << r.title() << r.linkText() << r.alternateText();

                    //for(QWebElement e = r.element(); !e.isNull(); e = e.parent())
                    //    qDebug() << e.tagName() << e.classes() << e.attributeNames() << e.toPlainText();


                    //QVariant v = wp->mainFrame()->evaluateJavaScript(QString("document.elementFromPoint(%1, %2);").arg(pp.x()).arg(pp.y()));
                    //qDebug() << v.toMap().keys();

                    QList<QWebElement> el;
                    el << wp->mainFrame()->documentElement();
                    for(int i = 0; i < el.size(); ++i)
                    {
                        QWebElement e = el[i];
                        
                        //qDebug() << e.tagName() << e.geometry() << e.classes() << e.attributeNames() << e.toPlainText();
                        
                        for(QWebElement c = e.firstChild(); !c.isNull(); c = c.nextSibling())
                            if(c.geometry().contains(pp) && c.geometry().height() <= e.geometry().height())
                                el += c;
                    }

                    return el[el.size() - 1].toOuterXml();
                }
#endif
				int sp;
				QString st;
				QString dt;

                if(v->_items[i]->_doc)
                {
                    sp = v->_items[i]->_doc->documentLayout()->hitTest(pp, Qt::ExactHit);
                    if(sp < 0)
                        return QString();
                    st = v->_items[i]->_doc->toPlainText();
				}
				
				if(v->_items[i]->_st)
				{
					sp = v->_items[i]->_st->hitTest(pp);
					if(sp < 0)
						return QString();
					st = v->_items[i]->_st->plainText();
					dt = v->_items[i]->_st->text();
				}
				
				if(!st.isEmpty())
				{
                    QChar c = st[sp];

                    if(c.isSpace())
                        return QString();

                    // find correspondence between strings using character occurrences count
                    int sc = 0;
                    int si = 0;

                    for(int ii = 0; ii < st.size(); ++ii)
                    {
                        if(st[ii] == c)
                        {
                            if(ii == sp)
                                si = sc;
                            ++sc;
                        }
                    }

					// qstatictext yet have model text
					// simultaneous call of sword model data() may cause data corruption
					if(dt.isEmpty())
						dt = d->currentSubView()->modelIndex(v->_items[i]->_row).data().toString();

                    // erase css style sheet in text
                    dt = dt.left(dt.indexOf("<head>", Qt::CaseInsensitive)) +
                        dt.mid(dt.indexOf("</head>", Qt::CaseInsensitive) + 7);

					// remove highlighting
					int hsp = dt.indexOf("<span style=\"background-color:#FFFF66;\">");
					if(hsp >= 0)
						dt = dt.remove(dt.indexOf("</span>", hsp), 7).remove(hsp, 40);
                    
                    int dc = 0;
                    int dp = 0;
                    bool skip = false;

                    for(int ii = 0; ii < dt.size(); ++ii)
                    {
                        if(dt[ii] == '<')
                            skip = true;
                        else if(dt[ii] == '>')
                            skip = false;
                        else if(!skip && dt[ii] == c)
                        {
                            if(si == dc)
                                dp = ii;
                            ++dc;
                        }
                    }

					//if(dt[dp] != st[sp])
                    if(dc != sc)
					{
						qDebug() << "Can't find correspondence " << st << dt << c << sc << dc;
                        Q_ASSERT(false);
                    }

                    int from = dt.lastIndexOf('<', dp);

                    // first tag must not be closing tag
                    if(dt[from + 1] != '/')
                        return dt.mid(from, dt.indexOf('>', dp) - from + 1);
                }
            }
        }
    }

    return QString();
}

BtMiniLayoutDelegate * BtMiniView::layoutDelegate()
{
    Q_D(BtMiniView);
    return d->_ld;
}

const BtMiniLayoutDelegate * BtMiniView::layoutDelegate() const
{
    Q_D(const BtMiniView);
    return d->_ld;
}

void BtMiniView::setRenderCaching(bool mode)
{
	Q_D(BtMiniView);

	d->_useRenderCaching = mode;

	if(mode)
	{
		d->_cachedSurface = new QPixmap(QSize(size().width(), size().height() * (1.0 + CACHED_SURFACE_OVERLAP)));
		d->_cachedRect = QRect();
	}
	else 
	{
		if(d->_cachedSurface)
			delete d->_cachedSurface;
	}
}

void BtMiniView::keyPressEvent( QKeyEvent *e )
{
	Q_D(BtMiniView);

#ifdef QT_DEBUG
	switch(e->key())
	{
	case Qt::Key_C:
		setRenderCaching(!d->_useRenderCaching);
		viewport()->update();
		break;
	}
#endif
}

void BtMiniView::setSleep(bool sleep)
{
	Q_D(BtMiniView);

	d->_sleep = sleep;

	if(sleep)
	{
		d->_mousePower = QPointF();

		foreach(BtMiniViewPrivate::BtMiniViewThread *t, d->_threads)
			t->_stop = true;
	}
}