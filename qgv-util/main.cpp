/*
 qgv-util is derived from vicar-utils and is licensed under GNU GPL.
 */

/*
@version: 0.5
@author: Sudheer K. <scifi1947 at gmail.com>
@license: GNU General Public License
*/

#include "telepathyutility.h"
#include <QtCore>
#include <QtDBus>

int main(int argc, char *argv[])
{
    qDBusRegisterMetaType<org::freedesktop::Telepathy::SimplePresence>();
    //qDBusRegisterMetaType<org::maemo::vicar::Profile>();
    //qDBusRegisterMetaType<org::maemo::vicar::ProfileList>();

    TelepathyUtility *tpUtility = new TelepathyUtility();

    if (argc > 1 && argv[1]){
        QString instruction = QString(argv[1]);
        if (instruction == "INSTALL"){
            //Check if Account already exists
            if (!tpUtility->accountExists()){
                qDebug() << "qgv-tp account not found. Creating ..";
                bool result = tpUtility->createAccount();
                if (!result) exit(1);
            }
            else{
                qDebug() << "qgv-qgv account found.";
            }
        }
        else if (instruction == "REMOVE"){
            bool result = tpUtility->deleteAccount();
            if (!result) exit(2);
        }
        else if (instruction == "ACCOUNTSTATUS"){
            QString status = tpUtility->getAccountStatus();
            qDebug() << "Account Status is "<< status;
        }
    }

    delete (tpUtility);
    tpUtility = 0;
}

