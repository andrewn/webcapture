webcapture
===

This is a clone of the webcapture example from the Qt GraphicsDojo hosted in gitorious:
  http://qt.gitorious.org/qt-labs/graphics-dojo

Original blog post:
  http://labs.trolltech.com/blogs/2009/01/15/capturing-web-pages/

I've modified it to accept more command line arguments using optparse by W. Evan Sheehan

Dependencies
---

You need Qt to be installed 

Build
---

### On Mac OS X:

1. $> qmake
2. Double click the Xcode project created
3. Click on build in Xcode
4. the 'webcapture' executable is created

### On Unix (specfically Centos):

NB: This requires fontconfig 1.4.2 see here for an explanation of why:
 http://theitdepartment.wordpress.com/2009/03/15/centos-qt-fcfreetypequeryface/

1. $> qmake
2. $> make
3. The executable will be built

You need to have an X11 display available. I run this headless and so have Xvfb running on display :1 ($> Xvfb :1 &). This means prefixing the command with:
  $> export DISPLAY=:0; ./webcapture http://news.bbc.co.uk

