#ifndef __VOICEMAILWIDGET_H__
#define __VOICEMAILWIDGET_H__

#include <QtGui>
#include <QMediaPlayer>

class VoicemailWidget : public QWidget
{
    Q_OBJECT

public:
    VoicemailWidget (QWidget *parent = 0, Qt::WindowFlags f = 0);
    bool play (const QString &strVmail);

signals:
    void log(const QString &strText, int level = 10);
    void status(const QString &strText, int timeout = 2000);

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
