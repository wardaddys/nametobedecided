#include "InjectorVisualWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QtMath>
#include <algorithm>

namespace {
const QColor STEEL_HI   (0xE8, 0xEC, 0xF4);
const QColor STEEL_MID  (0xA0, 0xA8, 0xB8);
const QColor STEEL_LO   (0x62, 0x6A, 0x7A);
const QColor STEEL_DK   (0x38, 0x3E, 0x4A);
const QColor STEEL_DKR  (0x1C, 0x1F, 0x26);
const QColor BLACK_RIM  (0x08, 0x0A, 0x0E);

// Fuel spray colours
const QColor FUEL_CORE  (0xE0, 0xF2, 0xFE, 200); // bright white-blue
const QColor FUEL_MIST  (0x38, 0xBD, 0xF8, 120); // cyan
const QColor FUEL_EDGE  (0x02, 0x84, 0xC7,  0);  // dark blue fade
}

InjectorVisualWidget::InjectorVisualWidget(QWidget *parent) : QWidget(parent) {
    setMinimumSize(240, 240);
}

void InjectorVisualWidget::setDuty(double pct) {
    pct = std::max(0.0, std::min(100.0, pct));
    if (qFuzzyCompare(pct + 1.0, m_duty + 1.0)) return;
    m_duty = pct;
    update();
}

void InjectorVisualWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const double W = width();
    const double H = height();
    const double cx = W / 2.0;

    // The injector sits in the top half, spraying downwards
    const double injW = W * 0.35;
    const double injH = H * 0.40;
    const double injY = H * 0.15;

    // ── 1. Fuel Spray Cone (drawn behind the nozzle) ────────────────────
    if (m_duty > 0.5) {
        double dutyNorm = m_duty / 100.0;
        
        // Spray width and length depend on duty cycle
        double sprayY_start = injY + injH - injH * 0.10;
        double sprayY_end   = sprayY_start + H * 0.40 * (0.4 + 0.6 * dutyNorm);
        double sprayW       = W * 0.60 * (0.3 + 0.7 * dutyNorm);

        QPolygonF cone;
        cone << QPointF(cx - injW * 0.1, sprayY_start)
             << QPointF(cx + injW * 0.1, sprayY_start)
             << QPointF(cx + sprayW / 2.0, sprayY_end)
             << QPointF(cx - sprayW / 2.0, sprayY_end);

        QLinearGradient sg(cx, sprayY_start, cx, sprayY_end);
        sg.setColorAt(0.0, FUEL_CORE);
        sg.setColorAt(0.4, FUEL_MIST);
        sg.setColorAt(1.0, FUEL_EDGE);
        p.setBrush(sg);
        p.setPen(Qt::NoPen);
        p.drawPolygon(cone);

        // Add soft radial glow at the nozzle tip
        QRadialGradient glow(cx, sprayY_start, injW * 0.8 * dutyNorm);
        glow.setColorAt(0.0, QColor(0xE0, 0xF2, 0xFE, int(200 * dutyNorm)));
        glow.setColorAt(1.0, QColor(0xE0, 0xF2, 0xFE, 0));
        p.setBrush(glow);
        p.drawEllipse(QPointF(cx, sprayY_start), injW * 0.8 * dutyNorm, injW * 0.4 * dutyNorm);
    }

    // ── 2. Injector Body (Cross-section view) ───────────────────────────
    {
        // Main cylindrical housing
        QRectF bodyRect(cx - injW / 2.0, injY, injW, injH * 0.8);
        QLinearGradient bg(bodyRect.topLeft(), bodyRect.topRight());
        bg.setColorAt(0.0, BLACK_RIM);
        bg.setColorAt(0.2, STEEL_DK);
        bg.setColorAt(0.5, STEEL_HI);
        bg.setColorAt(0.8, STEEL_MID);
        bg.setColorAt(1.0, BLACK_RIM);
        p.setBrush(bg);
        p.setPen(QPen(BLACK_RIM, 1.5));
        p.drawRoundedRect(bodyRect, 4, 4);

        // O-Ring (Top)
        QRectF oringTop(cx - injW * 0.55, injY + injH * 0.1, injW * 1.1, injH * 0.15);
        QLinearGradient og(oringTop.topLeft(), oringTop.topRight());
        og.setColorAt(0.0, QColor(0x11,0x11,0x11)); og.setColorAt(0.5, QColor(0x33,0x33,0x33)); og.setColorAt(1.0, QColor(0x05,0x05,0x05));
        p.setBrush(og); p.drawRoundedRect(oringTop, 6, 6);

        // O-Ring (Bottom)
        QRectF oringBot(cx - injW * 0.55, injY + injH * 0.6, injW * 1.1, injH * 0.15);
        p.setBrush(og); p.drawRoundedRect(oringBot, 6, 6);

        // Tapered Nozzle
        QPolygonF nozzle;
        double nzY = injY + injH * 0.8;
        nozzle << QPointF(cx - injW / 2.0, nzY)
               << QPointF(cx + injW / 2.0, nzY)
               << QPointF(cx + injW * 0.2, injY + injH)
               << QPointF(cx - injW * 0.2, injY + injH);
        QLinearGradient nzg(cx - injW / 2.0, nzY, cx + injW / 2.0, nzY);
        nzg.setColorAt(0.0, BLACK_RIM);
        nzg.setColorAt(0.5, STEEL_HI);
        nzg.setColorAt(1.0, BLACK_RIM);
        p.setBrush(nzg);
        p.drawPolygon(nozzle);
    }

    // ── 3. Internal Pintle (Needle) ─────────────────────────────────────
    {
        // Pintle retracts upwards as duty % increases
        // We'll simulate a fast vibration by averaging the position based on duty
        double pintleLift = (m_duty / 100.0) * (injH * 0.08); 
        
        double pw = injW * 0.15;
        double py = injY + injH * 0.2;
        double ph = injH * 0.8 - pintleLift; // Pintle tip sits just inside the nozzle

        QRectF pintle(cx - pw / 2.0, py, pw, ph);
        QLinearGradient pg(pintle.topLeft(), pintle.topRight());
        pg.setColorAt(0.0, STEEL_DKR);
        pg.setColorAt(0.5, STEEL_HI);
        pg.setColorAt(1.0, STEEL_DKR);
        p.setBrush(pg);
        p.setPen(Qt::NoPen);
        p.drawRect(pintle);

        // Pintle head (tapered tip)
        QPolygonF tip;
        tip << QPointF(cx - pw / 2.0, py + ph)
            << QPointF(cx + pw / 2.0, py + ph)
            << QPointF(cx, py + ph + pw);
        p.drawPolygon(tip);
    }

    // ── 4. Digital Readout ──────────────────────────────────────────────
    {
        p.setFont(QFont("JetBrains Mono", int(W * 0.10), QFont::Bold));
        QString valTxt = QString("%1%").arg(m_duty, 0, 'f', 1);

        QColor tc = (m_duty > 85.0) ? QColor(0xEF, 0x44, 0x44) : QColor(0x38, 0xBD, 0xF8);

        // Shadow
        p.setPen(QColor(0, 0, 0, 120));
        QRectF tr(1, H * 0.85, W, H * 0.12);
        p.drawText(tr, Qt::AlignCenter, valTxt);

        // Main text
        p.setPen(tc);
        tr.translate(-1, -1);
        p.drawText(tr, Qt::AlignCenter, valTxt);

        // Tiny label
        p.setFont(QFont("Inter", int(W * 0.05)));
        p.setPen(STEEL_MID);
        QRectF lr(0, H * 0.94, W, H * 0.06);
        p.drawText(lr, Qt::AlignCenter, "DUTY CYCLE");
    }
}
