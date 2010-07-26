#include "VoicemailWidget.h"

VoicemailWidget::VoicemailWidget (QWidget *parent, Qt::WindowFlags f)
: ChildWindowBase (parent, f)
, player (this)
, playlist (this)
, slider (Qt::Horizontal, this)
, grid (this)
, updater (this)
, btnPlay (this)
, btnStop (this)
{
    btnPlay.setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    btnStop.setIcon(style()->standardIcon(QStyle::SP_MediaStop));

    updater.setInterval (100);
    updater.setSingleShot (true);
    slider.setMinimum (0);
    slider.setMaximum (100);

    QBoxLayout *box = new QBoxLayout (QBoxLayout::LeftToRight, this);
    box->addWidget (&btnPlay);
    box->addWidget (&btnStop);

    grid.addWidget (&slider, 0,0);
    grid.addItem (box, 1,0);
    this->setLayout (&grid);
    this->setAttribute (Qt::WA_QuitOnClose, false);

    playlist.setMediaObject (&player);

    // slider.valueChanged -> this.valueChanged
    QObject::connect (&slider, SIGNAL (sliderMoved (int)),
                       this  , SLOT   (valueChanged (int)));

    // updater.timeout -> this.sliderUpdate
    QObject::connect (&updater, SIGNAL (timeout ()),
                       this   , SLOT   (sliderUpdate ()));

    // Play and stop button functionality
    QObject::connect (&btnPlay, SIGNAL (clicked ()),
                       this   , SLOT (play_pause ()));
    QObject::connect (&btnStop, SIGNAL (clicked ()),
                       this   , SLOT (stop_clicked()));

    // player.positionChanged -> this.positionChanged
    QObject::connect (&player, SIGNAL (positionChanged (qint64)),
                       this  , SLOT   (positionChanged (qint64)));
}//VoicemailWidget::VoicemailWidget

bool
VoicemailWidget::play (const QString &strVmail)
{
    QFileInfo info (strVmail);
    if (!info.exists())
    {
        emit status (QString("Vmail file does not exist: %1").arg (strVmail));
        return (false);
    }

    QString strFullname = info.absoluteFilePath ();
    playlist.addMedia (QUrl::fromLocalFile(strFullname));
    player.setVolume (50);
    btnPlay.setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    playlist.next ();
    player.play ();
    updater.start ();
    this->show ();

    return (true);
}//VoicemailWidget::play

void
VoicemailWidget::valueChanged (int value)
{
    // value : 100
    // pos   : duration
    quint64 pos = (quint64) (((double)value * player.duration ()) / 100);
    player.setPosition (pos);
}//VoicemailWidget::valueChanged

void
VoicemailWidget::positionChanged (qint64 pos)
{
    // pos   :: duration
    // value :: 100
    int value = 0;
    if (0 != player.duration ())
    {
        value = (int) ((pos * 100) / player.duration ());
    }
    slider.setValue (value);
}//VoicemailWidget::positionChanged

void
VoicemailWidget::closeEvent (QCloseEvent *)
{
    cleanup ();
}//VoicemailWidget::closeEvent

void
VoicemailWidget::done (int r)
{
    cleanup ();
    ChildWindowBase::done (r);
}//VoicemailWidget::done

void
VoicemailWidget::cleanup ()
{
    updater.stop ();
    player.stop ();
    playlist.clear ();
}//VoicemailWidget::cleanup

void
VoicemailWidget::sliderUpdate ()
{
    positionChanged (player.position ());
    updater.start ();
}//VoicemailWidget::sliderUpdate

void
VoicemailWidget::play_pause ()
{
    if (playlist.isEmpty ())
    {
        btnPlay.setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        return;
    }

    switch (player.state ())
    {
    case QMediaPlayer::PlayingState:
        btnPlay.setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        player.pause ();
        break;
    case QMediaPlayer::StoppedState:
    case QMediaPlayer::PausedState:
        btnPlay.setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        player.play ();
        break;
    default:
        break;
    }
}//VoicemailWidget::play_pause

void
VoicemailWidget::stop_clicked ()
{
    btnPlay.setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    player.stop ();
}//VoicemailWidget::stop_clicked
