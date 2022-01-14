/*
 * (c) Copyright Ascensio System SIA 2010-2019
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 */

#ifndef CUPDATEMANAGER_H
#define CUPDATEMANAGER_H

#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QMessageBox>
#include <QIcon>
#include <QUrl>
#include <QDir>
//#include <QSslConfiguration>
#include <QDesktopServices>
/*#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>*/
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
//#include <QAbstractButton>
//#include <QLabel>
//#include <QSizePolicy>
#include <QCheckBox>
//#include <QSslSocket>
#include <QDebug>
//#include <curl/curl.h>
//#include <curl/easy.h>
#include <ctime>
#include <algorithm>
#include "defines.h"
#include "version.h"
#include "Network/FileTransporter/include/FileTransporter.h"

typedef std::wstring WString;
typedef NSNetwork::NSFileTransport::CFileDownloader Downloader;


class CUpdateManager: public QObject
{
    Q_OBJECT

public:

    explicit CUpdateManager(QObject *parent = nullptr);

    ~CUpdateManager();

    void setNewUpdateSetting(const int& frequency);

private:

    void readUpdateSettings();

    void updateNeededCheking();

#if defined (Q_OS_WIN)
    void updateProgram();
#endif

    QTimer *timer;

    int current_frequency;

    time_t last_check;

    enum Frequency {
        DAY, WEEK, DISABLED
    };

    enum Mode {
        CHECK_UPDATES, DOWNLOAD_UPDATES
    };

    int downloadMode;

    Downloader *downloader;

    //QNetworkAccessManager *netManager;

public slots:

    void checkUpdates();

       signals:

    void onSendMessage(const bool &updateFlag);

private slots:

    void onResult();

    void onComplete(int error);

    int onProgress(int percent);

};


#endif // CUPDATEMANAGER_H
