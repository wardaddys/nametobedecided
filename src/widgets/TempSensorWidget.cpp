#include "TempSensorWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QtMath>
#include <algorithm>
#include <cmath>

// ============================================================================
//  Helpers
// ============================================================================
namespace {

// Steel housing palette (same as MapSensorWidget for visual consistency)
const QColor STEEL_HI  (0xE8, 0xEC, 0xF4);
const QColor STEEL_MID (0xA0, 0xA8, 0xB8);
const QColor STEEL_LO  (0x62, 0x6A, 0x7A);
const QColor STEEL_DK  (0x38, 0x3E, 0x4A);
const QColor STEEL_DKR (0x1C, 0x1F, 0x26);
const QColor BLACK_RIM (0x08, 0x0A, 0x0E);
const QColor GLASS_TUB (0x0E, 0x12, 0x1A);

// Fluid colours — interpolated based on temperature
struct TempColor {
    QColor cold  { 0x22, 0xD3, 0xEE };   // icy cyan   (-40°C)
    QColor cool  { 0x22, 0xC5, 0x5E };   // green      (  0°C)
    QColor warm  { 0xF5, 0x9E, 0x0B };   // amber      ( 90°C)
    QColor hot   { 0xEF, 0x44, 0x44 };   // red        (150°C+)
};

QColor lerpColor(QColor a, QColor b, double t) {
    t = std::max(0.0, std::min(1.0, t));
    return QColor(
        static_cast<int>(a.red()   + t * (b.red()   - a.red())),
        static_cast<int>(a.green() + t * (b.green() - a.green())),
        static_cast<int>(a.blue()  + t * (b.blue()  - a.blue()))
    );
}

QColor fluidColor(double degC) {
    TempColor tc;
    if (degC < 0.0)   return lerpColor(tc.cold, tc.cool, (degC + 40.0) / 40.0);
    if (degC < 90.0)  return lerpColor(tc.cool, tc.warm, degC / 90.0);
    return              lerpColor(tc.warm, tc.hot,  (degC - 90.0) / 60.0);
}

// Normalized fill level: -40°C → 0.0, 215°C → 1.0
inline double fillLevel(double degC) {
    return (std::max(-40.0, std::min(215.0, degC)) + 40.0) / 255.0;
}

} // namespace

// ============================================================================
//  TempSensorWidget
// ============================================================================

TempSensorWidget::TempSensorWidget(QWidget *parent) : QWidget(parent) {
    setMinimumSize(160, 280);
}

void TempSensorWidget::setTemp(double degC) {
    degC = std::max(-40.0, std::min(215.0, degC));
    if (qFuzzyCompare(degC + 1.0, m_temp + 1.0)) return;
    m_temp = degC;
    update();
}

void TempSensorWidget::setCltMode(bool isClt) {
    m_isClt = isClt;
    update();
}

void TempSensorWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const double W = width();
    const double H = height();

    // ── Layout ───────────────────────────────────────────────────────────
    // Central steel probe occupies the vertical middle 70% of the widget
    const double probeW  = W * 0.28;
    const double probeH  = H * 0.72;
    const double probeX  = (W - probeW) / 2.0;
    const double probeY  = H * 0.06;
    const QRectF probeRect(probeX, probeY, probeW, probeH);

    // Glass tube sits inside the probe — narrower
    const double tubeW   = probeW * 0.38;
    const double tubeH   = probeH * 0.78;
    const double tubeX   = (W - tubeW) / 2.0;
    const double tubeY   = probeY + probeH * 0.05;
    const QRectF tubeRect(tubeX, tubeY, tubeW, tubeH);

    // ── 1. Hexagonal probe body (drawn as rounded rectangle with sheen) ──
    {
        QLinearGradient g(probeRect.topLeft(), probeRect.topRight());
        g.setColorAt(0.00, BLACK_RIM);
        g.setColorAt(0.18, STEEL_DKR);
        g.setColorAt(0.38, STEEL_HI);
        g.setColorAt(0.50, STEEL_MID);
        g.setColorAt(0.65, STEEL_LO);
        g.setColorAt(0.85, STEEL_DKR);
        g.setColorAt(1.00, BLACK_RIM);
        p.setBrush(g);
        p.setPen(QPen(BLACK_RIM, 1.2));
        p.drawRoundedRect(probeRect, probeW * 0.20, probeW * 0.20);
    }

    // ── 2. Connector hex head at the top ─────────────────────────────────
    {
        double headH = probeH * 0.14;
        QRectF head(probeX - probeW * 0.12, probeY, probeW * 1.24, headH);
        QLinearGradient g(head.topLeft(), head.topRight());
        g.setColorAt(0.0, BLACK_RIM);  g.setColorAt(0.25, STEEL_DK);
        g.setColorAt(0.50, STEEL_HI); g.setColorAt(0.75, STEEL_MID);
        g.setColorAt(1.0, BLACK_RIM);
        p.setBrush(g);
        p.setPen(QPen(BLACK_RIM, 1.0));
        p.drawRoundedRect(head, 4, 4);

        // Connector wire stub on top
        QRectF wire(W / 2.0 - probeW * 0.09, probeY - probeH * 0.07,
                    probeW * 0.18, probeH * 0.08);
        QLinearGradient wg(wire.topLeft(), wire.topRight());
        wg.setColorAt(0.0, STEEL_DK); wg.setColorAt(0.5, STEEL_MID); wg.setColorAt(1.0, STEEL_DK);
        p.setBrush(wg); p.drawRoundedRect(wire, 3, 3);
    }

    // ── 3. Scale ticks along the side of the probe ───────────────────────
    {
        // 5 major ticks: -40, 0, 50, 100, 150, 200, 215
        struct Tick { double tempC; const char *label; };
        const Tick ticks[] = {
            {-40, "-40"}, {0, "0"}, {50, "50"},
            {100, "100"}, {150, "150"}, {200, "200"}
        };

        p.setFont(QFont("JetBrains Mono", int(W * 0.065)));

        for (auto &tk : ticks) {
            double norm = fillLevel(tk.tempC);
            double ty   = tubeY + tubeH * (1.0 - norm);

            // Tick line to the right of the tube
            double lx1 = tubeX + tubeW + probeW * 0.10;
            double lx2 = lx1 + probeW * 0.40;
            p.setPen(QPen(STEEL_MID, 1.0));
            p.drawLine(QPointF(lx1, ty), QPointF(lx2, ty));

            // Label
            QColor lcol = fluidColor(tk.tempC);
            p.setPen(lcol.lighter(130));
            QRectF lr(lx2 + 3, ty - W * 0.05, W * 0.32, W * 0.10);
            p.drawText(lr, Qt::AlignVCenter | Qt::AlignLeft, tk.label);
        }
    }

    // ── 4. Glass tube background (dark inner bore) ───────────────────────
    {
        QLinearGradient g(tubeRect.topLeft(), tubeRect.topRight());
        g.setColorAt(0.0, BLACK_RIM);
        g.setColorAt(0.20, QColor(0x06, 0x08, 0x0E));
        g.setColorAt(0.80, QColor(0x06, 0x08, 0x0E));
        g.setColorAt(1.0, BLACK_RIM);
        p.setBrush(g);
        p.setPen(QPen(STEEL_DKR, 0.8));
        p.drawRoundedRect(tubeRect, tubeW / 2, tubeW / 2);
    }

    // ── 5. Fluid column inside the tube ─────────────────────────────────
    {
        double level  = fillLevel(m_temp);
        double fluidH = tubeH * level;
        double fluidY = tubeY + tubeH - fluidH;

        if (fluidH > 1.0) {
            QColor fluidTop = fluidColor(m_temp);
            QColor fluidBot = fluidTop.darker(160);

            QRectF fluid(tubeX + tubeW * 0.10, fluidY,
                         tubeW * 0.80, fluidH);

            // Clip to tube shape
            QPainterPath tubePath;
            tubePath.addRoundedRect(tubeRect, tubeW / 2, tubeW / 2);
            p.save();
            p.setClipPath(tubePath);

            QLinearGradient fg(fluid.topLeft(), fluid.bottomLeft());
            fg.setColorAt(0.0, fluidTop);
            fg.setColorAt(1.0, fluidBot);
            p.setBrush(fg);
            p.setPen(Qt::NoPen);
            p.drawRect(fluid);

            // Surface glow at the top of the fluid
            QRectF surface(tubeX + tubeW * 0.05, fluidY - tubeW * 0.25,
                           tubeW * 0.90, tubeW * 0.50);
            QRadialGradient sg(surface.center(), tubeW * 0.5);
            sg.setColorAt(0.0, QColor(fluidTop.red(), fluidTop.green(), fluidTop.blue(), 200));
            sg.setColorAt(1.0, QColor(fluidTop.red(), fluidTop.green(), fluidTop.blue(), 0));
            p.setBrush(sg); p.drawEllipse(surface);

            p.restore();
        }
    }

    // ── 6. Glass tube highlight (left-edge specular reflection) ──────────
    {
        double hx = tubeX + tubeW * 0.08;
        QRectF highlight(hx, tubeY + tubeH * 0.05, tubeW * 0.12, tubeH * 0.80);
        QLinearGradient hg(highlight.topLeft(), highlight.topRight());
        hg.setColorAt(0.0, QColor(255, 255, 255, 60));
        hg.setColorAt(1.0, QColor(255, 255, 255, 0));
        p.setBrush(hg); p.setPen(Qt::NoPen);
        QPainterPath tubePath;
        tubePath.addRoundedRect(tubeRect, tubeW / 2, tubeW / 2);
        p.save(); p.setClipPath(tubePath);
        p.drawRect(highlight);
        p.restore();
    }

    // ── 7. Bulb at the bottom ────────────────────────────────────────────
    {
        double bulbR = probeW * 0.48;
        QPointF bulbC(W / 2.0, tubeY + tubeH + bulbR * 0.5);
        QRectF  bulbRect(bulbC.x() - bulbR, bulbC.y() - bulbR, 2*bulbR, 2*bulbR);

        // Steel collar
        QLinearGradient sg(bulbRect.topLeft(), bulbRect.topRight());
        sg.setColorAt(0.0, BLACK_RIM); sg.setColorAt(0.3, STEEL_MID);
        sg.setColorAt(0.7, STEEL_DK);  sg.setColorAt(1.0, BLACK_RIM);
        p.setBrush(sg); p.setPen(QPen(BLACK_RIM, 1));
        p.drawEllipse(bulbRect);

        // Fluid fill in bulb
        QColor bulbFluid = fluidColor(m_temp);
        QRadialGradient bg(bulbC + QPointF(-bulbR*0.25, -bulbR*0.25), bulbR * 1.1);
        bg.setColorAt(0.0, bulbFluid.lighter(130));
        bg.setColorAt(0.6, bulbFluid);
        bg.setColorAt(1.0, bulbFluid.darker(160));
        p.setBrush(bg); p.drawEllipse(bulbRect.adjusted(3,3,-3,-3));

        // Specular dot
        QRectF spec(bulbC.x() - bulbR*0.30, bulbC.y() - bulbR*0.45,
                    bulbR * 0.35, bulbR * 0.25);
        QRadialGradient sp(spec.center(), bulbR * 0.25);
        sp.setColorAt(0.0, QColor(255,255,255,160)); sp.setColorAt(1.0, QColor(255,255,255,0));
        p.setBrush(sp); p.drawEllipse(spec);
    }

    // ── 8. Digital readout ───────────────────────────────────────────────
    {
        QString label = m_isClt ? "CLT" : "IAT";
        QColor  accent = m_isClt ? QColor(0xF5, 0x9E, 0x0B) : QColor(0x22, 0xD3, 0xEE);
        QColor  fluid  = fluidColor(m_temp);

        p.setFont(QFont("JetBrains Mono", int(W * 0.100), QFont::Bold));
        QString valTxt = QString("%1°C").arg(m_temp, 0, 'f', 1);

        // Shadow
        p.setPen(QColor(0, 0, 0, 120));
        QRectF tr(1, H * 0.885, W, H * 0.12);
        p.drawText(tr, Qt::AlignCenter, valTxt);

        // Main text
        p.setPen(fluid.lighter(140));
        tr.translate(-1, -1);
        p.drawText(tr, Qt::AlignCenter, valTxt);

        // Tiny label
        p.setFont(QFont("Inter", int(W * 0.060)));
        p.setPen(accent);
        QRectF lr(0, H * 0.955, W, H * 0.06);
        p.drawText(lr, Qt::AlignCenter, label + " SENSOR");
    }
}
