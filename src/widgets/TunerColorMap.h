#ifndef TUNERCOLORMAP_H
#define TUNERCOLORMAP_H

#include <QColor>

class TunerColorMap {
public:
    // Returns background color for a normalized value 0.0–1.0
    // Uses deep blue → cyan → green → yellow → red
    static QColor valueToColor(double normalizedValue);
    
    // Returns appropriate text color (light or dark) for contrast against background
    static QColor textColorForBackground(const QColor& bg);
    
    // Normalize a value within a min/max range
    static double normalize(double value, double min, double max);
};

#endif // TUNERCOLORMAP_H
