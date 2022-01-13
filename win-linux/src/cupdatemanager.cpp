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

#include "cupdatemanager.h"


CUpdateManager::CUpdateManager(QObject *parent):
    QObject(parent),
    current_frequency(Frequency::DAY),
    last_check(0)
{
    std::wstring url = L"http://nct.onlyoffice.com/sh/XHh";
    //std::wstring url = L"http://download.onlyoffice.com/install/desktop/editors/windows/onlyoffice/updates/editors_update_x64.exe";
    downloader = new Downloader(url, false);
    downloader->SetEvent_OnComplete(onComplete);
    downloader->SetEvent_OnProgress(onProgress);
    //downloader->DownloadAsync();
#if defined (Q_OS_WIN)
    downloader->SetFilePath(_wtmpnam(nullptr));
#else
    const QString temp_file = QDir::homePath() + QString("/temp.json");
    downloader->SetFilePath(temp_file.toStdWString());
#endif

    timer = new QTimer(this);
    timer->setSingleShot(false);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkUpdates()));
    readUpdateSettings();
    //netManager = new QNetworkAccessManager(this);
    //connect(netManager, &QNetworkAccessManager::finished, this, &CUpdateManager::onResult);
}

CUpdateManager::~CUpdateManager()
{
    delete downloader;
}

void CUpdateManager::checkUpdates()
{
    last_check = time(nullptr);
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    reg_user.setValue("Updates/last_check", static_cast<qlonglong>(last_check));
    reg_user.endGroup();

    // =============== Check ================
    //QUrl url = QUrl("");
    //downloader->DownloadAsync();
    downloader->Start(0);

    //QNetworkRequest request;
    //request.setUrl(url);
    //netManager->get(request);

    /*CURL *curl;
    FILE *fp;
    CURLcode res;
    const char *urls = "https://nct.onlyoffice.com/sh/XHh";
    char outfilename[FILENAME_MAX] = "/home/helg/Templates/page.json";
    curl = curl_easy_init();
    if (curl) {
        fp = fopen(outfilename, "wb");
        curl_easy_setopt(curl, CURLOPT_URL, urls);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            qDebug() << stderr << curl_easy_strerror(res);
        }
        curl_easy_cleanup(curl);
        fclose(fp);
    }*/

    // ======================================

    QTimer::singleShot(3000, this, [this]() {
        updateNeededCheking();
    });
    qDebug() << "Checked ...";
}

void CUpdateManager::readUpdateSettings()
{
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    current_frequency = reg_user.value("Updates/frequency").toInt();
    last_check = time_t(reg_user.value("Updates/last_check").toLongLong());
    reg_user.endGroup();
    QTimer::singleShot(3000, this, [this]() {
        updateNeededCheking();
    });
}

void CUpdateManager::setNewUpdateSetting(const int& frequency)
{
    current_frequency = frequency;
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    reg_user.setValue("Updates/frequency", current_frequency);
    reg_user.endGroup();
    QTimer::singleShot(3000, this, [this]() {
        updateNeededCheking();
    });
}

void CUpdateManager::updateNeededCheking() {
    timer->stop();
    int interval = 0;
    const time_t DAY_TO_SEC = 24*3600; // 24*3600
    const time_t WEEK_TO_SEC = 7*24*3600; // 7*24*3600
    const time_t curr_time = time(nullptr);
    const time_t elapsed_time = curr_time - last_check;
    switch (current_frequency) {
    case Frequency::DAY:
        if (elapsed_time > DAY_TO_SEC) {
            checkUpdates();
        } else {
            interval = static_cast<int>(DAY_TO_SEC - elapsed_time);
            timer->setInterval(interval*1000);
            timer->start();
        }
        break;
    case Frequency::WEEK:
        if (elapsed_time > WEEK_TO_SEC) {
            checkUpdates();
        } else {
            interval = static_cast<int>(WEEK_TO_SEC - elapsed_time);
            timer->setInterval(interval*1000);
            timer->start();
        }
        break;
    case Frequency::DISABLED:
    default:
        break;
    }
}

#if defined (Q_OS_WIN)
void CUpdateManager::updateProgram()
{
    /*
    Реализация
    */
}
#endif

void CUpdateManager::onResult()
{
    if (true){
        QByteArray ReplyText;
        QJsonDocument doc = QJsonDocument::fromJson(ReplyText);
        QJsonObject obj = doc.object();
        // parse version
        QJsonValue version = obj.value(QString("version"));
        QJsonValue date = obj.value(QString("date"));
        // parse release notes
        QJsonValue release_notes = obj.value(QString("releaseNotes"));
        QJsonObject obj_1 = release_notes.toObject();
        QJsonValue en = obj_1.value(QString("en-EN"));
        QJsonValue ru = obj_1.value(QString("ru-RU"));
        // parse package
        QJsonValue package = obj.value(QString("package"));
        QJsonObject obj_2 = package.toObject();
        QJsonValue win_64 = obj_2.value(QString("win_64"));
        QJsonValue win_32 = obj_2.value(QString("win_32"));
        QJsonObject obj_3 = win_64.toObject();
        QJsonValue url_win_64 = obj_3.value(QString("url"));
        QJsonValue arguments_64 = obj_3.value(QString("installArguments"));
        QJsonObject obj_4 = win_32.toObject();
        QJsonValue url_win_32 = obj_4.value(QString("url"));
        QJsonValue arguments_32 = obj_4.value(QString("installArguments"));

        qDebug() << "Version: " << version.toString();
        qDebug() << "Date: " << date.toString();
        qDebug() << en.toString();
        qDebug() << ru.toString() << "\n";
        qDebug() << url_win_64.toString() << "\n" << arguments_64.toString() << "\n\n";
        qDebug() << url_win_32.toString() << "\n" << arguments_32.toString() << "\n\n";

        bool updateFlag = false;
        int curr_ver[4] = {VER_NUM_MAJOR, VER_NUM_MINOR, VER_NUM_BUILD, VER_NUM_REVISION};
        QStringList ver = version.toString().split('.');
        for (int i = 0; i < std::min(ver.size(), 4); i++) {
            if (ver.at(i).toInt() > curr_ver[i]) {
                updateFlag = true;
                break;
            }
        }
        emit onSendMessage(updateFlag);
    }
    else {
        qDebug() << "ERROR";
    }
    //reply->deleteLater();
}

void CUpdateManager::onComplete(int error)
{
    qDebug() << "Complete... " << error;
}

int CUpdateManager::onProgress(int percent)
{
    qDebug() << "Percent... " << percent;
    return percent;
}
