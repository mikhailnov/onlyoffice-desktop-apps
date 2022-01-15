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
    downloadMode(Mode::CHECK_UPDATES),
    locale("en-EN"),
    last_check(0)
{
#if defined (Q_OS_WIN)
    package_url = L"";
#endif
    changelog_url = L"";
    check_url = CHECK_URL;
    downloader = new Downloader(check_url, false);
    downloader->SetEvent_OnComplete([this](int error) {
        if (error == 0) {
            qDebug() << "Download complete... ";
            switch (downloadMode) {
            case Mode::CHECK_UPDATES:
                onLoadCheckFinished();
                break;
            case Mode::DOWNLOAD_CHANGELOG:
                onLoadChangelogFinished();
                break;
#if defined (Q_OS_WIN)
            case Mode::DOWNLOAD_UPDATES:
                onLoadUpdateFinished();
                break;
#endif
            default:
                break;
            }
        }
        else {
            qDebug() << "Download error: " << error;
        }
    });
    downloader->SetEvent_OnProgress([this](int percent) {   // не отрабатывает
        qDebug() << "Precent... " << percent;
        emit progresChanged(percent);
    });
    timer = new QTimer(this);
    timer->setSingleShot(false);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkUpdates()));
    readUpdateSettings();
}

CUpdateManager::~CUpdateManager()
{
    delete downloader;
}

void CUpdateManager::checkUpdates()
{
#if defined (Q_OS_WIN)
    package_url = L"";
    package_args = L"";
#endif
    changelog_url = L"";
    last_check = time(nullptr);
    GET_REGISTRY_USER(reg_user);
    locale = reg_user.value("locale").toString();
    qDebug() << locale;
    reg_user.beginGroup("Updates");
    reg_user.setValue("Updates/last_check", static_cast<qlonglong>(last_check));
    reg_user.endGroup();

    // =========== Download JSON ============
    downloadMode = Mode::CHECK_UPDATES;
    downloader->SetFileUrl(check_url);
    QUuid uuid = QUuid::createUuid();
    const QString tmp_name = uuid.toString().replace(QRegularExpression("[{|}]+"), "") + QString(".json");
    const QString tmp_file = QDir::tempPath() + QDir::separator() + tmp_name;
    downloader->SetFilePath(tmp_file.toStdWString());
    downloader->Start(0);   
    // ======================================

    QTimer::singleShot(3000, this, [this]() {
        updateNeededCheking();
    });
    qDebug() << "Updates checked ...";
}

void CUpdateManager::readUpdateSettings()
{
    GET_REGISTRY_USER(reg_user);
    locale = reg_user.value("locale").toString();
    qDebug() << locale;
    reg_user.beginGroup("Updates");
    current_frequency = reg_user.value("Updates/frequency").toInt();
    last_check = time_t(reg_user.value("Updates/last_check").toLongLong());
    reg_user.endGroup();
#if defined (Q_OS_WIN)
    reg_user.beginGroup("Temp"); // Удаление пакета обновления при старте программы
    const QString tmp_file = reg_user.value("Temp/temp_file").toString();
    reg_user.endGroup();
    if (!tmp_file.isEmpty()) {
        if (QDir().exists(tmp_file)) QDir().remove(tmp_file);
        reg_user.beginGroup("Temp");
        reg_user.setValue("Temp/temp_file", QString(""));
        reg_user.endGroup();
    }
#endif
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
void CUpdateManager::loadUpdates()
{
    if (package_url != L"") {
        downloadMode = Mode::DOWNLOAD_UPDATES;
        downloader->SetFileUrl(package_url);
        QUuid uuid = QUuid::createUuid();
        const QString tmp_name = uuid.toString().replace(QRegularExpression("[{|}]+"), "") + QString(".exe");
        const QString tmp_file = QDir::tempPath() + QDir::separator() + tmp_name;
        downloader->SetFilePath(tmp_file.toStdWString());
        downloader->Start(0);
    }
}

void CUpdateManager::cancelLoading()
{
    const QString path = QString::fromStdWString(downloader->GetFilePath());
    downloader->Stop();
    if (QDir().exists(path)) QDir().remove(path);
}

void CUpdateManager::onLoadUpdateFinished()
{
    const QString path = QString::fromStdWString(downloader->GetFilePath());
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Temp");
    reg_user.setValue("Temp/temp_file", path);
    reg_user.endGroup();
    /*
    * Реализация
    */
}
#endif

void CUpdateManager::onLoadCheckFinished()
{
    const QString path = QString::fromStdWString(downloader->GetFilePath());
    qDebug() << path;
    QFile jsonFile(path);
    if (jsonFile.open(QIODevice::ReadOnly)) {
        QByteArray ReplyText = jsonFile.readAll();
        jsonFile.close();
        QJsonDocument doc = QJsonDocument::fromJson(ReplyText);
        QJsonObject obj = doc.object();

        // parse version
        QJsonValue version = obj.value(QString("version"));
        QJsonValue date = obj.value(QString("date"));

        // parse release notes
        QJsonValue release_notes = obj.value(QString("releaseNotes"));
        QJsonObject obj_1 = release_notes.toObject();
        const QString page = (locale == "ru-RU") ? "ru-RU" : "en-EN";
        QJsonValue changelog = obj_1.value(page);
        changelog_url = changelog.toString().toStdWString();

        // parse package
#if defined (Q_OS_WIN)
        QJsonValue package = obj.value(QString("package"));
        QJsonObject obj_2 = package.toObject();
    #if defined (Q_OS_WIN64)
        QJsonValue win = obj_2.value(QString("win_64"));
    #elif defined (Q_OS_WIN32)
        QJsonValue win = obj_2.value(QString("win_32"));
    #endif
        QJsonObject obj_3 = win.toObject();
        QJsonValue url_win = obj_3.value(QString("url"));
        QJsonValue arguments = obj_3.value(QString("installArguments"));
        package_url = url_win.toString().toStdWString();
        package_args = arguments.toString().toStdWString();
        qDebug() << url_win.toString() << "\n" << arguments.toString();
#endif
        qDebug() << "Version: " << version.toString();
        qDebug() << "Date: " << date.toString();
        qDebug() << "Changelog: " << changelog.toString();

        bool updateFlag = false;
        int curr_ver[4] = {VER_NUM_MAJOR, VER_NUM_MINOR, VER_NUM_BUILD, VER_NUM_REVISION};
        QStringList ver = version.toString().split('.');
        for (int i = 0; i < std::min(ver.size(), 4); i++) {
            if (ver.at(i).toInt() > curr_ver[i]) {
                updateFlag = true;
                break;
            }
        }
        emit checkFinished(updateFlag);
    }
    if (QDir().exists(path)) QDir().remove(path);
}

void CUpdateManager::loadChangelog()
{
    if (changelog_url != L"") {
        downloadMode = Mode::DOWNLOAD_CHANGELOG;
        downloader->SetFileUrl(changelog_url);
        QUuid uuid = QUuid::createUuid();
        const QString tmp_name = uuid.toString().replace(QRegularExpression("[{|}]+"), "") + QString(".html");
        const QString tmp_file = QDir::tempPath() + QDir::separator() + tmp_name;
        downloader->SetFilePath(tmp_file.toStdWString());
        downloader->Start(0);
    }
}

void CUpdateManager::onLoadChangelogFinished()
{
    const QString path = QString::fromStdWString(downloader->GetFilePath());
    QFile htmlFile(path);
    if (htmlFile.open(QIODevice::ReadOnly)) {
        const QString html = QString(htmlFile.readAll());
        htmlFile.close();
        emit changelogLoaded(html);
    }
    if (QDir().exists(path)) QDir().remove(path);
}
