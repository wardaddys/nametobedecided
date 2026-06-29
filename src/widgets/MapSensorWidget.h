#ifndef MAPSENSORWIDGET_H
#define MAPSENSORWIDGET_H

#include <QWidget>

/**
 * @brief 3D pressure dial widget for the MAP calibration panel.
 *
 * Renders a skeuomorphic round vacuum/pressure gauge with:
 *  - Machined steel housing with conical rim lighting
 *  - Sweeping needle from vacuum (10 kPa) to boost (300 kPa)
 *  - Colour-coded arc: cyan (vacuum) → green (atmospheric) → amber (boost)
 *  - Glowing needle tip + digital kPa readout
 *
 * Call setKpa(double) to drive the animation.
 * Core functionality is untouched — this is purely a QPainter widget.
 */
class MapSensorWidget : public QWidget {
    Q_OBJECT
public:
    explicit MapSensorWidget(QWidget *parent = nullptr);

    /** Set the current MAP value in kPa (range 0–300). Triggers a repaint. */
    void setKpa(double kpa);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    double m_kpa = 0.0;
};

#endif // MAPSENSORWIDGET_H
