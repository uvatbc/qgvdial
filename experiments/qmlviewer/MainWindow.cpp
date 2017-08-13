#include "MainWindow.h"
#include "CQmlViewer.h"

#include <iostream>
using namespace std;

#define MAIN_QML_PATH "qml/harmattan/main.qml"

MainWindow::MainWindow(QObject *parent)
: QObject(parent)
, m_view(new CQmlViewer)
{
    QTimer::singleShot(10, this, SLOT(init()));
}//MainWindow::MainWindow

MainWindow::~MainWindow()
{
    if (m_view) {
        delete m_view;
        m_view = NULL;
    }
}

void
MainWindow::init()
{
    m_view->setMainQmlFile(MAIN_QML_PATH);
    m_view->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    m_view->showExpanded();

    do {
        QObject *mainPageStack = getQMLObject ("MainPageStack");
        if (NULL == mainPageStack) {
            break;
        }

        QObject *tfaMethodPage = getQMLObject ("TFASelectMethodPage");
        if (NULL == tfaMethodPage) {
            break;
        }

        QMetaObject::invokeMethod (mainPageStack, "pushTfaMethodDlg");

        QStringList options;
        options << "Op 1" << "Op 2";

        QMetaObject::invokeMethod(tfaMethodPage, "clearModel");
        for (int i = 0; i < options.length(); i++) {
            QMetaObject::invokeMethod(tfaMethodPage, "setModel",
                                      Q_ARG(QVariant, options[i]));
        }
    } while (0);
}//MainWindow::init

QObject *
MainWindow::getQMLObject(const char *pageName)
{
    QObject *pObj = NULL;
    do { // Begin cleanup block (not a loop)
        QObject *pRoot = (QObject *) m_view->rootObject ();
        if (NULL == pRoot) {
            Q_WARN(QString("Couldn't get root object in QML for %1")
                    .arg(pageName));
            break;
        }

        if (pRoot->objectName() == pageName) {
            pObj = pRoot;
            break;
        }

        pObj = pRoot->findChild <QObject*> (pageName);
        if (NULL == pObj) {
            Q_WARN(QString("Could not find page %1").arg (pageName));
            break;
        }
    } while (0); // End cleanup block (not a loop)

    return (pObj);
}//MainWindow::getQMLObject
