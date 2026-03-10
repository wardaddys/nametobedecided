#include "DashboardTheme.h"

DashboardTheme DashboardThemeManager::getTheme(ThemeType type) {
    switch (type) {
        case RacingRed: return getRacingRedTheme();
        case Midnight:  return getMidnightTheme();
        case Classic:   return getClassicTheme();
        case Cyan:
        default:        return getCyanTheme();
    }
}

DashboardTheme DashboardThemeManager::getCyanTheme() {
    DashboardTheme theme;
    theme.name = "Cyan";
    
    // Background
    theme.bgPrimary = QColor(15, 15, 20);
    theme.bgSecondary = QColor(25, 28, 35);
    theme.bgAccent = QColor(30, 35, 45);
    
    // Gauges
    theme.gaugeBorder = QColor(0, 188, 212);      // Cyan
    theme.gaugeBackground = QColor(0, 0, 0);
    theme.gaugeNeedle = QColor(255, 255, 255);
    theme.gaugeText = QColor(170, 170, 170);
    theme.gaugeValue = QColor(0, 191, 255);       // Cyan
    
    // Status
    theme.statusGood = QColor(0, 255, 0);
    theme.statusWarning = QColor(255, 165, 0);
    theme.statusDanger = QColor(255, 0, 0);
    theme.statusInactive = QColor(128, 128, 128);
    
    // Graph
    theme.graphLine1 = QColor(0, 255, 128);       // Green
    theme.graphLine2 = QColor(255, 100, 100);     // Red
    theme.graphGrid = QColor(50, 55, 65);
    theme.graphBackground = QColor(20, 22, 28);
    
    // Accents
    theme.accent = QColor(0, 191, 255);           // Cyan
    theme.accentSecondary = QColor(0, 150, 200);
    
    // Text
    theme.textPrimary = QColor(255, 255, 255);
    theme.textSecondary = QColor(200, 200, 200);
    theme.textMuted = QColor(128, 128, 128);
    
    return theme;
}

DashboardTheme DashboardThemeManager::getRacingRedTheme() {
    DashboardTheme theme;
    theme.name = "Racing Red";
    
    // Background
    theme.bgPrimary = QColor(20, 10, 10);
    theme.bgSecondary = QColor(35, 20, 20);
    theme.bgAccent = QColor(50, 25, 25);
    
    // Gauges
    theme.gaugeBorder = QColor(255, 50, 50);      // Red
    theme.gaugeBackground = QColor(0, 0, 0);
    theme.gaugeNeedle = QColor(255, 200, 0);      // Orange/Yellow
    theme.gaugeText = QColor(200, 180, 160);
    theme.gaugeValue = QColor(255, 100, 50);      // Orange
    
    // Status
    theme.statusGood = QColor(0, 255, 0);
    theme.statusWarning = QColor(255, 200, 0);
    theme.statusDanger = QColor(255, 0, 0);
    theme.statusInactive = QColor(100, 80, 80);
    
    // Graph
    theme.graphLine1 = QColor(255, 220, 100);     // Yellow
    theme.graphLine2 = QColor(0, 200, 100);       // Green
    theme.graphGrid = QColor(60, 40, 40);
    theme.graphBackground = QColor(25, 15, 15);
    
    // Accents
    theme.accent = QColor(255, 80, 50);           // Red-Orange
    theme.accentSecondary = QColor(200, 50, 30);
    
    // Text
    theme.textPrimary = QColor(255, 255, 255);
    theme.textSecondary = QColor(220, 200, 180);
    theme.textMuted = QColor(140, 120, 100);
    
    return theme;
}

DashboardTheme DashboardThemeManager::getMidnightTheme() {
    DashboardTheme theme;
    theme.name = "Midnight";
    
    // Background
    theme.bgPrimary = QColor(15, 10, 25);
    theme.bgSecondary = QColor(25, 20, 40);
    theme.bgAccent = QColor(40, 30, 60);
    
    // Gauges
    theme.gaugeBorder = QColor(150, 100, 255);    // Purple
    theme.gaugeBackground = QColor(5, 5, 15);
    theme.gaugeNeedle = QColor(255, 255, 255);
    theme.gaugeText = QColor(180, 160, 200);
    theme.gaugeValue = QColor(200, 150, 255);     // Light purple
    
    // Status
    theme.statusGood = QColor(100, 255, 150);     // Mint
    theme.statusWarning = QColor(255, 180, 100);
    theme.statusDanger = QColor(255, 80, 120);    // Pink-red
    theme.statusInactive = QColor(100, 90, 120);
    
    // Graph
    theme.graphLine1 = QColor(150, 255, 200);     // Mint
    theme.graphLine2 = QColor(255, 150, 200);     // Pink
    theme.graphGrid = QColor(50, 40, 70);
    theme.graphBackground = QColor(20, 15, 35);
    
    // Accents
    theme.accent = QColor(180, 120, 255);         // Violet
    theme.accentSecondary = QColor(120, 80, 200);
    
    // Text
    theme.textPrimary = QColor(255, 255, 255);
    theme.textSecondary = QColor(200, 190, 220);
    theme.textMuted = QColor(120, 110, 140);
    
    return theme;
}

DashboardTheme DashboardThemeManager::getClassicTheme() {
    DashboardTheme theme;
    theme.name = "Classic";
    
    // Background
    theme.bgPrimary = QColor(30, 30, 30);
    theme.bgSecondary = QColor(45, 45, 45);
    theme.bgAccent = QColor(60, 60, 60);
    
    // Gauges
    theme.gaugeBorder = QColor(200, 200, 200);    // Light gray
    theme.gaugeBackground = QColor(20, 20, 20);
    theme.gaugeNeedle = QColor(255, 50, 50);      // Red needle
    theme.gaugeText = QColor(200, 200, 200);
    theme.gaugeValue = QColor(255, 255, 255);
    
    // Status
    theme.statusGood = QColor(50, 200, 50);
    theme.statusWarning = QColor(255, 180, 50);
    theme.statusDanger = QColor(255, 50, 50);
    theme.statusInactive = QColor(100, 100, 100);
    
    // Graph
    theme.graphLine1 = QColor(100, 200, 100);     // Green
    theme.graphLine2 = QColor(200, 100, 100);     // Red
    theme.graphGrid = QColor(70, 70, 70);
    theme.graphBackground = QColor(35, 35, 35);
    
    // Accents
    theme.accent = QColor(220, 220, 220);         // Light gray
    theme.accentSecondary = QColor(180, 180, 180);
    
    // Text
    theme.textPrimary = QColor(255, 255, 255);
    theme.textSecondary = QColor(200, 200, 200);
    theme.textMuted = QColor(130, 130, 130);
    
    return theme;
}

QStringList DashboardThemeManager::getThemeNames() {
    return QStringList() << "Cyan" << "Racing Red" << "Midnight" << "Classic";
}
