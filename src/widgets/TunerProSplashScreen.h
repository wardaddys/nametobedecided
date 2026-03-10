#ifndef TUNERPROSPLASHSCREEN_H
#define TUNERPROSPLASHSCREEN_H

#include <QSplashScreen>
#include <QPropertyAnimation>
#include <QTimer>
#include <QPushButton>

class TunerProSplashScreen : public QSplashScreen {
    Q_OBJECT
public:
    enum Action {
        OpenLast,
        Browse,
        Exit
    };

    explicit TunerProSplashScreen(const QPixmap &pixmap = QPixmap());
    void startAnimations();

signals:
    void actionSelected(Action action);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onTick();
    void onMsgTick();

private:
    double m_starAngle;
    double m_loadWidth;
    int m_msgIndex;
    QStringList m_messages;
    QTimer m_animTimer;
    QTimer m_msgTimer;
    QPushButton *btn1;
    QPushButton *btn2;
    QPushButton *btn3;
};

#endif // TUNERPROSPLASHSCREEN_H
