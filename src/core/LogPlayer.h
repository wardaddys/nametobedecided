#ifndef LOGPLAYER_H
#define LOGPLAYER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QList>
#include "ECUData.h"

class LogPlayer : public QObject {
    Q_OBJECT

public:
    explicit LogPlayer(QObject *parent = nullptr);
    ~LogPlayer();

    bool loadLog(const QString &filePath);
    void play();
    void pause();
    void stop();
    void seek(int percentage); // 0 to 100

    bool isPlaying() const;
    bool hasLogLoaded() const;
    
    int getTotalDurationMs() const;
    int getCurrentTimeMs() const;
    QString getLogName() const;

signals:
    void dataFrameReady(const RealTimeData &data);
    void playbackProgress(int currentMs, int totalMs);
    void stateChanged(bool isPlaying);
    void logLoaded(bool success);

private slots:
    void onTick();

private:
    QList<RealTimeData> m_logData;
    QTimer m_timer;
    int m_currentIndex;
    bool m_isPlaying;
    QString m_logName;

    void parseCsvLine(const QString &line);
};

#endif // LOGPLAYER_H
