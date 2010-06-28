#ifndef __VOICEMAILWIDGET_H__
#define __VOICEMAILWIDGET_H__

#include "global.h"
#include "ChildWindowBase.h"
#include <QMediaPlayer>

class VoicemailWidget : public ChildWindowBase
{
    Q_OBJECT

public:
    VoicemailWidget (QWidget *parent = 0, Qt::WindowFlags f = 0);
    bool play (const QString &strVmail);

private slots:
    //! Slider value changed
    void valueChanged (int value);
    //! Player position changed
    void positionChanged (qint64 position);
    //! timer timeout reaches here to update the slider position
    void sliderUpdate ();

    //! Play/pause clicked
    void play_pause ();
    //! Stop clicked
    void stop_clicked ();

private:
    void closeEvent (QCloseEvent *event);

private:
    //! Qt Mobility's media player object
    QMediaPlayer   *player;

    //! The slider control for the vmail
    QSlider         slider;

    //! Grid
    QGridLayout     grid;

    //! Timer for updating slider position
    QTimer          updater;

    //! Play button
    QToolButton     btnPlay;
    //! Stop button
    QToolButton     btnStop;
};

#endif //__VOICEMAILWIDGET_H__
