#include "UniqueAppHelper.h"
#include <iostream>
using namespace std;

#if defined(Q_OS_UNIX)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#else
#error Unsupported platform
#endif

enum UniqueSignals {
    Clean = 0,
    Owned,
    WakeUp,
};

UniqueAppHelper::UniqueAppHelper ()
: fUnique (this)
, bOwner (false)
{
    // Create the name that we will use
    QString strName = QDir::tempPath ()
                    + QDir::separator ()
                    + "qgvdial_unique";

    QFileInfo info(strName);
    bool bSetOwner = !info.exists ();
    if (0 == info.size ())
    {
        bSetOwner = true;
    }

    // Open a handle in the most cross platform way possible
    int createflags, shareflags, fd;
#if defined(Q_OS_UNIX)
    createflags = O_CREAT | O_RDWR;
    shareflags  = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
#elif defined(Q_OS_WIN)
    createflags = _O_CREAT | _O_RDWR;
    shareflags  = _S_IREAD | _S_IWRITE;
#endif
    fd = open (strName.toUtf8 (), createflags, shareflags);
    if (-1 == fd)
    {
        cerr << "Failed to open uniqueness file" << endl;
        return;
    }

    if (!fUnique.open (fd, QIODevice::ReadWrite | QIODevice::Unbuffered))
    {
        cerr << "Failed to transfer uniqueness file fd to QT" << endl;
        return;
    }

    if ((bSetOwner) && (writeOwner ()))
    {
        bOwner = true;
    }
}//UniqueAppHelper::UniqueAppHelper

UniqueAppHelper::~UniqueAppHelper ()
{
    if (bOwner)
    {
        writeChar (Clean);
        fUnique.close ();
        QString strName = QDir::tempPath ()
                        + QDir::separator ()
                        + "qgvdial_unique";
        QFile::remove (strName);
    }
}//UniqueAppHelper::~UniqueAppHelper

bool
UniqueAppHelper::setUnique ()
{
    bool rv = false;
    do { // Not a loop
        char ch = 0;
        if (!readChar (ch))
        {
            break;
        }

        if (bOwner)
        {
            emit log ("Was already owner");
            rv = true;
            break;
        }

        if (Clean != ch)
        {
            quint64 pid = 0;
            if ((readOwner (pid)) && (isPidValid (pid)))
            {
                emit log ("Looks like there is an owner");
                break;
            }
        }

        if (!writeOwner ())
        {
            emit log ("Could not set owner");
            break;
        }

        bOwner = true;
        emit log ("Owned!");

        rv = true;
    } while (0); // Not a loop
    return (rv);
}//UniqueAppHelper::setUnique

void
UniqueAppHelper::signalOriginal ()
{
    do { // Not a loop
        if (setUnique ())
        {
            emit log ("Is unique. what do you want me to do?");
            break;
        }

        if (!writeChar (WakeUp))
        {
            emit log ("Failed to write WakeUp");
            break;
        }

        emit log ("Original window should wake up now");
    } while (0); // Not a loop
}//UniqueAppHelper::signalOriginal

bool
UniqueAppHelper::isWakeSignaled ()
{
    bool rv = false;
    do { // Not a loop
        char ch;
        if (!readChar (ch))
        {
            emit log ("Cannot read character.");
            break;
        }

        if (WakeUp == ch)
        {
            rv = true;
            if (!writeChar (Owned))
            {
                emit log ("Cannot write character.");
                break;
            }
        }
    } while (0); // Not a loop
    return (rv);
}//UniqueAppHelper::isWakeSignaled

bool
UniqueAppHelper::writeChar (char ch)
{
    bool rv = false;
    do { // Not a loop
        if (!fUnique.isOpen ())
        {
            emit log ("file not open");
            break;
        }

        fUnique.seek (0);
        quint64 written = fUnique.write (&ch, sizeof(ch));
        fUnique.flush ();
        if (sizeof(ch) != written)
        {
            emit log ("Could not write to uniqueness file");
            break;
        }

        rv = true;
    } while (0); // Not a loop
    return (rv);
}//UniqueAppHelper::writeChar

bool
UniqueAppHelper::writeOwner ()
{
    bool rv = false;
    do { // Not a loop
        if (!writeChar (Owned))
        {
            emit log ("Failed to write owner flag");
            break;
        }

        quint64 pid = currentPid ();

        fUnique.seek (1);
        quint64 written = fUnique.write ((char *)&pid, sizeof(pid));
        fUnique.flush ();
        if (sizeof(pid) != written)
        {
            emit log ("Could not write the PID");
            break;
        }

        rv = true;
    } while (0); // Not a loop
    return (rv);
}//UniqueAppHelper::writeOwner

bool
UniqueAppHelper::readChar (char &ch)
{
    bool rv = false;
    do { // Not a loop
        if (!fUnique.isOpen ())
        {
            emit log ("file not open");
            break;
        }

        fUnique.flush ();
        fUnique.seek (0);
        QByteArray by = fUnique.read (sizeof(ch));
        if (sizeof(ch) != by.size ())
        {
            emit log ("Could not read");
            break;
        }
        ch = by.data ()[0];

        rv = true;
    } while (0); // Not a loop
    return (rv);
}//UniqueAppHelper::readChar

bool
UniqueAppHelper::readOwner (quint64 &pid)
{
    bool rv = false;
    do { // Not a loop
        char ch = Clean;
        if (!readChar (ch))
        {
            emit log ("Failed to read owner flag");
            break;
        }

        if (Clean == ch)
        {
            pid = 0;
            emit log ("No owner");
            rv = true;
            break;
        }

        fUnique.seek (1);
        quint64 byRead = fUnique.read ((char *)&pid, sizeof(pid));
        fUnique.flush ();
        if (sizeof(pid) != byRead)
        {
            emit log ("Could not read the PID");
            break;
        }

        rv = true;
    } while (0); // Not a loop
    return (rv);
}//UniqueAppHelper::readOwner

quint64
UniqueAppHelper::currentPid ()
{
    quint64 pid = 0;

#ifdef Q_OS_UNIX
    pid = getpid ();
#elif defined(Q_OS_WIN)
    pid = GetCurrentProcessId ();
#endif

    return pid;
}//UniqueAppHelper::currentPid

bool
UniqueAppHelper::isPidValid (quint64 pid)
{
    bool rv = false;
#ifdef Q_OS_UNIX
    QString strPath = QString("/proc/%1").arg(pid);
    if (QFileInfo(strPath).exists ())
    {
        rv = true;
    }
#elif defined(Q_OS_WIN)
    HANDLE hProcess = OpenProcess (PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (NULL != hProcess)
    {
        CloseHandle (hProcess);
        rv = true;
    }
#endif

    return (rv);
}//UniqueAppHelper::isPidValid
