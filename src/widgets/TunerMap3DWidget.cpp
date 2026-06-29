#include "TunerMap3DWidget.h"
#include "TunerColorMap.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVector3D>
#include <QtMath>
#include <algorithm>

TunerMap3DWidget::TunerMap3DWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(320);
    setCursor(Qt::OpenHandCursor);
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(16);
    connect(m_animTimer, &QTimer::timeout, this, &TunerMap3DWidget::onAnimTick);

    QVBoxLayout* vl = new QVBoxLayout(this);
    vl->setContentsMargins(0,0,0,0);
    vl->addStretch();

    QHBoxLayout* tb = new QHBoxLayout();
    tb->setContentsMargins(10,0,10,10);

    auto makeBtn = [&](const QString& label, const QString& id, bool checkable = false) {
        QPushButton* b = new QPushButton(label, this);
        b->setObjectName(id);
        b->setCheckable(checkable);
        b->setStyleSheet(
            "QPushButton { background:#0F1829; color:#7A8FAD; border:1px solid #1A2640;"
            " padding:5px 14px; font-family:'JetBrains Mono'; font-size:11px; }"
            "QPushButton:hover { color:#00E5C8; border-color:#00E5C860; }"
            "QPushButton:checked { background:#00E5C810; color:#00E5C8; border-color:#00E5C8; }");
        return b;
    };

    auto* smBtn   = makeBtn("SMOOTH / FLAT",      "smBtn",  true);
    auto* rvBtn   = makeBtn("RESET VIEW",         "rvBtn",  false);
    auto* syncBtn = makeBtn("SYNC WITH SELECTION","syncBtn",true);

    tb->addWidget(smBtn); tb->addWidget(rvBtn); tb->addWidget(syncBtn); tb->addStretch();
    vl->addLayout(tb);

    connect(smBtn,   &QPushButton::clicked, this, &TunerMap3DWidget::toggleSmoothMode);
    connect(rvBtn,   &QPushButton::clicked, this, &TunerMap3DWidget::resetView);
    connect(syncBtn, &QPushButton::clicked, this, &TunerMap3DWidget::toggleSync);
}

void TunerMap3DWidget::toggleSmoothMode() { m_smoothMode = !m_smoothMode; update(); }
void TunerMap3DWidget::toggleSync()       { m_syncEnabled = !m_syncEnabled; }

void TunerMap3DWidget::resetView() {
    m_tgtAzimuth = 220.0f; m_tgtElevation = 30.0f;
    m_panOffset = {0,0}; m_zoom = 1.0f;
    m_animTimer->start();
}

void TunerMap3DWidget::onAnimTick() {
    float da = m_tgtAzimuth - m_azimuth;
    float de = m_tgtElevation - m_elevation;
    while (da >  180.f) da -= 360.f;
    while (da < -180.f) da += 360.f;
    m_azimuth   += da * 0.15f;
    m_elevation += de * 0.15f;
    if (qAbs(da) < 0.1f && qAbs(de) < 0.1f) {
        m_azimuth = m_tgtAzimuth; m_elevation = m_tgtElevation;
        m_animTimer->stop();
    }
    update();
}

void TunerMap3DWidget::setTableData(const QVector<QVector<double>>& data, double mn, double mx) {
    m_data = data; m_minVal = mn; m_maxVal = mx;
    m_rows = data.size(); m_cols = m_rows > 0 ? data[0].size() : 0;
    m_hovRow = m_hovCol = -1; m_centres.clear();
    update();
}

void TunerMap3DWidget::setAxisLabels(const QStringList& x, const QStringList& z) {
    m_xLabels = x; m_zLabels = z; update();
}



// ── Interaction ───────────────────────────────────────────────────────────────

QPointF TunerMap3DWidget::project(float x, float y, float z) const {
    float az = qDegreesToRadians(m_azimuth);
    float el = qDegreesToRadians(m_elevation);
    float rx  =  x*qCos(az) + z*qSin(az);
    float ry  =  y;
    float rz  = -x*qSin(az) + z*qCos(az);
    float ry2 =  ry*qCos(el) - rz*qSin(el);
    float rz2 = -(ry*qSin(el) + rz*qCos(el));
    float fl  = 600.f * m_zoom;
    float per = fl / (fl + rz2 + 60.f);
    float sc  = qMin(width(), height()) * 0.78f / 20.f;
    return { width()/2.f + rx*per*sc + m_panOffset.x(),
             height()/2.f - ry2*per*sc + m_panOffset.y() };
}

float TunerMap3DWidget::depth(float x, float y, float z) const {
    float az = qDegreesToRadians(m_azimuth);
    float el = qDegreesToRadians(m_elevation);
    float rz  = -x*qSin(az) + z*qCos(az);
    return -(y*qSin(el) + rz*qCos(el));   // camera-space Z (distance away from camera)
}

// ── Floor grid ───────────────────────────────────────────────────────────────

void TunerMap3DWidget::renderFloorGrid(QPainter& p) {
    if (m_rows < 2 || m_cols < 2) return;
    float xSc = 20.f/qMax(m_cols-1,1), zSc = 20.f/qMax(m_rows-1,1), yF = -6.f;
    p.setPen(QPen(QColor(30,50,80,90), 0.5f));
    for (int c = 0; c < m_cols; ++c) {
        float x = c*xSc - 10.f;
        p.drawLine(project(x,yF,-10), project(x,yF,10));
    }
    for (int r = 0; r < m_rows; ++r) {
        float z = r*zSc - 10.f;
        p.drawLine(project(-10,yF,z), project(10,yF,z));
    }
}

// ── Surface ───────────────────────────────────────────────────────────────────

void TunerMap3DWidget::renderSurface(QPainter& p) {
    if (m_rows < 2 || m_cols < 2) return;

    bool isFlat = (m_maxVal - m_minVal) < 0.5;
    float xSc = 20.f/qMax(m_cols-1,1), zSc = 20.f/qMax(m_rows-1,1), yH = 12.f;

    auto norm = [&](double v) -> float {
        if (isFlat) return 0.5f;
        return float((v - m_minVal) / qMax(m_maxVal - m_minVal, 0.001));
    };

    struct Quad { QPointF p[4]; QColor fill; float sortZ; bool sel; int r,c; };
    QVector<Quad> quads;
    quads.reserve((m_rows-1)*(m_cols-1));
    m_centres.clear();

    for (int row = 0; row < m_rows-1; ++row) {
        for (int col = 0; col < m_cols-1; ++col) {
            // v_RC: R=row offset, C=col offset
            double v00 = m_data[row  ][col  ]; // (x0,z0)
            double v01 = m_data[row  ][col+1]; // (x1,z0) same row, next col
            double v10 = m_data[row+1][col  ]; // (x0,z1) next row, same col
            double v11 = m_data[row+1][col+1]; // (x1,z1)

            float x0=col*xSc-10.f, x1=(col+1)*xSc-10.f;
            float z0=row*zSc-10.f, z1=(row+1)*zSc-10.f;

            // BUG-1 FIX: heights match their XZ positions
            float y00=norm(v00)*yH-yH/2.f; // (x0,z0)
            float y01=norm(v01)*yH-yH/2.f; // (x1,z0)
            float y10=norm(v10)*yH-yH/2.f; // (x0,z1)
            float y11=norm(v11)*yH-yH/2.f; // (x1,z1)

            double aN = (norm(v00)+norm(v01)+norm(v10)+norm(v11))/4.0;
            double colorN = isFlat
                ? (v00-m_minVal)/qMax(m_maxVal-m_minVal+0.001,0.001)
                : aN;
            QColor fill = TunerColorMap::valueToColor(colorN);

            // BUG-2 FIX: correct edge vectors for face normal
            QVector3D a(x1-x0, y01-y00, 0.f);       // edge along X (Z constant)
            QVector3D b(0.f,   y10-y00, z1-z0);     // edge along Z (X constant)
            QVector3D n = QVector3D::crossProduct(b, a).normalized();
            QVector3D L(0.4f, 1.f, 0.6f); L.normalize();
            float diff = qBound(0.4f, QVector3D::dotProduct(n,L)*0.6f+0.70f, 1.f);

            fill = QColor(qBound(0,int(fill.red()*diff),255),
                          qBound(0,int(fill.green()*diff),255),
                          qBound(0,int(fill.blue()*diff),255), 255);

            bool sel = (m_selRow>=0 && m_selCol>=0 &&
                        (row==m_selRow||row==m_selRow-1) &&
                        (col==m_selCol||col==m_selCol-1));

            float cx=(x0+x1)/2.f, czw=(z0+z1)/2.f, cyw=(y00+y01+y10+y11)/4.f;

            Quad q;
            // BUG-1 FIX: correct vertex assignment
            q.p[0]=project(x0,y00,z0); // (x0,z0)
            q.p[1]=project(x1,y01,z0); // (x1,z0) ← was y10
            q.p[2]=project(x1,y11,z1); // (x1,z1)
            q.p[3]=project(x0,y10,z1); // (x0,z1) ← was y01
            q.fill=fill;
            q.sortZ=depth(cx,cyw,czw); // BUG-3 FIX: camera-Z sort
            q.sel=sel; q.r=row; q.c=col;
            quads.append(q);

            m_centres.append({project(cx,cyw,czw), row, col});
        }
    }

    // BUG-3 FIX: back-to-front by camera depth
    std::sort(quads.begin(), quads.end(),
              [](const Quad& a, const Quad& b){ return a.sortZ > b.sortZ; });

    p.setRenderHint(QPainter::Antialiasing, m_smoothMode);
    for (const Quad& q : quads) {
        QPolygonF poly; for (int i=0;i<4;++i) poly<<q.p[i];
        p.setBrush(q.fill);
        p.setPen(m_smoothMode ? Qt::NoPen : QPen(QColor(0,0,0,55), 0.8f));
        p.drawPolygon(poly);

        if (q.r==m_hovRow && q.c==m_hovCol) {
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(255,255,255,130), 1.5f));
            p.drawPolygon(poly);
        }
        if (q.sel) {
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor("#00E5C8"), 2.5f));
            p.drawPolygon(poly);
        }
    }

    // Flat-table label
    if (isFlat) {
        p.setRenderHint(QPainter::Antialiasing, true);
        QFont f("JetBrains Mono",11,QFont::Bold);
        p.setFont(f);
        p.setPen(QColor(255,255,255,180));
        p.drawText(QRectF(0,height()/2.0-20,width(),40), Qt::AlignCenter,
                   QString("Flat Surface  |  value = %1").arg(double(m_data[0][0]), 0, 'f', 2));
    }

    // Selection pin
    if (m_selRow>=0 && m_selCol>=0 && m_selRow<m_rows && m_selCol<m_cols) {
        float xS=m_selCol*xSc-10.f, zS=m_selRow*zSc-10.f;
        double v=m_data[m_selRow][m_selCol];
        float yB=-yH/2.f, yT=float((double(v)-m_minVal)/qMax(m_maxVal-m_minVal,0.001))*yH-yH/2.f;
        p.setRenderHint(QPainter::Antialiasing,true);
        p.setPen(QPen(QColor("#00E5C8"),1.5f,Qt::DashLine));
        p.drawLine(project(xS,yB,zS), project(xS,yT+1.5f,zS));
        QPointF ph=project(xS,yT+1.5f,zS);
        p.setBrush(QColor("#00E5C8")); p.setPen(Qt::NoPen);
        p.drawEllipse(ph,4.0,4.0);
    }
}

// ── Axes ─────────────────────────────────────────────────────────────────────

void TunerMap3DWidget::renderAxes(QPainter& p) {
    p.setRenderHint(QPainter::Antialiasing, true);
    float yF=-6.f;
    float xSc=20.f/qMax(m_cols-1,1), zSc=20.f/qMax(m_rows-1,1);

    auto drawAxisLine = [&](QColor col, QPointF a, QPointF b) {
        p.setPen(QPen(col, 2.f)); p.drawLine(a, b);
    };

    // X (RPM)
    drawAxisLine(QColor("#00E5C8"), project(-10,yF,-10), project(10,yF,-10));
    { QPointF lp=project(11.5f,yF,-10);
      p.setPen(QColor("#9ECFE0")); p.setFont(QFont("JetBrains Mono",9,QFont::Bold));
      p.drawText(QRectF(lp.x(),lp.y()-10,64,20), Qt::AlignLeft|Qt::AlignVCenter,"RPM →"); }

    // Z (Load)
    drawAxisLine(QColor("#9B59F5"), project(-10,yF,-10), project(-10,yF,10));
    { QPointF lp=project(-10,yF,11.5f);
      p.setPen(QColor("#B89EF0")); p.setFont(QFont("JetBrains Mono",9,QFont::Bold));
      p.drawText(QRectF(lp.x(),lp.y()-10,72,20), Qt::AlignLeft|Qt::AlignVCenter,"Load →"); }

    // Y (Value)
    drawAxisLine(QColor("#F5A623"), project(-10,yF,-10), project(-10,6,  -10));
    { p.setPen(QColor("#F5C87A")); p.setFont(QFont("JetBrains Mono",9,QFont::Bold));
      QPointF lp=project(-10,8.f,-10);
      p.drawText(QRectF(lp.x()-40,lp.y()-10,60,20),Qt::AlignCenter,"Value"); }

    // Y min/max labels
    p.setFont(QFont("JetBrains Mono",8));
    p.setPen(QColor("#7A8FAD"));
    { QPointF lp=project(-11.5f,yF,-10);
      p.drawText(QRectF(lp.x()-44,lp.y()-8,44,16),Qt::AlignRight|Qt::AlignVCenter,
                 QString::number(m_minVal,'f',1)); }
    { QPointF lp=project(-11.5f,6.f,-10);
      p.drawText(QRectF(lp.x()-44,lp.y()-8,44,16),Qt::AlignRight|Qt::AlignVCenter,
                 QString::number(m_maxVal,'f',1)); }

    // X tick labels
    if (!m_xLabels.isEmpty()) {
        int step=qMax(1,m_cols/8);
        p.setFont(QFont("JetBrains Mono",7)); p.setPen(QColor("#5A6E8A"));
        for (int c=0; c<m_cols; c+=step) {
            float x=c*xSc-10.f;
            p.drawLine(project(x,yF,-10), project(x,yF-0.6f,-10));
            if (c<m_xLabels.size()) {
                QPointF lp=project(x,yF-1.4f,-10);
                p.drawText(QRectF(lp.x()-20,lp.y()-8,40,16),Qt::AlignCenter,m_xLabels[c]);
            }
        }
    }

    // Z tick labels
    if (!m_zLabels.isEmpty()) {
        int step=qMax(1,m_rows/8);
        p.setFont(QFont("JetBrains Mono",7)); p.setPen(QColor("#5A6E8A"));
        for (int r=0; r<m_rows; r+=step) {
            float z=r*zSc-10.f;
            p.drawLine(project(-10,yF,z), project(-10.6f,yF,z));
            if (r<m_zLabels.size()) {
                QPointF lp=project(-11.2f,yF,z);
                p.drawText(QRectF(lp.x()-44,lp.y()-8,44,16),Qt::AlignRight|Qt::AlignVCenter,m_zLabels[r]);
            }
        }
    }
}

// ── Hover tooltip ─────────────────────────────────────────────────────────────

void TunerMap3DWidget::renderHover(QPainter& p) {
    if (m_hovRow<0||m_hovCol<0||m_hovRow>=m_rows||m_hovCol>=m_cols) return;
    double v=m_data[m_hovRow][m_hovCol];
    QString txt=QString("R%1 C%2: %3").arg(m_hovRow).arg(m_hovCol).arg(v,0,'f',2);
    if (!m_xLabels.isEmpty()&&m_hovCol<m_xLabels.size())
        txt=QString("%1 kPa / %2 RPM: %3")
            .arg(m_zLabels.value(m_hovRow,"?"))
            .arg(m_xLabels.value(m_hovCol,"?"))
            .arg(v,0,'f',2);

    p.setRenderHint(QPainter::Antialiasing,true);
    QFont f("JetBrains Mono",10); p.setFont(f);
    QFontMetrics fm(f);
    int tw=fm.horizontalAdvance(txt)+16, th=fm.height()+10;
    int tx=qBound(4, (int)m_lastMouse.x()-tw/2, width()-tw-4);
    int ty=qBound(4, (int)m_lastMouse.y()-th-8, height()-th-4);
    p.setBrush(QColor(15,24,44,210));
    p.setPen(QPen(QColor("#00E5C8"),1));
    p.drawRoundedRect(QRect(tx,ty,tw,th),5,5);
    p.setPen(Qt::white);
    p.drawText(QRect(tx,ty,tw,th),Qt::AlignCenter,txt);
}

// ── Paint ─────────────────────────────────────────────────────────────────────

void TunerMap3DWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    // Background
    p.fillRect(rect(), QColor("#03050D"));
    QRadialGradient g(width()/2, height()/2, qMin(width(),height())*0.55f);
    g.setColorAt(0, QColor(0,229,200,14)); g.setColorAt(1,Qt::transparent);
    p.fillRect(rect(), g);

    if (m_data.isEmpty()) {
        p.setPen(QColor("#3D5070"));
        p.setFont(QFont("JetBrains Mono",13));
        p.drawText(rect(), Qt::AlignCenter, "Select a table to view 3D surface");
        return;
    }

    renderFloorGrid(p);
    renderSurface(p);
    renderAxes(p);
    renderHover(p);

    // Mode badge
    p.setPen(QColor("#3D5070"));
    p.setFont(QFont("JetBrains Mono",9));
    p.drawText(QPoint(12,18), m_smoothMode ? "● SMOOTH" : "○ FLAT");

    // Draw subtle border around the widget
    p.setPen(QPen(QColor("#333"), 1.0f));
    p.setBrush(Qt::NoBrush);
    p.drawRect(rect().adjusted(0, 0, -1, -1));
}

// ── Mouse events ──────────────────────────────────────────────────────────────

void TunerMap3DWidget::mousePressEvent(QMouseEvent* e) {
    m_lastMouse = e->pos(); m_dragging = true;
    setCursor(Qt::ClosedHandCursor);
}

void TunerMap3DWidget::mouseMoveEvent(QMouseEvent* e) {
    QPoint prev = m_lastMouse;
    m_lastMouse = e->pos();
    if (m_dragging) {
        QPoint d = e->pos() - prev;
        if (e->buttons() & Qt::LeftButton) {
            m_azimuth   += d.x() * 0.4f;
            m_elevation  = qBound(-89.f, m_elevation + d.y()*0.3f, 89.f);
        } else if (e->buttons() & Qt::RightButton) {
            m_panOffset += QPointF(d.x(), d.y());
        }
        update();
    } else {
        // Hover: find nearest quad centre
        int bestR=-1, bestC=-1; float bestD=900.f;
        for (const auto& qc : m_centres) {
            float dx=qc.screen.x()-e->pos().x(), dy=qc.screen.y()-e->pos().y();
            float d2=dx*dx+dy*dy;
            if (d2<bestD) { bestD=d2; bestR=qc.row; bestC=qc.col; }
        }
        if (bestD < 2500.f) { // within 50px
            if (m_hovRow!=bestR||m_hovCol!=bestC) { m_hovRow=bestR; m_hovCol=bestC; update(); }
        } else {
            if (m_hovRow>=0) { m_hovRow=m_hovCol=-1; update(); }
        }
    }
}

void TunerMap3DWidget::mouseReleaseEvent(QMouseEvent*) {
    m_dragging = false; setCursor(Qt::OpenHandCursor);
}

void TunerMap3DWidget::wheelEvent(QWheelEvent* e) {
    m_zoom = qBound(0.3f, m_zoom + e->angleDelta().y()/120.f*0.1f, 4.f);
    update();
}

void TunerMap3DWidget::mouseDoubleClickEvent(QMouseEvent*) { resetView(); }
