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
    m_logoImage = QPixmap(":/Statup screen images/TunerStudio OS logo.png");
    m_welcomeImage = QPixmap(":/Statup screen images/TunerStudio OS welcome message .png");

    if (qApp && qApp->primaryScreen()) {
        // Let's use a nice 1280x720 fixed size for the splash screen for consistency
        int targetW = 1280;
        int targetH = 720;
        
        if (!m_bgImage.isNull()) {
            QPixmap scaledBg = m_bgImage.scaled(targetW, targetH, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            
            // The car engine is a cutout with a transparent background. 
            // We must composite it over a very deep, dark cyberpunk lighting gradient
            QPixmap composite(targetW, targetH);
            QPainter p(&composite);
            
            p.fillRect(composite.rect(), QColor("#FFFFFF")); // Matching car hood white
            
            // Push car engine properly so it fills the bottom visually
            int ox = (targetW - scaledBg.width()) / 2;
            int oy = (targetH - scaledBg.height()) / 2 - 20; // Shifted up to touch the top ceiling
            p.drawPixmap(ox, oy, scaledBg);
            
            // Render a soft neutral vignette to maintain depth without blue tint
            QRadialGradient vign(targetW/2.0, targetH/2.0, targetW/1.1);
            vign.setColorAt(0.0, QColor(255, 255, 255, 0)); 
            vign.setColorAt(0.7, QColor(200, 205, 210, 120)); // Soft grey shadow
            vign.setColorAt(1.0, QColor(160, 165, 170, 200)); // Darker grey edges
            p.fillRect(composite.rect(), vign);
            
            p.end();
            
            m_bgImage = composite; // Save the composite as the real background
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

    // 2. Draw Welcome Message (Plaque) at Top Center
    if (!m_welcomeImage.isNull()) {
        int pw = w * 0.55; 
        int ph = m_welcomeImage.height() * pw / m_welcomeImage.width();
        painter.drawPixmap((w - pw) / 2, -110, pw, ph, m_welcomeImage); // Aggressively pulled to the absolute top ceiling
    }

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

    // 5. DIAGNOSTIC GANTRY 2.0 - MASTERPIECE UI
    int barW = w * 0.60; 
    int barH = 38;
    int barX = (w - barW) / 2;
    int barY = h - 80; 
    int lightCount = 5;
    int lightSpacing = barW / (lightCount + 1);
    int lightSize = 22;

    // A. PREMIUM POLISHED CARBON FIBER TUBE
    QLinearGradient tubeGrad(0, barY, 0, barY + barH);
    tubeGrad.setColorAt(0.0, QColor(40, 42, 45)); // Top highlight
    tubeGrad.setColorAt(0.15, QColor(15, 16, 18)); // Body
    tubeGrad.setColorAt(0.5, QColor(0, 0, 0));      // Depth shadow
    tubeGrad.setColorAt(0.85, QColor(10, 11, 13)); // Body return
    tubeGrad.setColorAt(1.0, QColor(45, 48, 52));  // Bottom bounce highlight
    
    painter.setPen(Qt::NoPen);
    painter.setBrush(tubeGrad);
    painter.drawRoundedRect(barX, barY, barW, barH, 19, 19);

    // Realistic Carbon Fiber Weave (Checkerboard Pattern)
    painter.setPen(QPen(QColor(255, 255, 255, 12), 1));
    for(int i=0; i<barW; i+=6) {
        for(int j=0; j<barH; j+=6) {
            if((i+j)%4 == 0) painter.drawRect(barX + i, barY + j, 2, 2);
        }
    }

    // B. INTEGRATED PROTECTION GLASS PANEL (Frosted Acrylic)
    painter.setBrush(QColor(100, 200, 255, 10)); // Very faint blue tint
    painter.setPen(QPen(QColor(255, 255, 255, 30), 1));
    painter.drawRoundedRect(barX + 15, barY + 6, barW - 30, barH - 12, 10, 10);
    
    // Scanline Texture on Glass
    painter.setPen(QPen(QColor(255, 255, 255, 20), 1));
    for(int s=barY + 10; s < barY + barH - 10; s+=3) {
        painter.drawLine(barX + 20, s, barX + barW - 20, s);
    }

    for(int i=0; i<lightCount; i++) {
        int lx = barX + (i + 1) * lightSpacing;
        int ly = barY + barH / 2;
        
        // C. HOLOGRAPHIC STAGE MARKERS
        painter.setPen(QColor(150, 180, 200, 100));
        painter.setFont(QFont("Consolas", 7));
        painter.drawText(lx - 12, barY + 4, QString("S-0%1").arg(i+1));

        // D. LIGHT MECHANICAL SOCKETS
        painter.setBrush(QColor(0, 0, 0));
        painter.setPen(QPen(QColor(100, 100, 100, 50), 1));
        painter.drawEllipse(QPoint(lx, ly), lightSize/2 + 2, lightSize/2 + 2);
        
        // E. LIGHT LOGIC & FX
        double threshold = (i + 1) * (100.0 / lightCount);
        bool isLit = (m_loadWidth >= threshold);
        bool allGreen = (m_loadWidth >= 100);

        if (allGreen) {
            // "LAUNCH READY" - High-End Plasma Bloom
            QRadialGradient pGlow(lx, ly, 30);
            pGlow.setColorAt(0.0, QColor(0, 255, 150, 230));
            pGlow.setColorAt(0.7, QColor(0, 255, 150, 30));
            pGlow.setColorAt(1.0, QColor(0, 255, 150, 0));
            painter.setBrush(pGlow);
            painter.drawEllipse(QPoint(lx, ly), 30, 30);
            
            painter.setBrush(Qt::white);
            painter.drawEllipse(QPoint(lx, ly), 4, 4); 
        } else if (isLit) {
            // SEQUENTIAL POWER IGNITION
            QRadialGradient rGlow(lx, ly, 25);
            rGlow.setColorAt(0.0, QColor(255, 50, 0, 240));
            rGlow.setColorAt(0.8, QColor(255, 50, 0, 40));
            rGlow.setColorAt(1.0, QColor(255, 50, 0, 0));
            painter.setBrush(rGlow);
            painter.drawEllipse(QPoint(lx, ly), 25, 25);
            
            painter.setBrush(Qt::white);
            painter.drawEllipse(QPoint(lx, ly), 4, 4);
        } else {
            // Ambient reflection in off state
            painter.setPen(QPen(QColor(255, 255, 255, 20), 1));
            painter.drawArc(lx - 6, ly - 6, 12, 12, 120 * 16, 60 * 16);
        }
    }

    // 6. ULTIMATE TELEMETRY HUD
    QColor hudColor = (m_loadWidth >= 100) ? QColor(0, 255, 150) : QColor(255, 80, 50);
    painter.setPen(hudColor);
    painter.setFont(QFont("Consolas", 10, QFont::Bold));
    QString mainText = (m_loadWidth >= 100) ? ">> CRITICAL_MISSION_INIT_SUCCESS :: LAunch_Ready" : 
                                              QString(">> ANALYZING_SYSTEM_VALVES :: PHASE_0%1").arg(m_msgIndex + 1);
    painter.drawText(QRect(0, barY + 48, w, 25), Qt::AlignCenter, mainText);
    
    // Sub-Frequency line
    painter.setPen(QPen(hudColor.darker(150), 1));
    painter.drawLine(barX + 50, barY + 70, barX + barW - 50, barY + 70);
    painter.setFont(QFont("Consolas", 8));
    painter.drawText(QRect(0, barY + 72, w, 15), Qt::AlignCenter, "F-SYNC: 104.2MHz // CORE_TEMP: 32C");
}

void TunerProSplashScreen::mousePressEvent(QMouseEvent *event) {
    // Override default QSplashScreen behavior
    event->accept();
}
