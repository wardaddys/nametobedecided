#ifndef TEMPSENSORWIDGET_H
#define TEMPSENSORWIDGET_H

#include <QWidget>
#include <QColor>

/**
 * @brief 3D thermometer widget for the CLT / IAT calibration panels.
 *
 * Renders a skeuomorphic cylindrical temperature sensor probe with:
 *  - Glass-tube body with metallic end fittings
 *  - Liquid column that rises from -40°C to +215°C
 *  - Colour-coded fluid: blue (cold) → green (normal) → red (hot)
 *  - Glowing scale ticks along the tube
 *  - Digital °C readout
 *
 * Call setTemp(double degC) to drive the animation.
 * Call setAccentColor() to switch between IAT (blue) and CLT (orange) themes.
 * Core functionality is untouched — this is purely a QPainter widget.
 */
class TempSensorWidget : public QWidget {
    Q_OBJECT
public:
    explicit TempSensorWidget(QWidget *parent = nullptr);

    /** Set the current temperature in °C (range -40 to 215). Triggers repaint. */
    void setTemp(double degC);

    /** Choose the sensor theme: true = CLT (warm orange), false = IAT (cool cyan). */
    void setCltMode(bool isClt);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    double m_temp   = 20.0;
    bool   m_isClt  = true;
};

#endif // TEMPSENSORWIDGET_H
