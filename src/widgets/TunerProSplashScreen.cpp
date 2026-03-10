#include "TunerProSplashScreen.h"
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QMouseEvent>

TunerProSplashScreen::TunerProSplashScreen(const QPixmap &pixmap)
    : QSplashScreen(pixmap, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::SplashScreen),
      m_starAngle(0), m_loadWidth(0), m_msgIndex(0) {
      
    if (qApp && qApp->primaryScreen()) {
        QSize sz = qApp->primaryScreen()->size();
        QPixmap blank(sz);
        blank.fill(QColor("#03050D"));
        setPixmap(blank);
    }

      
    m_messages << "Initializing serial interface..."
               << "Loading INI configuration..."
               << "Preparing tuning tables..."
               << "Starting dashboard..."
               << "Ready.";
               
    connect(&m_animTimer, &QTimer::timeout, this, &TunerProSplashScreen::onTick);
    // Wait for startAnimations to trigger load width
    connect(&m_msgTimer, &QTimer::timeout, this, &TunerProSplashScreen::onMsgTick);

    // Create buttons (hidden initially)
    btn1 = new QPushButton("Open Last Project", this);
    btn2 = new QPushButton("Browse", this);
    btn3 = new QPushButton("Exit", this);
    
    QString btnStyle = "QPushButton { background-color: #1A2640; border: 1px solid rgba(0,229,200,130); color: #EEF2FF; font-family: 'DM Sans', 'Segoe UI'; font-size: 14px; padding: 12px 32px; } QPushButton:hover { background-color: rgba(0,229,200,16); border-color: #00E5C8; }";
    btn1->setStyleSheet(btnStyle);
    btn2->setStyleSheet(btnStyle);
    btn3->setStyleSheet(btnStyle);

    btn1->hide(); btn2->hide(); btn3->hide();
    
    connect(btn1, &QPushButton::clicked, this, [this]() { emit actionSelected(OpenLast); });
    connect(btn2, &QPushButton::clicked, this, [this]() { emit actionSelected(Browse); });
    connect(btn3, &QPushButton::clicked, this, [this]() { emit actionSelected(Exit); });
}

void TunerProSplashScreen::startAnimations() {
    m_animTimer.start(16); // ~60fps
    m_msgTimer.start(600); // text changes
}

void TunerProSplashScreen::onTick() {
    m_starAngle += 0.15;
    
    if (m_loadWidth < 100.0) {
        m_loadWidth += (100.0 / (2.5 * 60.0)); // 2.5 seconds to reach 100%
    } else {
        // Fade in buttons around 1.5-2.5 sec, let's just show them when loadWidth > 60
        if (!btn1->isVisible()) {
            btn1->show(); btn2->show(); btn3->show();
            int bW = btn1->width() + btn2->width() + btn3->width() + 32;
            int cx = width() / 2;
            int y = height() * 2 / 3;
            btn1->move(cx - bW/2, y);
            btn2->move(cx - bW/2 + btn1->width() + 16, y);
            btn3->move(cx - bW/2 + btn1->width() + 16 + btn2->width() + 16, y);
        }
    }
    update();
}

void TunerProSplashScreen::onMsgTick() {
    if (m_msgIndex < m_messages.size() - 1) {
        m_msgIndex++;
    }
}

void TunerProSplashScreen::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // bg
    painter.fillRect(rect(), QColor("#03050D"));

    int w = width();
    int h = height();

    // bg pattern
    painter.setPen(QPen(QColor(30, 46, 74, 15), 0.8)); // #1E2E4A with opacity 0.06 ~ 15
    for(int ty = 0; ty < h+120; ty += 120) {
        for(int tx = 0; tx < w+120; tx += 120) {
            QPolygonF star;
            for(int i = 0; i < 16; i++) {
                double rad = (i % 2 == 0) ? 50.0 : 29.0;
                double a = qDegreesToRadians(-90.0 + i * (360.0/16.0));
                star << QPointF(tx + rad*cos(a), ty + rad*sin(a));
            }
            painter.drawPolygon(star);
        }
    }

    int cx = w / 2;
    int cy = h / 2;

    // glow
    QRadialGradient glow(cx, cy, 600);
    glow.setColorAt(0, QColor(0, 229, 200, 20)); // 0.08 alpha ~ 20
    glow.setColorAt(1, Qt::transparent);
    painter.fillRect(rect(), glow);

    // large star centered
    painter.save();
    painter.translate(cx, cy);
    painter.rotate(m_starAngle);
    
    QLinearGradient starGrad(-250, -250, 250, 250);
    starGrad.setColorAt(0, QColor("#00E5C8"));
    starGrad.setColorAt(0.33, QColor("#9B59F5"));
    starGrad.setColorAt(0.66, QColor("#00D97E"));
    starGrad.setColorAt(1.0, QColor("#F5A623"));
    
    painter.setPen(QPen(starGrad, 1.5));
    QPolygonF bigStar;
    for(int i = 0; i < 16; i++) {
        double rad = (i % 2 == 0) ? 250.0 : 145.0; // scaled 5x
        double a = qDegreesToRadians(-90.0 + i * (360.0/16.0));
        bigStar << QPointF(rad*cos(a), rad*sin(a));
    }
    painter.drawPolygon(bigStar);
    painter.restore();

    int logoY = cy - 80;
    
    // octagonal frame
    QPolygonF oct;
    for (int i=0; i<8; i++) {
        double a = qDegreesToRadians(-90.0 + i * 45.0);
        oct << QPointF(cx + 80*cos(a), logoY + 80*sin(a));
    }
    painter.setPen(QPen(starGrad, 1.5));
    painter.drawPolygon(oct);

    // TP
    painter.setPen(QColor("#00E5C8"));
    painter.setFont(QFont("JetBrains Mono", 28, QFont::Bold));
    painter.drawText(QRectF(cx-80, logoY-80, 160, 160), Qt::AlignCenter, "TP");

    // Tunerpro
    painter.setPen(QColor("#EEF2FF"));
    QFont fName("Geometric", 48, QFont::Bold); // Fallback to system geometric if not exist
    fName.setLetterSpacing(QFont::AbsoluteSpacing, 8);
    painter.setFont(fName);
    painter.drawText(QRect(0, logoY + 100, w, 60), Qt::AlignCenter, "TUNERPRO");

    // Tagline
    painter.setPen(QColor("#7A8FAD"));
    painter.setFont(QFont("Sans Serifs", 16));
    painter.drawText(QRect(0, logoY + 160, w, 30), Qt::AlignCenter, "Open-Source ECU Tuning");

    // v2.0
    painter.setPen(QColor("#3D5070"));
    painter.setFont(QFont("JetBrains Mono", 13));
    painter.drawText(w - 60, h - 20, "v2.0");

    // Load bar
    int barW = w * 0.6;
    int barX = (w - barW) / 2;
    int barY = h - 100;

    painter.fillRect(barX, barY, barW, 2, QColor("#1A2640"));
    
    double fillW = barW * qMin(m_loadWidth / 100.0, 1.0);
    if(fillW > 0) {
        QLinearGradient fg(barX, 0, barX + fillW, 0);
        fg.setColorAt(0, QColor("#00E5C8"));
        fg.setColorAt(1, QColor("#9B59F5"));
        painter.fillRect(barX, barY, fillW, 2, QBrush(fg));
        
        // glow
        if (fillW > 20) {
            QLinearGradient gl(barX + fillW - 20, 0, barX + fillW, 0);
            gl.setColorAt(0, Qt::transparent);
            gl.setColorAt(1, QColor(255, 255, 255, 128));
            painter.fillRect(barX + fillW - 20, barY - 1, 20, 4, gl);
        }
    }

    // load text
    painter.setPen(QColor("#7A8FAD"));
    painter.setFont(QFont("JetBrains Mono", 13));
    painter.drawText(QRect(0, barY + 10, w, 30), Qt::AlignCenter, m_messages[m_msgIndex]);
}

void TunerProSplashScreen::mousePressEvent(QMouseEvent *event) {
    // Override default QSplashScreen behavior so clicking bg doesn't hide it
    // which would leave the Main EventLoop hanging.
    event->accept();
}
