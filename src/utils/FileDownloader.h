#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QFile>
#include <QString>
#include <functional>

class FileDownloader : public QObject {
public:
    explicit FileDownloader(QObject *parent = nullptr) : QObject(parent) {}
    
    void download(const QString& url, const QString& savePath, std::function<void(bool, QString)> callback) {
        QNetworkRequest request((QUrl(url)));
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        
        QNetworkReply *reply = m_manager.get(request);
        connect(reply, &QNetworkReply::finished, [this, reply, savePath, callback]() {
            if (reply->error() == QNetworkReply::NoError) {
                QFile file(savePath);
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(reply->readAll());
                    file.close();
                    callback(true, "");
                } else {
                    callback(false, "Could not open file for writing: " + savePath);
                }
            } else {
                callback(false, reply->errorString());
            }
            reply->deleteLater();
        });
    }

private:
    QNetworkAccessManager m_manager;
};

#endif // FILEDOWNLOADER_H
