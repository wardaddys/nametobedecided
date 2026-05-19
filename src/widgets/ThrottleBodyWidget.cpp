#include "ThrottleBodyWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QLinearGradient>
#include <QConicalGradient>
#include <QtMath>
#include <algorithm>

namespace {

// Palette mirrors the React reference (tps-calibration-modal.tsx).
constexpr QColor CYAN()       { return QColor(0x22, 0xD3, 0xEE); }
constexpr QColor CYAN_DIM()   { return QColor(0x26, 0x2C, 0x35); }
constexpr QColor BRASS_HI()   { return QColor(0xFF, 0xD7, 0x00); }
constexpr QColor BRASS_MID()  { return QColor(0xB8, 0x86, 0x0B); }
constexpr QColor BRASS_LO()   { return QColor(0x5C, 0x43, 0x05); }
constexpr QColor BRASS_DARK() { return QColor(0x1A, 0x12, 0x01); }
constexpr QColor STEEL_HI()   { return QColor(0xE5, 0xE9, 0xF0); }
constexpr QColor STEEL_MID()  { return QColor(0x9C, 0xA5, 0xB2); }
constexpr QColor STEEL_LO()   { return QColor(0x6B, 0x72, 0x80); }
constexpr QColor STEEL_DK()   { return QColor(0x3A, 0x3F, 0x47); }
constexpr QColor STEEL_DKR()  { return QColor(0x1F, 0x22, 0x28); }
constexpr QColor BLACK_RIM()  { return QColor(0x0B, 0x0D, 0x11); }
constexpr QColor BORE_BLACK() { return QColor(0x03, 0x04, 0x05); }

// Helper: angle in degrees -> radians.
inline double rad(double deg) { return deg * M_PI / 180.0; }

} // namespace

ThrottleBodyWidget::ThrottleBodyWidget(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setMinimumSize(260, 260);
}

void ThrottleBodyWidget::setTPS(double pct) {
    pct = std::max(0.0, std::min(100.0, pct));
    if (qFuzzyCompare(pct + 1.0, m_tps + 1.0)) return;
    m_tps = pct;
    update();
}

void ThrottleBodyWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // Layout: square, centered.
    const int side  = std::min(width(), height());
    const QPointF c(width() / 2.0, height() / 2.0);
    const double  S = side;                      // overall canvas size
    const double  R = S * 0.275;                 // outer housing radius
    const double  Rrim = R * 0.86;               // machined rim radius
    const double  Rbore = R * 0.755;             // bore inner radius
    const double  angDeg = (m_tps / 100.0) * 80.0; // plate tilt

    // ---------- 1. Side modules (sensor / spring-pulley / idle valve) ----------

    // TPS sensor block (left)
    {
        const double w = S * 0.105;
        const double h = S * 0.215;
        const QRectF body(c.x() - R - w * 0.55, c.y() - h * 0.5, w, h);
        QLinearGradient g(body.topLeft(), body.bottomRight());
        g.setColorAt(0.0, QColor(0x2A, 0x2F, 0x35));
        g.setColorAt(1.0, QColor(0x15, 0x18, 0x1C));
        p.setBrush(g);
        p.setPen(QPen(BLACK_RIM(), 1));
        p.drawRoundedRect(body, 4, 4);
        // ribbed connector face
        p.setPen(QPen(BLACK_RIM(), 1));
        for (int i = 0; i < 4; ++i) {
            double y = body.top() + body.height() * (0.22 + i * 0.18);
            p.drawLine(QPointF(body.right() - body.width() * 0.42, y),
                       QPointF(body.right() - body.width() * 0.12, y));
        }
        // small bright metal pin block butting up to housing
        QRectF pin(body.right() - 1, c.y() - h * 0.28, w * 0.30, h * 0.56);
        QLinearGradient pg(pin.topLeft(), pin.bottomLeft());
        pg.setColorAt(0.0, STEEL_LO());
        pg.setColorAt(1.0, STEEL_DK());
        p.setBrush(pg);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(pin, 2, 2);
    }

    // Spring/pulley assembly (right)
    {
        const double w = S * 0.085;
        const double h = S * 0.18;
        // bright pulley pad
        QRectF pad(c.x() + R - w * 0.05, c.y() - h * 0.5, w * 0.6, h);
        QLinearGradient g(pad.topLeft(), pad.bottomLeft());
        g.setColorAt(0.0, STEEL_HI());
        g.setColorAt(1.0, STEEL_LO());
        p.setBrush(g);
        p.setPen(QPen(BLACK_RIM(), 1));
        p.drawRoundedRect(pad, 2, 2);
        // ribbed pulley wheel (vertical stripes)
        QRectF wheel(pad.right(), pad.top() + h * 0.08, w * 0.9, h * 0.7);
        p.setPen(Qt::NoPen);
        for (double x = wheel.left(); x < wheel.right(); x += 3.0) {
            double t = (x - wheel.left()) / wheel.width();
            QColor col = (static_cast<int>((x - wheel.left()) / 1.5) % 2 == 0)
                            ? QColor(0x15, 0x18, 0x1C) : STEEL_LO();
            p.setBrush(col);
            p.drawRect(QRectF(x, wheel.top(), 1.5, wheel.height()));
            Q_UNUSED(t);
        }
        // bracket arm extending up/back
        QRectF bracket(wheel.right(), pad.top() - h * 0.05, w * 0.45, h * 1.05);
        QLinearGradient bg(bracket.topLeft(), bracket.topRight());
        bg.setColorAt(0.0, STEEL_DK());
        bg.setColorAt(1.0, QColor(0x15, 0x18, 0x1C));
        p.setBrush(bg);
        p.setPen(QPen(BLACK_RIM(), 1));
        p.drawRoundedRect(bracket, 2, 2);
    }

    // Idle valve (bottom)
    {
        const double w = S * 0.12;
        const double h = S * 0.08;
        QRectF body(c.x() - w * 0.5, c.y() + R + h * 0.05, w, h);
        QLinearGradient g(body.topLeft(), body.topRight());
        g.setColorAt(0.0, STEEL_LO());
        g.setColorAt(1.0, STEEL_DK());
        p.setBrush(g);
        p.setPen(QPen(BLACK_RIM(), 1));
        p.drawRoundedRect(body, 3, 6);
        // brass nipple
        QRectF nip(body.right() - 1, body.top() + h * 0.25, w * 0.4, h * 0.5);
        QLinearGradient ng(nip.topLeft(), nip.bottomLeft());
        ng.setColorAt(0.0, BRASS_HI());
        ng.setColorAt(0.5, BRASS_MID());
        ng.setColorAt(1.0, BRASS_LO());
        p.setBrush(ng);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(nip, 2, 2);
    }

    // ---------- 2. Mounting ears (5 round flanges around housing) ----------
    const double earAngles[5] = { 0.0, -55.0, 55.0, -125.0, 125.0 };
    const double earR = R * 1.06;
    const double earSize = R * 0.30;
    const double earHole = earSize * 0.40;
    for (double a : earAngles) {
        QPointF ec(c.x() + earR * std::sin(rad(a)),
                   c.y() - earR * std::cos(rad(a)));
        QRectF er(ec.x() - earSize / 2, ec.y() - earSize / 2, earSize, earSize);
        QLinearGradient g(er.topLeft(), er.bottomRight());
        g.setColorAt(0.0, STEEL_HI());
        g.setColorAt(1.0, STEEL_LO());
        p.setBrush(g);
        p.setPen(Qt::NoPen);
        p.drawEllipse(er);
        // hole
        QRectF hr(ec.x() - earHole / 2, ec.y() - earHole / 2, earHole, earHole);
        QRadialGradient hg(ec, earHole * 0.8);
        hg.setColorAt(0.0, QColor(0, 0, 0));
        hg.setColorAt(1.0, BLACK_RIM());
        p.setBrush(hg);
        p.drawEllipse(hr);
    }

    // ---------- 3. Main housing ring (conic-gradient look) ----------
    {
        QRectF outer(c.x() - R, c.y() - R, 2 * R, 2 * R);
        QConicalGradient cg(c, 90);
        cg.setColorAt(0.00, STEEL_LO());
        cg.setColorAt(0.16, STEEL_MID());
        cg.setColorAt(0.33, STEEL_HI());
        cg.setColorAt(0.50, STEEL_LO());
        cg.setColorAt(0.66, QColor(0x4A, 0x50, 0x58));
        cg.setColorAt(0.83, STEEL_MID());
        cg.setColorAt(1.00, STEEL_LO());
        p.setBrush(cg);
        p.setPen(Qt::NoPen);
        p.drawEllipse(outer);
    }

    // ---------- 4. Machined rim (dark ring inside housing) ----------
    {
        QRectF rim(c.x() - Rrim, c.y() - Rrim, 2 * Rrim, 2 * Rrim);
        QLinearGradient g(rim.topLeft(), rim.bottomRight());
        g.setColorAt(0.0, STEEL_DKR());
        g.setColorAt(1.0, STEEL_DK());
        p.setBrush(g);
        p.setPen(Qt::NoPen);
        p.drawEllipse(rim);
    }

    // ---------- 5. Bore (deep black hole) with inset shadow ----------
    QRectF boreRect(c.x() - Rbore, c.y() - Rbore, 2 * Rbore, 2 * Rbore);
    {
        p.setBrush(BORE_BLACK());
        p.setPen(Qt::NoPen);
        p.drawEllipse(boreRect);
        // soft inner shadow from above
        QRadialGradient rg(QPointF(c.x(), c.y() - Rbore * 0.4), Rbore * 1.2);
        rg.setColorAt(0.30, QColor(0, 0, 0, 0));
        rg.setColorAt(0.80, QColor(0, 0, 0, 200));
        rg.setColorAt(1.00, QColor(0, 0, 0, 255));
        p.setBrush(rg);
        p.drawEllipse(boreRect);
    }

    // ---------- 6. Butterfly valve (rotates around horizontal shaft) ----------
    // The plate is a circle of diameter ~ bore. When tilted by `angDeg` around
    // the X axis, its projection onto the page is an ellipse with vertical
    // semi-axis = R * cos(angDeg). We paint that ellipse with a brass gradient
    // whose direction depends on the tilt so it really reads as 3D.
    {
        p.save();
        // clip to the bore so the plate edges don't paint over the rim
        QPainterPath clip;
        clip.addEllipse(boreRect);
        p.setClipPath(clip);

        const double cosA  = std::cos(rad(angDeg));
        const double plateR = Rbore * 0.985;       // slight inset
        const double plateH = plateR * std::max(0.06, cosA); // never fully edge-on

        QRectF plate(c.x() - plateR, c.y() - plateH, 2 * plateR, 2 * plateH);

        // brass face gradient — highlight moves with angle so the closed plate
        // shows a strong top highlight and the open plate looks edge-lit.
        QLinearGradient bg(plate.topLeft(), plate.bottomRight());
        // closed (cosA ~ 1)  -> bright brass face
        // open   (cosA ~ 0)  -> mostly dark edge of the plate
        const double brightness = std::pow(cosA, 0.6); // 0..1
        QColor hi  = BRASS_HI();
        QColor mid = BRASS_MID();
        QColor lo  = BRASS_LO();
        hi.setAlpha(255);
        mid.setAlpha(255);
        lo.setAlpha(255);
        // mix toward dark as plate opens
        auto mix = [](QColor a, QColor b, double t) {
            return QColor(
                static_cast<int>(a.red()   * (1 - t) + b.red()   * t),
                static_cast<int>(a.green() * (1 - t) + b.green() * t),
                static_cast<int>(a.blue()  * (1 - t) + b.blue()  * t));
        };
        QColor faceHi  = mix(BRASS_DARK(), hi,  brightness);
        QColor faceMid = mix(BRASS_DARK(), mid, brightness * 0.85 + 0.05);
        QColor faceLo  = mix(BRASS_DARK(), lo,  brightness * 0.7 + 0.05);

        bg.setColorAt(0.0, faceHi);
        bg.setColorAt(0.5, faceMid);
        bg.setColorAt(1.0, faceLo);
        p.setBrush(bg);
        p.setPen(Qt::NoPen);
        p.drawEllipse(plate);

        // Plate thickness band — the side of the disc visible when tilted.
        // We draw a thin dark ellipse offset upward (positive tilt = top edge
        // recedes, bottom edge comes forward).
        if (angDeg > 1.0) {
            const double thick = plateR * 0.04;
            QRectF edgeTop(plate.left(),
                           plate.top() - thick * std::sin(rad(angDeg)) * 0.6,
                           plate.width(), plate.height());
            QLinearGradient eg(edgeTop.topLeft(), edgeTop.bottomLeft());
            eg.setColorAt(0.0, mix(BRASS_DARK(), faceLo, 0.4));
            eg.setColorAt(1.0, mix(BRASS_DARK(), faceLo, 0.1));
            p.setBrush(eg);
            QPainterPath edgePath;
            edgePath.addEllipse(edgeTop);
            QPainterPath facePath;
            facePath.addEllipse(plate);
            p.drawPath(edgePath.subtracted(facePath));
        }

        // Subtle face highlight (only visible when nearly closed)
        if (brightness > 0.3) {
            QLinearGradient gl(plate.topLeft(), QPointF(plate.center().x(), plate.center().y()));
            gl.setColorAt(0.0, QColor(255, 255, 255, static_cast<int>(70 * brightness)));
            gl.setColorAt(1.0, QColor(255, 255, 255, 0));
            p.setBrush(gl);
            p.drawEllipse(plate);
        }

        // ---------- 7. Central shaft (steel rod across the bore) ----------
        const double shaftH = plateR * 0.115;
        QRectF shaft(c.x() - Rbore, c.y() - shaftH / 2, 2 * Rbore, shaftH);
        QLinearGradient sg(shaft.topLeft(), shaft.bottomLeft());
        sg.setColorAt(0.0, STEEL_DK());
        sg.setColorAt(0.45, STEEL_HI());
        sg.setColorAt(0.55, STEEL_HI());
        sg.setColorAt(1.0, QColor(0x4A, 0x50, 0x58));
        p.setBrush(sg);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(shaft, shaftH / 2, shaftH / 2);

        // Two flat-head screws on the shaft
        const double screwR = shaftH * 0.6;
        for (double dx : { -plateR * 0.45, plateR * 0.45 }) {
            QPointF sc(c.x() + dx, c.y());
            QRadialGradient rg(sc + QPointF(-screwR * 0.3, -screwR * 0.3), screwR * 1.4);
            rg.setColorAt(0.0, QColor(0xD1, 0xD5, 0xDB));
            rg.setColorAt(1.0, QColor(0x4A, 0x50, 0x58));
            p.setBrush(rg);
            p.drawEllipse(sc, screwR, screwR);
            // slot
            p.setPen(QPen(QColor(0x11, 0x11, 0x11), std::max(1.0, screwR * 0.18)));
            QPointF a(sc.x() - screwR * 0.7, sc.y() + screwR * 0.4);
            QPointF b(sc.x() + screwR * 0.7, sc.y() - screwR * 0.4);
            p.drawLine(a, b);
            p.setPen(Qt::NoPen);
        }

        p.restore();
    }

    // ---------- 8. Cyan HUD tick ring (overlay, outside the bore) ----------
    {
        const int ticks = 44;
        const double inner = R * 1.18;
        for (int i = 0; i < ticks; ++i) {
            const double t = static_cast<double>(i) / (ticks - 1);
            const double a = (-90.0 + t * 270.0); // -90 to +180 (start top, sweep clockwise 3/4)
            const double ar = rad(a);
            const double len = (i % 4 == 0) ? S * 0.035 : S * 0.018;
            const QPointF p1(c.x() + inner * std::cos(ar),
                             c.y() + inner * std::sin(ar));
            const QPointF p2(c.x() + (inner + len) * std::cos(ar),
                             c.y() + (inner + len) * std::sin(ar));
            const bool reached = t <= (m_tps / 100.0);
            QPen pen(reached ? CYAN() : CYAN_DIM());
            pen.setCapStyle(Qt::RoundCap);
            pen.setWidthF((i % 4 == 0) ? 1.7 : 1.0);
            p.setPen(pen);
            p.setOpacity(reached ? 0.95 : 0.55);
            p.drawLine(p1, p2);
        }
        p.setOpacity(1.0);

        // Indicator dot
        const double t = m_tps / 100.0;
        const double a = rad(-90.0 + t * 270.0);
        const QPointF ip(c.x() + (inner + S * 0.022) * std::cos(a),
                         c.y() + (inner + S * 0.022) * std::sin(a));
        // outer glow
        QRadialGradient glow(ip, S * 0.035);
        glow.setColorAt(0.0, QColor(0x22, 0xD3, 0xEE, 200));
        glow.setColorAt(1.0, QColor(0x22, 0xD3, 0xEE, 0));
        p.setBrush(glow);
        p.setPen(Qt::NoPen);
        p.drawEllipse(ip, S * 0.030, S * 0.030);
        // dot
        p.setBrush(CYAN());
        p.drawEllipse(ip, S * 0.012, S * 0.012);
        p.setBrush(Qt::white);
        p.drawEllipse(ip, S * 0.005, S * 0.005);
    }
}
