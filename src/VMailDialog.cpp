#include "VMailDialog.h"
#include "ui_VMailDialog.h"

VMailDialog::VMailDialog (QWidget *parent)
: QDialog(parent)
, ui(new Ui::VMailDialog)
, player (this)
{
    ui->setupUi (this);

#ifdef Q_WS_MAEMO_5
    this->setAttribute (Qt::WA_Maemo5StackedWindow);
#endif

    bIsBtnPlay = false;
    ui->btnPlay->setIcon (style()->standardIcon(QStyle::SP_MediaPause));
    ui->btnStop->setIcon (style()->standardIcon(QStyle::SP_MediaStop));

    // player.positionChanged -> this.positionChanged
    QObject::connect (&player, SIGNAL (positionChanged (qint64)),
                       this  , SLOT   (positionChanged (qint64)));
}//VMailDialog::VMailDialog

VMailDialog::~VMailDialog ()
{
    delete ui;
}//VMailDialog::~VMailDialog

void
VMailDialog::on_btnPlay_clicked ()
{
    switch (player.state ())
    {
    case QMediaPlayer::PlayingState:
        player.pause ();
        ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        break;
    case QMediaPlayer::StoppedState:
    case QMediaPlayer::PausedState:
        player.play ();
        ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        break;
    default:
        break;
    }
}//VMailDialog::on_btnPlay_clicked

void
VMailDialog::on_btnStop_clicked ()
{
    ui->btnPlay->setIcon (style()->standardIcon(QStyle::SP_MediaPlay));
    player.stop ();
}//VMailDialog::on_btnStop_clicked

void
VMailDialog::positionChanged (qint64 position)
{
    // pos   :: duration
    // value :: 100
    int value = 0;
    if (0 != player.duration ())
    {
        value = (int) ((position * 100) / player.duration ());
    }
    ui->durationSlider->setValue (value);
}//VMailDialog::positionChanged

bool
VMailDialog::play (const QString &strVmail)
{
    QFileInfo info (strVmail);
    if (!info.exists())
    {
        emit status (QString("Vmail file does not exist: %1").arg (strVmail));
        return (false);
    }

    QString strFullname = info.absoluteFilePath ();
    player.setMedia (QUrl::fromLocalFile(strFullname));
    player.setVolume (50);
    ui->btnPlay->setIcon (style()->standardIcon(QStyle::SP_MediaPause));
    player.play ();
//    updater.start ();
    this->show ();

    return (true);
}//VMailDialog::play

void
VMailDialog::on_durationSlider_actionTriggered (int /*action*/)
{
    qint64 duration = player.duration ();
    if (0 == duration) {
        emit log ("Duration is 0");
        return;
    }

    int position = ui->durationSlider->value ();

    player.setPosition (position * duration / 100);
}//VMailDialog::on_durationSlider_actionTriggered
