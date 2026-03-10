#include "TunerColorMap.h"
#include <QtMath>
#include <algorithm>

QColor TunerColorMap::valueToColor(double normalizedValue) {
    normalizedValue = std::clamp(normalizedValue, 0.0, 1.0);

    struct ColorStop {
        double t;
        int r, g, b;
    };

    const ColorStop stops[] = {
        { 0.00, 10,  22,  40  }, // #0A1628
        { 0.25, 0,   102, 204 }, // #0066CC
        { 0.50, 0,   170, 85  }, // #00AA55
        { 0.75, 255, 184, 0   }, // #FFB800
        { 1.00, 255, 34,  0   }  // #FF2200
    };

    int numStops = sizeof(stops) / sizeof(stops[0]);
    if (normalizedValue <= stops[0].t) {
        return QColor(stops[0].r, stops[0].g, stops[0].b, 200);
    }
    if (normalizedValue >= stops[numStops - 1].t) {
        return QColor(stops[numStops - 1].r, stops[numStops - 1].g, stops[numStops - 1].b, 200);
    }

    // Find the stops it falls between
    for (int i = 0; i < numStops - 1; ++i) {
        if (normalizedValue >= stops[i].t && normalizedValue <= stops[i+1].t) {
            double range = stops[i+1].t - stops[i].t;
            double factor = (normalizedValue - stops[i].t) / range;
            int r = stops[i].r + factor * (stops[i+1].r - stops[i].r);
            int g = stops[i].g + factor * (stops[i+1].g - stops[i].g);
            int b = stops[i].b + factor * (stops[i+1].b - stops[i].b);
            return QColor(r, g, b, 200);
        }
    }
    
    return QColor(0, 0, 0, 200); // Fallback
}

QColor TunerColorMap::textColorForBackground(const QColor& bg) {
    // Calculate perceived luminance: L = 0.299*R + 0.587*G + 0.114*B
    double l = 0.299 * bg.red() + 0.587 * bg.green() + 0.114 * bg.blue();
    if (l > 140) {
        return QColor("#0A0A0A");
    } else {
        return QColor("#FFFFFF");
    }
}

double TunerColorMap::normalize(double value, double min, double max) {
    if (max <= min) return 0.0;
    value = std::clamp(value, min, max);
    return (value - min) / (max - min);
}
