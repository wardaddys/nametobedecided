#ifndef TUNERPROSPLASHSCREEN_H
#define TUNERPROSPLASHSCREEN_H

#include <QSplashScreen>
#include <QPropertyAnimation>
#include <QTimer>
#include <QToolButton>
#include <QStringList>

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
    bool eventFilter(QObject *watched, QEvent *event) override;

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
    QToolButton *btn1;
    QToolButton *btn2;
    QToolButton *btn3;
    
    bool m_hoverBtn1 = false;
    bool m_hoverBtn2 = false;
    bool m_hoverBtn3 = false;
    
    QPixmap m_bgImage;
};

#endif // TUNERPROSPLASHSCREEN_H
