#ifndef DASHBOARDTHEME_H
#define DASHBOARDTHEME_H

#include <QColor>
#include <QString>
#include <QMap>

/**
 * @brief Dashboard theme configuration
 * 
 * Provides color schemes for dashboard elements including gauges,
 * backgrounds, accents, and data graph colors.
 */
struct DashboardTheme {
    QString name;
    
    // Background colors
    QColor bgPrimary;
    QColor bgSecondary;
    QColor bgAccent;
    
    // Gauge colors
    QColor gaugeBorder;
    QColor gaugeBackground;
    QColor gaugeNeedle;
    QColor gaugeText;
    QColor gaugeValue;
    
    // Status colors
    QColor statusGood;
    QColor statusWarning;
    QColor statusDanger;
    QColor statusInactive;
    
    // Graph colors
    QColor graphLine1;  // Primary data line
    QColor graphLine2;  // Secondary data line
    QColor graphGrid;
    QColor graphBackground;
    
    // Accent color (borders, highlights)
    QColor accent;
    QColor accentSecondary;
    
    // Text colors
    QColor textPrimary;
    QColor textSecondary;
    QColor textMuted;
};

/**
 * @brief Theme manager providing preset themes
 */
class DashboardThemeManager {
public:
    enum ThemeType {
        Cyan,       // Default blue/cyan theme
        RacingRed,  // Red/orange performance theme
        Midnight,   // Purple/violet dark theme
        Classic     // White/gray traditional style
    };
    
    static DashboardTheme getTheme(ThemeType type);
    static DashboardTheme getCyanTheme();
    static DashboardTheme getRacingRedTheme();
    static DashboardTheme getMidnightTheme();
    static DashboardTheme getClassicTheme();
    
    static QStringList getThemeNames();
};

#endif // DASHBOARDTHEME_H
