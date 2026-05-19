#ifndef THROTTLEBODYWIDGET_H
#define THROTTLEBODYWIDGET_H

#include <QWidget>

/**
 * ThrottleBodyWidget
 * ------------------
 * Top-down 3D-styled rendering of a mechanical throttle body for the
 * TPS calibration dialog. Painted entirely with QPainter (no OpenGL,
 * no external assets) so it slots into the existing Qt UI without any
 * build-system changes beyond adding this pair of files.
 *
 * The butterfly valve angle is driven by setTPS() in percent (0..100).
 * Internally the plate rotates 0..80 degrees about the horizontal shaft
 * to match the React reference design, and a cyan HUD tick-ring with
 * an indicator dot tracks the same percentage around the bore.
 */
class ThrottleBodyWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(double tps READ tps WRITE setTPS)

public:
    explicit ThrottleBodyWidget(QWidget *parent = nullptr);

    /** Current TPS in percent (0..100). */
    double tps() const { return m_tps; }

    /** Current butterfly plate angle in degrees (0..80). */
    double angle() const { return (m_tps / 100.0) * 80.0; }

public slots:
    /** Set TPS in percent. Triggers a repaint. Clamped to [0,100]. */
    void setTPS(double pct);

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override { return QSize(320, 320); }
    QSize minimumSizeHint() const override { return QSize(260, 260); }

private:
    double m_tps = 0.0;
};

#endif // THROTTLEBODYWIDGET_H
