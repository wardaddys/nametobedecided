#include "TunerProSplashScreen.h"
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QMouseEvent>
#include <QToolButton>
#include <QStyle>

TunerProSplashScreen::TunerProSplashScreen(const QPixmap &pixmap)
    : QSplashScreen(pixmap, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::SplashScreen),
      m_starAngle(0), m_loadWidth(0), m_msgIndex(0) {
      
    m_bgImage = QPixmap(":/Statup screen images/TunerStudio OS backgroung image.png");

    if (qApp && qApp->primaryScreen()) {
        // Let's use a nice 1280x720 fixed size for the splash screen for consistency
        int targetW = 1280;
        int targetH = 720;
        
        if (!m_bgImage.isNull()) {
            QPixmap scaledBg = m_bgImage.scaled(targetW, targetH, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            
            // Just crop the image to perfectly fit the target size without any color overlays
            int cropX = std::max(0, (scaledBg.width() - targetW) / 2);
            int cropY = std::max(0, (scaledBg.height() - targetH) / 2);
            m_bgImage = scaledBg.copy(cropX, cropY, targetW, targetH);
            setPixmap(m_bgImage);  // This rectifies the physical QSplashScreen window mask
            resize(m_bgImage.size());
        } else {
            QSize sz(targetW, targetH);
            QPixmap blank(sz);
            blank.fill(QColor("#03050D"));
            setPixmap(blank);
            resize(sz);
        }
    }

    m_messages << "Initializing serial interface..."
               << "Loading INI configuration..."
               << "Preparing tuning tables..."
               << "Starting dashboard..."
               << "Ready.";
               
    connect(&m_animTimer, &QTimer::timeout, this, &TunerProSplashScreen::onTick);
    connect(&m_msgTimer, &QTimer::timeout, this, &TunerProSplashScreen::onMsgTick);

    // Create toolbuttons but make them completely transparent.
    // They will act only as invisible click/hover catchers over our drawn graphics.
    btn1 = new QToolButton(this);
    btn2 = new QToolButton(this);
    btn3 = new QToolButton(this);
    
    QString transparentStyle = "QToolButton { background: transparent; border: none; } QToolButton:pressed { background: rgba(255,255,255,10); }";
    btn1->setStyleSheet(transparentStyle);
    btn2->setStyleSheet(transparentStyle);
    btn3->setStyleSheet(transparentStyle);
    
    btn1->installEventFilter(this);
    btn2->installEventFilter(this);
    btn3->installEventFilter(this);
    
    btn1->setFixedSize(300, 80);
    btn2->setFixedSize(300, 80);
    btn3->setFixedSize(300, 80);

    btn1->hide(); btn2->hide(); btn3->hide();
    
    connect(btn1, &QToolButton::clicked, this, [this]() { emit actionSelected(OpenLast); });
    connect(btn2, &QToolButton::clicked, this, [this]() { emit actionSelected(Browse); });
    connect(btn3, &QToolButton::clicked, this, [this]() { emit actionSelected(Exit); });
}

bool TunerProSplashScreen::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Enter || event->type() == QEvent::Leave) {
        if (obj == btn1) m_hoverBtn1 = (event->type() == QEvent::Enter);
        if (obj == btn2) m_hoverBtn2 = (event->type() == QEvent::Enter);
        if (obj == btn3) m_hoverBtn3 = (event->type() == QEvent::Enter);
        update(); // Trigger repaint to show neon hover highlights
    }
    return QSplashScreen::eventFilter(obj, event);
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
            int w = width();
            int h = height();
            // Layout hidden buttons exactly over the sci-fi rectangles we will draw in paintEvent
            int bW = btn1->width();
            int spacing = 35;
            int totalW = (bW * 3) + (spacing * 2);
            int cx = w / 2;
            int startX = cx - (totalW / 2);
            // Height perfectly aligned within the new curved dashboard pane
            int y = h * 0.65; 
            btn1->move(startX, y);
            btn2->move(startX + bW + spacing, y);
            btn3->move(startX + (bW + spacing) * 2, y);
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
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    int w = width();
    int h = height();

    // 1. Draw Background Image
    if (!m_bgImage.isNull()) {
        painter.drawPixmap(0, 0, m_bgImage);
    } else {
        painter.fillRect(rect(), QColor("#11151c"));
    }

    // 2. Welcome Message Removed per request
    // 3. Center Logo Badge Removed per request

    // 4. Draw Sci-Fi Cyberpunk Holographic Buttons Manually
    if (btn1->isVisible()) {
        painter.setRenderHint(QPainter::Antialiasing);
        int bw = btn1->width();
        int bh = btn1->height();
        
        auto drawSciFiBtn = [&](int x, int y, QColor neonColor, QString title, QString subtitle, bool hovered, int iconType) {
            int radius = 15;
            
            // 1. DYNAMIC SHADOW / HOVER DEPTH (Ambient Occlusion)
            QPainterPath shadowPath;
            int shadowOff = hovered ? 8 : 4;
            shadowPath.addRoundedRect(x + 2, y + shadowOff, bw - 4, bh, radius, radius);
            painter.fillPath(shadowPath, QColor(0, 0, 0, 100)); // Blurry-like dark shadow

            // 2. SOLID 3D EXTRUSION (Thickness)
            QPainterPath depthPath;
            int d = 4; // Thickness
            depthPath.addRoundedRect(x, y + d, bw, bh, radius, radius);
            painter.fillPath(depthPath, neonColor.darker(300));

            // 3. MAIN BUTTON SHELL
            QPainterPath path;
            path.addRoundedRect(x, y, bw, bh, radius, radius);
            
            // Background: Deep dark space gradient
            QLinearGradient bgGrad(x, y, x, y + bh);
            bgGrad.setColorAt(0.0, QColor(25, 35, 45));
            bgGrad.setColorAt(1.0, QColor(5, 10, 15));
            painter.fillPath(path, bgGrad);
            
            // OUTER RADIANCE (Glow)
            if (hovered) {
                QPainterPath glowPath;
                glowPath.addRoundedRect(x - 3, y - 3, bw + 6, bh + 6, radius + 2, radius + 2);
                QRadialGradient radGlow(x + bw/2, y + bh/2, bw);
                radGlow.setColorAt(0.0, QColor(neonColor.red(), neonColor.green(), neonColor.blue(), 40));
                radGlow.setColorAt(1.0, Qt::transparent);
                painter.fillPath(glowPath, radGlow);
            }

            // 4. INNER BEVEL & GLOSS
            QPainterPath glossPath;
            glossPath.addRoundedRect(x + 2, y + 2, bw - 4, bh / 2, radius - 2, radius - 2);
            QLinearGradient gloss(x, y, x, y + bh / 2);
            gloss.setColorAt(0.0, QColor(255, 255, 255, 45));
            gloss.setColorAt(1.0, QColor(255, 255, 255, 0));
            painter.fillPath(glossPath, gloss);

            // 5. VIBRANT NEON BORDER
            painter.setPen(QPen(neonColor, hovered ? 3 : 2));
            painter.drawPath(path);
            
            // 6. ICON & TEXT
            int iconX = x + 30;
            int iconY = y + bh / 2;
            painter.setPen(QPen(Qt::white, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            if (iconType == 1) {
                painter.drawArc(iconX - 10, iconY - 10, 20, 20, 0, 270 * 16);
                painter.drawLine(iconX, iconY - 10, iconX + 6, iconY - 14);
                painter.drawLine(iconX, iconY - 10, iconX + 6, iconY - 4);
            } else if (iconType == 2) {
                painter.drawRect(iconX - 10, iconY - 5, 20, 12);
                painter.drawRect(iconX - 10, iconY - 8, 8, 4);
            } else if (iconType == 3) {
                painter.drawLine(iconX - 8, iconY - 8, iconX + 8, iconY + 8);
                painter.drawLine(iconX + 8, iconY - 8, iconX - 8, iconY + 8);
            }
            
            painter.setPen(Qt::white);
            painter.setFont(QFont("Segoe UI", 12, QFont::Bold));
            painter.drawText(x + 75, y + 25, bw - 85, 30, Qt::AlignLeft | Qt::AlignVCenter, title);
            
            painter.setPen(QColor(180, 210, 230));
            painter.setFont(QFont("Segoe UI", 8, QFont::Normal));
            painter.drawText(x + 75, y + 43, bw - 85, 25, Qt::AlignLeft | Qt::AlignVCenter, subtitle);
        };
        
        drawSciFiBtn(btn1->x(), btn1->y(), QColor("#00c3ff"), "OPEN LAST PROJECT", "Continue where you left off", m_hoverBtn1, 1);
        drawSciFiBtn(btn2->x(), btn2->y(), QColor("#00ff66"), "BROWSE", "Open an existing project", m_hoverBtn2, 2);
        drawSciFiBtn(btn3->x(), btn3->y(), QColor("#ff3333"), "EXIT", "Close TunerStudio OS", m_hoverBtn3, 3);
    }

    // 5. 3D REALISTIC TRAFFIC LIGHTS (F1 STYLE GRID)
    int numSignals = 5;
    int sigW = 40;
    int sigH = 65;
    int gap = 25;
    int totalW = (numSignals * sigW) + ((numSignals - 1) * gap);
    int startX = (w - totalW) / 2;
    int sigY = h - 90; // Position near the bottom

    // A. Draw Mounting Rail behind the lights
    QLinearGradient railGrad(0, sigY + 25, 0, sigY + 35);
    railGrad.setColorAt(0.0, QColor(50, 50, 50));
    railGrad.setColorAt(0.5, QColor(100, 100, 100)); // Specular highlight on metal tube
    railGrad.setColorAt(1.0, QColor(20, 20, 20));
    painter.setBrush(railGrad);
    painter.setPen(QPen(QColor(10, 10, 10), 1));
    painter.drawRoundedRect(startX - 30, sigY + 25, totalW + 60, 10, 4, 4);

    // Draw each individual 3D light housing
    for (int i = 0; i < numSignals; i++) {
        int x = startX + i * (sigW + gap);
        int y = sigY;
        
        // 1. The housing (Matte Black 3D Box)
        QPainterPath boxPath;
        boxPath.addRoundedRect(x, y, sigW, sigH, 6, 6);
        
        QLinearGradient boxGrad(x, y, x + sigW, y); // Horizontal lighting
        boxGrad.setColorAt(0.0, QColor(25, 25, 25));
        boxGrad.setColorAt(0.2, QColor(55, 55, 55)); // Left highlight
        boxGrad.setColorAt(0.7, QColor(15, 15, 15)); // Main body
        boxGrad.setColorAt(1.0, QColor(5, 5, 5));    // Right shadow
        painter.fillPath(boxPath, boxGrad);
        
        painter.setPen(QPen(QColor(0, 0, 0), 2));
        painter.drawPath(boxPath);
        
        // 2. The Socket Base
        int cx = x + sigW / 2;
        int cy = y + sigH / 2 + 3; // Shifted down slightly
        int r = 14;
        
        painter.setPen(Qt::NoPen);
        QRadialGradient socketGrad(cx, cy, r + 4);
        socketGrad.setColorAt(0.8, QColor(0, 0, 0));
        socketGrad.setColorAt(1.0, QColor(70, 70, 70)); // Inner bevel reflection
        painter.setBrush(socketGrad);
        painter.drawEllipse(QPoint(cx, cy), r + 4, r + 4);

        // Lens background (Unlit)
        painter.setBrush(QColor(15, 5, 5));
        painter.drawEllipse(QPoint(cx, cy), r, r);
        
        // LED dots texture
        painter.setPen(QPen(QColor(0, 0, 0, 100), 1));
        for(int dy = -r; dy <= r; dy += 3) {
            for(int dx = -r; dx <= r; dx += 3) {
                if(dx*dx + dy*dy < (r-2)*(r-2)) {
                    painter.drawPoint(cx + dx, cy + dy);
                }
            }
        }

        // Logic
        double threshold = (i + 1) * (100.0 / numSignals);
        bool isLit = (m_loadWidth >= threshold);
        bool allGreen = (m_loadWidth >= 100);

        if (allGreen) {
            // GREEN LIGHT
            QRadialGradient lensGlow(cx, cy, r);
            lensGlow.setColorAt(0.0, QColor(200, 255, 200)); // White-hot core
            lensGlow.setColorAt(0.4, QColor(0, 255, 60));    // Vibrant green
            lensGlow.setColorAt(1.0, QColor(0, 120, 20));    // Dark edge
            painter.setBrush(lensGlow);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QPoint(cx, cy), r, r);
            
            // Subtle external halo
            QRadialGradient halo(cx, cy, r * 2.5);
            halo.setColorAt(0.0, QColor(0, 255, 60, 80));
            halo.setColorAt(1.0, QColor(0, 255, 60, 0));
            painter.setBrush(halo);
            painter.drawEllipse(QPoint(cx, cy), r * 2.5, r * 2.5);
            
        } else if (isLit) {
            // RED LIGHT
            QRadialGradient lensGlow(cx, cy, r);
            lensGlow.setColorAt(0.0, QColor(255, 200, 200)); // White-hot core
            lensGlow.setColorAt(0.4, QColor(255, 30, 0));    // Vibrant red
            lensGlow.setColorAt(1.0, QColor(120, 0, 0));     // Dark edge
            painter.setBrush(lensGlow);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QPoint(cx, cy), r, r);
            
            // Subtle external halo
            QRadialGradient halo(cx, cy, r * 2.5);
            halo.setColorAt(0.0, QColor(255, 30, 0, 90));
            halo.setColorAt(1.0, QColor(255, 30, 0, 0));
            painter.setBrush(halo);
            painter.drawEllipse(QPoint(cx, cy), r * 2.5, r * 2.5);
            
        } else {
            // UNLIT - Glass Reflection
            QPainterPath reflection;
            reflection.arcMoveTo(cx - r + 1, cy - r + 1, (r-1)*2, (r-1)*2, 45);
            reflection.arcTo(cx - r + 1, cy - r + 1, (r-1)*2, (r-1)*2, 45, 90);
            reflection.arcTo(cx - r + 3, cy - r + 5, (r-3)*2, (r-5)*2, 135, -90);
            
            QLinearGradient reflGrad(cx, cy - r, cx, cy);
            reflGrad.setColorAt(0.0, QColor(255, 255, 255, 140));
            reflGrad.setColorAt(1.0, QColor(255, 255, 255, 0));
            painter.setBrush(reflGrad);
            painter.setPen(Qt::NoPen);
            painter.drawPath(reflection);
        }

        // 3. The Visor (3D Hood)
        QPainterPath hood;
        int vr = r + 4; // Visor radius outer
        int vi = r + 2; // Visor radius inner
        hood.arcMoveTo(cx - vr, cy - vr - 6, vr*2, vr*2 + 6, 180);
        hood.arcTo(cx - vr, cy - vr - 6, vr*2, vr*2 + 6, 180, -180); // Outer curve
        hood.arcTo(cx - vi, cy - vi, vi*2, vi*2, 0, 180);            // Inner curve
        
        QLinearGradient hoodGrad(cx, cy - vr - 6, cx, cy);
        hoodGrad.setColorAt(0.0, QColor(70, 70, 70)); // Highlight on top rim
        hoodGrad.setColorAt(0.4, QColor(25, 25, 25)); // Body
        hoodGrad.setColorAt(1.0, QColor(5, 5, 5));    // Base shadow
        painter.setBrush(hoodGrad);
        painter.setPen(QPen(QColor(10, 10, 10), 1));
        painter.drawPath(hood);
    }
}

void TunerProSplashScreen::mousePressEvent(QMouseEvent *event) {
    // Override default QSplashScreen behavior
    event->accept();
}
