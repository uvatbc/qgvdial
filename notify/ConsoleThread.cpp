#include "ConsoleThread.h"
#include "stdio.h"

ConsoleThread::ConsoleThread(QObject *parent)
: QThread(parent)
{
}//ConsoleThread::ConsoleThread

void
ConsoleThread::run()
{
    int c;
    int sequence = 0;

    Q_DEBUG("Starting keyboard loop");

    while (1) {
        c = getchar();

        if        ((c == 'q') && (sequence == 0)) {
            sequence++;
        } else if ((c == 'u') && (sequence == 1)) {
            sequence++;
        } else if ((c == 'i') && (sequence == 2)) {
            sequence++;
        } else if ((c == 't') && (sequence == 3)) {
            sequence++;
        } else if (c == '\n') {
            continue;
        } else {
            sequence = 0;
        }

        Q_DEBUG(QString("key pressed : %1").arg (QChar(c)));

        if (sequence == 4) {
            break;
        }
    }

    Q_DEBUG("Quitting keyboard loop");
}//ConsoleThread::run
