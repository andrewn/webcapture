/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Graphics Dojo project on Qt Labs.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 or 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <iostream>

#include <QtGui>
#include <QtWebKit>

#include "optparse.cpp"

class WebCapture : public QObject
{
    Q_OBJECT

public:
    QImage image;
    WebCapture();
	// Single method to load the web page and output to filesystem at a set scale, size
    void load(const QUrl &url, int zoom, int scale, const QString &outputFileName, int width, int height, int pause);

signals:
    void finished();

private slots:
	// Slots defined by Qt
    void showProgress(int percent);
    void saveResult(bool ok);
	void saveImage();

private:
    QWebPage m_page;
    QString m_fileName;
    int m_zoom;
    int m_percent;
    int m_scale;
    int m_height;
    int m_width;
    int m_pause;
	bool m_ok;
};

WebCapture::WebCapture(): QObject(), m_zoom(100), m_percent(0)
{
    connect(&m_page, SIGNAL(loadProgress(int)), this, SLOT(showProgress(int)));
    connect(&m_page, SIGNAL(loadFinished(bool)), this, SLOT(saveResult(bool)));
}

/**
 * load the web page and output to filesystem at a set scale, size 
 * url   - the url to load
 * zoom  - use WebKit's page zoom 
 * scale - scale the resulting image
 * outputFileName - the filepath to save to including extension
 * width  - the width of the output image
 * height - the height of the output image
 */
void WebCapture::load(const QUrl &url, int zoom, int scale, const QString &outputFileName, int width, int height, int pause)
{
    std::cout << "Loading " << qPrintable(url.toString()) << std::endl;
	
	// Store these values in the class instance
	// as they're used later
	m_ok = false;
	
    m_zoom    = zoom;
    m_scale   = scale;
    m_percent = 0;
    
    m_height  = height;
    m_width   = width;
    
    m_pause   = pause * 1000;
    
    m_fileName = outputFileName;
	
	// Create the image
    image = QImage();
	
	// Load the page
    m_page.mainFrame()->load(url);
    m_page.mainFrame()->setScrollBarPolicy( Qt::Vertical, Qt::ScrollBarAlwaysOff );
    m_page.mainFrame()->setScrollBarPolicy( Qt::Horizontal, Qt::ScrollBarAlwaysOff );
    
	// Set WebKit's zoo factor
	qreal zoom_factor = static_cast<qreal>( m_zoom ) / 100.0;
    m_page.mainFrame()->setZoomFactor( zoom_factor );
    
	// Calculate the scale factor from the percentage value
    qreal scale_factor = static_cast<qreal>(m_scale) / 100.0;
	
    m_page.setViewportSize( QSize( width, 3 * width / 4 ) / scale_factor );
}

// A progress indicator
void WebCapture::showProgress(int percent)
{
    if (m_percent >= percent)
        return;

    while (m_percent++ < percent)
        std::cout << "#" << std::flush;
}

void WebCapture::saveResult(bool ok) 
{
	m_ok = ok;
	
	// If m_pause != 0 
	if (m_pause == 0) {
		saveImage();
	} else {
		QTimer::singleShot( m_pause, this, SLOT(saveImage()));
	}
}

void WebCapture::saveImage()
{
    std::cout << std::endl;

    // crude error-checking
    if (!m_ok) {
        std::cerr << "Failed loading " << qPrintable(m_page.mainFrame()->url().toString()) << std::endl;
        emit finished();
        return;
    }
    
    qreal scale_factor = static_cast<qreal>(m_scale) / 100.0;

    // create the image for the buffer
    QSize size = m_page.mainFrame()->contentsSize();
    
    if ( m_height != 0 ) {
      size.setHeight( m_height );
    }
    
    if ( m_width != 0 ) {
      size.setWidth( m_width );
    }
    
    image = QImage(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    // render the web page
    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    m_page.setViewportSize( size );
    
    m_page.mainFrame()->render(&p);
    p.end();

    // Scale the image. By default, the scale_factor will be 1 indicating no scaling.
    image = image.scaled(size * scale_factor, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	// Save out the image 
    image.save(m_fileName);
	
	// Done!
    emit finished();
}

// shamelessly copied from Qt Demo Browser
static QUrl guessUrlFromString(const QString &string)
{
    QString urlStr = string.trimmed();
    QRegExp test(QLatin1String("^[a-zA-Z]+\\:.*"));

    // Check if it looks like a qualified URL. Try parsing it and see.
    bool hasSchema = test.exactMatch(urlStr);
    if (hasSchema) {
        QUrl url(urlStr, QUrl::TolerantMode);
        if (url.isValid())
            return url;
    }

    // Might be a file.
    if (QFile::exists(urlStr))
        return QUrl::fromLocalFile(urlStr);

    // Might be a shorturl - try to detect the schema.
    if (!hasSchema) {
        int dotIndex = urlStr.indexOf(QLatin1Char('.'));
        if (dotIndex != -1) {
            QString prefix = urlStr.left(dotIndex).toLower();
            QString schema = (prefix == QLatin1String("ftp")) ? prefix : QLatin1String("http");
            QUrl url(schema + QLatin1String("://") + urlStr, QUrl::TolerantMode);
            if (url.isValid())
                return url;
        }
    }

    // Fall back to QUrl's own tolerant parser.
    return QUrl(string, QUrl::TolerantMode);
}

#include "webcapture.moc"

// This provides the command line options

int main(int argc, char * argv[])
{

  OptionParser opts;
  opts	= OptionParser("webcapture 1.0, Captures images of web pages.\nUsage: webcapture [OPTIONS] URL\n");
  
  //#define NULL ((char *)0)
  
  opts.add_option("-z", "--zoom-factor", "zoom", "the percentage factor to zoom the page by", "100");
  opts.add_option("-s", "--scale-factor", "scale", "the percentage factor to scale the final image by", "100");
  opts.add_option("-o", "--output", "output", "the output filepath to save to (MUST have an extension e.g. .png)", "image.png");
  opts.add_option("-w", "--width", "width", "width of the viewport", "0");
  opts.add_option("-h", "--height", "height", "height of the viewport", "0");
  opts.add_option("-p", "--pause", "pause", "pause for X seconds before taking the screenshot (allows javascript etc to kick in)", "0");
  
  
  opts.parse_args( argc, argv );
  
  // All options will be in here
  map<std::string, std::string> options;
  options = opts.options;
  
  // All urls should be in here
  vector<std::string> arguments;
  arguments = opts.arguments;
  
/*
  puts( "Debug\n\n" );
  std::cout << options["zoom"] << std::endl ;
  std::cout << options["output"] << std::endl;
  std::cout << options["width"] << std::endl ;
  std::cout << options["height"] << std::endl ;
*/
    
  if ( arguments.size() == 0 ) {
    opts.help();
  } else {
    
    QUrl url          = guessUrlFromString( QString::fromLatin1( arguments[0].c_str() ) );
    int zoom          = qMax( 10, QString::fromLatin1( options["zoom"].c_str() ).toInt() );
    int scale         = qMax( 10, QString::fromLatin1( options["scale"].c_str() ).toInt() );
    QString fileName  = QString::fromLatin1( options["output"].c_str() );
    int width         = QString::fromLatin1( options["width"].c_str() ).toInt();
    int height        = QString::fromLatin1( options["height"].c_str() ).toInt();
    int pause         = QString::fromLatin1( options["pause"].c_str() ).toInt();
    
    QApplication a(argc, argv);
    WebCapture capture;
    QObject::connect(&capture, SIGNAL(finished()), QApplication::instance(), SLOT(quit()));
    capture.load(url, zoom, scale, fileName, width, height, pause);

    return a.exec();
  }
  /*
    QUrl url = guessUrlFromString(QString::fromLatin1(argv[1]));
    int zoom = qMax(10, QString::fromLatin1(argv[2]).toInt());
    QString fileName = QString::fromLatin1(argv[3]);
    int width = (argc == 5) ? QString::fromLatin1(argv[4]).toInt() : 1024;

    QApplication a(argc, argv);
    WebCapture capture;
    QObject::connect(&capture, SIGNAL(finished()), QApplication::instance(), SLOT(quit()));
    capture.load(url, zoom, fileName, width);

    return a.exec();
    */
}

