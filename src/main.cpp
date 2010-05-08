#ifdef QTOPIA

#include <qtopiaapplication.h>
#include "phonetiq.h"

QTOPIA_ADD_APPLICATION(QTOPIA_TARGET, PhonetiqMainWindow)
QTOPIA_MAIN

#else // QTOPIA

#include <QtGui/QApplication>
#include "phonetiq.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PhonetiqMainWindow w;
    w.show();
    return a.exec();
}

#endif // QTOPIA
