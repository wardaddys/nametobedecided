#include "MapSensorWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QConicalGradient>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QtMath>
#include <algorithm>
#include <cmath>

// ============================================================================
//  Palette
// ============================================================================
namespace {
// Housing / steel
const QColor STEEL_HI   (0xE8, 0xEC, 0xF4);
const QColor STEEL_MID  (0xA0, 0xA8, 0xB8);
const QColor STEEL_LO   (0x62, 0x6A, 0x7A);
const QColor STEEL_DK   (0x38, 0x3E, 0x4A);
const QColor STEEL_DKR  (0x1C, 0x1F, 0x26);
const QColor BLACK_RIM  (0x08, 0x0A, 0x0E);
const QColor FACE_BG    (0x0A, 0x0C, 0x12);

// Arc colours
const QColor ARC_VAC    (0x22, 0xD3, 0xEE);   // cyan   — vacuum
const QColor ARC_ATM    (0x22, 0xC5, 0x5E);   // green  — atmospheric
const QColor ARC_BOOST  (0xF5, 0x9E, 0x0B);   // amber  — boost

// Needle
const QColor NEEDLE_HI  (0xFF, 0xFF, 0xFF);
const QColor NEEDLE_LO  (0xE8, 0x30, 0x30);

inline double rad(double deg) { return deg * M_PI / 180.0; }

// Map kPa range [10, 300] → needle angle [-135°, +135°] (starts bottom-left, ends bottom-right)
inline double kpaToAngle(double kpa) {
    double norm = (std::max(10.0, std::min(300.0, kpa)) - 10.0) / (300.0 - 10.0);
    return -135.0 + norm * 270.0;
}
} // namespace

MapSensorWidget::MapSensorWidget(QWidget *parent) : QWidget(parent) {
    setMinimumSize(240, 240);
}

void MapSensorWidget::setKpa(double kpa) {
    kpa = std::max(0.0, std::min(350.0, kpa));
    if (qFuzzyCompare(kpa + 1.0, m_kpa + 1.0)) return;
    m_kpa = kpa;
    update();
}

void MapSensorWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const double side = std::min(width(), height());
    const QPointF c(width() / 2.0, height() / 2.0);
    const double R     = side * 0.44;   // outer housing radius
    const double Rrim  = R * 0.88;      // inner machined rim
    const double Rface = R * 0.76;      // dial face radius

    // ── 1. Mounting ears ───────────────────────────────────────────────────
    const double earAngles[4] = { -45.0, 45.0, -135.0, 135.0 };
    const double earR    = R * 1.05;
    const double earSize = R * 0.22;
    const double earHole = earSize * 0.40;
    for (double a : earAngles) {
        QPointF ec(c.x() + earR * std::sin(rad(a)), c.y() - earR * std::cos(rad(a)));
        QRectF er(ec.x() - earSize / 2, ec.y() - earSize / 2, earSize, earSize);
        QLinearGradient g(er.topLeft(), er.bottomRight());
        g.setColorAt(0.0, STEEL_HI);   g.setColorAt(1.0, STEEL_LO);
        p.setBrush(g); p.setPen(Qt::NoPen); p.drawEllipse(er);
        QRectF hr(ec.x() - earHole / 2, ec.y() - earHole / 2, earHole, earHole);
        QRadialGradient hg(ec, earHole); hg.setColorAt(0, QColor(0,0,0)); hg.setColorAt(1, BLACK_RIM);
        p.setBrush(hg); p.drawEllipse(hr);
    }

    // ── 2. Outer housing ring (conical sheen) ────────────────────────────
    {
        QRectF outer(c.x() - R, c.y() - R, 2*R, 2*R);
        QConicalGradient cg(c, 90);
        cg.setColorAt(0.00, STEEL_LO);   cg.setColorAt(0.20, STEEL_MID);
        cg.setColorAt(0.38, STEEL_HI);   cg.setColorAt(0.55, STEEL_LO);
        cg.setColorAt(0.72, STEEL_DK);   cg.setColorAt(0.88, STEEL_MID);
        cg.setColorAt(1.00, STEEL_LO);
        p.setBrush(cg); p.setPen(Qt::NoPen); p.drawEllipse(outer);
    }

    // ── 3. Machined rim (dark inset ring) ───────────────────────────────
    {
        QRectF rim(c.x() - Rrim, c.y() - Rrim, 2*Rrim, 2*Rrim);
        QLinearGradient g(rim.topLeft(), rim.bottomRight());
        g.setColorAt(0, STEEL_DKR); g.setColorAt(1, STEEL_DK);
        p.setBrush(g); p.setPen(Qt::NoPen); p.drawEllipse(rim);
    }

    // ── 4. Dial face ────────────────────────────────────────────────────
    {
        QRectF face(c.x() - Rface, c.y() - Rface, 2*Rface, 2*Rface);
        QRadialGradient rg(c + QPointF(0, -Rface * 0.3), Rface * 1.4);
        rg.setColorAt(0.0, QColor(0x12, 0x16, 0x1E));
        rg.setColorAt(1.0, FACE_BG);
        p.setBrush(rg); p.setPen(Qt::NoPen); p.drawEllipse(face);
    }

    // ── 5. Scale arc (colour-coded) ─────────────────────────────────────
    {
        const double arcR = Rface * 0.82;
        const int kpaMin = 10, kpaAtm = 101, kpaMax = 300;

        auto drawArcSegment = [&](double kpaFrom, double kpaTo, QColor col) {
            double a1 = -135.0 + ((kpaFrom - kpaMin) / double(kpaMax - kpaMin)) * 270.0;
            double a2 = -135.0 + ((kpaTo   - kpaMin) / double(kpaMax - kpaMin)) * 270.0;
            QPainterPath arc;
            arc.arcMoveTo(c.x() - arcR, c.y() - arcR, 2*arcR, 2*arcR, -a1 + 90);
            arc.arcTo   (c.x() - arcR, c.y() - arcR, 2*arcR, 2*arcR, -a1 + 90, -(a2 - a1));
            QPen pen(col, side * 0.030, Qt::SolidLine, Qt::FlatCap);
            p.setPen(pen); p.setBrush(Qt::NoBrush); p.drawPath(arc);
        };
        drawArcSegment(kpaMin, kpaAtm, ARC_VAC);
        drawArcSegment(kpaAtm, kpaMax, ARC_BOOST);
    }

    // ── 6. Scale ticks + labels ──────────────────────────────────────────
    {
        const double tickOuter = Rface * 0.80;
        const int steps[] = { 0, 50, 101, 150, 200, 250, 300 };
        p.setFont(QFont("JetBrains Mono", int(side * 0.038), QFont::Bold));
        for (int kpa : steps) {
            double norm = (kpa - 10.0) / (300.0 - 10.0);
            double angDeg = -135.0 + norm * 270.0;
            double ar = rad(angDeg);
            double sinA = std::sin(ar), cosA = std::cos(ar);

            double innerT = tickOuter - side * 0.055;
            QPointF t1(c.x() + innerT * sinA, c.y() - innerT * cosA);
            QPointF t2(c.x() + tickOuter * sinA, c.y() - tickOuter * cosA);
            p.setPen(QPen(STEEL_MID, 1.5));
            p.drawLine(t1, t2);

            // label
            double labelR = tickOuter - side * 0.10;
            QPointF lp(c.x() + labelR * sinA, c.y() - labelR * cosA);
            QString txt = (kpa == 101) ? "ATM" : QString::number(kpa);
            QColor lcol = (kpa == 101) ? ARC_ATM : STEEL_MID;
            p.setPen(lcol);
            QRectF lr(lp.x() - 20, lp.y() - 8, 40, 16);
            p.drawText(lr, Qt::AlignCenter, txt);
        }
    }

    // ── 7. Needle ───────────────────────────────────────────────────────
    {
        double angDeg = kpaToAngle(m_kpa);
        double ar = rad(angDeg);
        double sinA = std::sin(ar), cosA = std::cos(ar);

        // Glow under needle tip
        double tipR = Rface * 0.70;
        QPointF tip(c.x() + tipR * sinA, c.y() - tipR * cosA);
        QRadialGradient glow(tip, side * 0.06);
        glow.setColorAt(0.0, QColor(0xFF, 0x44, 0x44, 180));
        glow.setColorAt(1.0, QColor(0xFF, 0x44, 0x44, 0));
        p.setBrush(glow); p.setPen(Qt::NoPen);
        p.drawEllipse(tip, side * 0.06, side * 0.06);

        // Needle body (tapered trapezoid)
        double baseW = side * 0.013;
        double tipW  = side * 0.004;
        double baseR = Rface * 0.05;
        QPointF baseL(c.x() + baseR * sinA - baseW * cosA, c.y() - baseR * cosA - baseW * sinA);
        QPointF baseR2(c.x() + baseR * sinA + baseW * cosA, c.y() - baseR * cosA + baseW * sinA);
        QPointF tipL( c.x() + tipR * sinA  - tipW  * cosA, c.y() - tipR * cosA  - tipW  * sinA);
        QPointF tipR3(c.x() + tipR * sinA  + tipW  * cosA, c.y() - tipR * cosA  + tipW  * sinA);
        QPolygonF needle;
        needle << baseL << baseR2 << tipR3 << tipL;
        QLinearGradient ng(c, tip);
        ng.setColorAt(0.0, NEEDLE_HI);
        ng.setColorAt(1.0, NEEDLE_LO);
        p.setBrush(ng);
        p.setPen(Qt::NoPen);
        p.drawPolygon(needle);

        // Centre pivot boss
        QRectF boss(c.x() - side*0.045, c.y() - side*0.045, side*0.09, side*0.09);
        QRadialGradient bg(c + QPointF(-side*0.015, -side*0.015), side*0.06);
        bg.setColorAt(0.0, STEEL_HI); bg.setColorAt(1.0, STEEL_DKR);
        p.setBrush(bg); p.drawEllipse(boss);
        QRectF cap(c.x() - side*0.018, c.y() - side*0.018, side*0.036, side*0.036);
        p.setBrush(QColor(0x10, 0x12, 0x18));
        p.drawEllipse(cap);
    }

    // ── 8. Digital readout ──────────────────────────────────────────────
    {
        QString txt = QString("%1 kPa").arg(m_kpa, 0, 'f', 0);
        p.setFont(QFont("JetBrains Mono", int(side * 0.060), QFont::Bold));

        // Determine colour by zone
        QColor tc = (m_kpa < 95.0) ? ARC_VAC : (m_kpa < 115.0 ? ARC_ATM : ARC_BOOST);
        p.setPen(tc);
        QRectF tr(c.x() - Rface * 0.55, c.y() + Rface * 0.35, Rface * 1.10, side * 0.12);
        p.drawText(tr, Qt::AlignCenter, txt);

        // Units label below
        p.setFont(QFont("Inter", int(side * 0.033)));
        p.setPen(STEEL_LO);
        QRectF ur(c.x() - Rface * 0.5, c.y() + Rface * 0.52, Rface, side * 0.09);
        p.drawText(ur, Qt::AlignCenter, "MANIFOLD PRESSURE");
    }
}
