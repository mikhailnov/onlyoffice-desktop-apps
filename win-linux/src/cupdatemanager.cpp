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
    timer = new QTimer(this);
    timer->setSingleShot(false);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkUpdates()));
    readUpdateSettings();
    //netManager = new QNetworkAccessManager(this);
    //connect(netManager, &QNetworkAccessManager::finished, this, &CUpdater::onResult);
}

CUpdateManager::~CUpdateManager()
{

}

void CUpdateManager::checkUpdates()
{
    last_check = time(nullptr);
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    reg_user.setValue("Updates/last_check", static_cast<qlonglong>(last_check));
    reg_user.endGroup();

    //if (url.isEmpty())
      //  url = QUrl("https://nct.onlyoffice.com/sh/XHh");
    //QNetworkRequest request;
    //request.setUrl(url);
    //netManager->get(request);
    /*QObject::connect(netManager, &QNetworkAccessManager::sslErrors,
    [](QNetworkReply *reply, const QList<QSslError> &errors) {
        reply->ignoreSslErrors();
    };*/

    /*CURL *curl;
    FILE *fp;
    CURLcode res;
    const char *urls = "https://nct.onlyoffice.com/sh/XHh";
    char outfilename[FILENAME_MAX] = "/home/helg/Templates/page.json";
    curl = curl_easy_init();
    if (curl)
    {
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
    QTimer::singleShot(3000, this, [this]() {
        updateNeededCheking();
    });
    qDebug() << "Checked ...\n";
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
    const time_t DAY_TO_SEC = 10; // для проверки поставил 10 сек (должно быть 24*3600)
    const time_t WEEK_TO_SEC = 7*24*3600;
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

void CUpdateManager::sendMessage()
{
    QMessageBox msgBox(nullptr);
    msgBox.setWindowTitle("Only Office");
    msgBox.setWindowIcon(QIcon(":/res/icons/app.ico"));
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setFixedWidth(150);
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(tr("A new version of the program is available.\n"));
#if defined (Q_OS_LINUX)
    msgBox.setInformativeText(tr("Go to the update download page?"));
#elif defined (Q_OS_WIN)
    msgBox.setInformativeText(tr("Update the program?"));
#endif
    msgBox.setDetailedText(tr("Changelog is loading, wait ..."));
    QCheckBox *checkBox = new QCheckBox(&msgBox);
    checkBox->setText(tr("Don't show this message."));
    msgBox.setCheckBox(checkBox);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.setModal(true);
    switch (msgBox.exec()) {
    case QMessageBox::Yes:
        qDebug() << "Ok";
#if defined (Q_OS_LINUX)
        QDesktopServices::openUrl(QUrl("https://www.onlyoffice.com/en/download-desktop.aspx", QUrl::TolerantMode));
#elif defined (Q_OS_WIN)
        updateProgram();
#endif
        break;
    case QMessageBox::No:
        qDebug() << "No";
        break;
    default:
        break;
    }

    /*
    Отправка уведомления в окно About
    */
}

#if defined (Q_OS_WIN)
void CUpdateManager::updateProgram()
{
    /*
    Реализация
    */
}
#endif

/*void CUpdateManager::onResult(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError){
        QByteArray ReplyText = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(ReplyText);
        QJsonObject obj = doc.object();
        QJsonValue version = obj.value(QString("version"));
        QJsonValue date = obj.value(QString("date"));
        QJsonValue page = obj.value(QString("downloadPage"));
        qDebug() << "Version: " << version.toString();
        qDebug() << "Date: " << date.toString();
        qDebug() << "Page: " << page.toString();
    }
    else {
        qDebug() << "ERROR";
    }
    reply->deleteLater();
}*/
