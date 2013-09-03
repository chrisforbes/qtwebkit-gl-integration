
#include <QtCore>
#include <QtWebKit>

class EventGlue : public QObject
{
    Q_OBJECT

public:
    EventGlue(QWebPage *page)
        : page(page)
    {
        connect(this->page, SIGNAL(loadFinished(bool)), this, SLOT(finished(bool)));
		connect(this->page, SIGNAL(repaintRequested(QRect const &)), this, SLOT(repaint(QRect const &)));
    }

    virtual ~EventGlue() {}

private slots:
    void finished(bool ok);
	void repaint(QRect const& dirty);

private:
    QWebPage *page;
};
