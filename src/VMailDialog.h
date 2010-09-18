#ifndef VMAILDIALOG_H
#define VMAILDIALOG_H

#include <QtGui>
#include <QMediaPlayer>

namespace Ui {
    class VMailDialog;
}

class VMailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VMailDialog (QWidget *parent = 0);
    ~VMailDialog ();

    bool play (const QString &strVmail);

signals:
    void log (const QString &strText, int level = 10);
    void status (const QString &strText, int timeout = 3000);

private slots:
    void on_durationSlider_actionTriggered (int action);
    //! Play button clicked
    void on_btnPlay_clicked ();
    //! Stop button clicked
    void on_btnStop_clicked ();

    //! Player position changed
    void positionChanged (qint64 position);

private:
    Ui::VMailDialog *ui;
    bool            bIsBtnPlay;

    //! Qt Mobility's media player object
    QMediaPlayer    player;
};

#endif // VMAILDIALOG_H
