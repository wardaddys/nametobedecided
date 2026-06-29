using System;
using System.IO;
using System.Text.RegularExpressions;

string filePath = "d:/fyp/fyp final project ui improvements june 1 monday/src/widgets/DashboardWidget.cpp";
string text = File.ReadAllText(filePath);

text = text.Replace(@"#include "ThrottleBodyWidget.h"", @"#include "ThrottleBodyWidget.h"
#include "MapSensorWidget.h"
#include "TempSensorWidget.h"
#include "InjectorVisualWidget.h"");

text = text.Replace(@"buildTempPanel();
}", @"buildTempPanel();
    buildDutyPanel();
}");

text = Regex.Replace(text, @"void CalibrationOverlay::showCalibration.*?show\(\);\s+raise\(\);\s+}", @"void CalibrationOverlay::showCalibration(const QString &title) {
    m_titleLabel->setText(title);
    m_currentTitle = title;

    if (title.contains("TPS", Qt::CaseInsensitive)) {
        m_panel->setFixedSize(920, 620);
        loadTpsFromSettings();
        m_stack->setCurrentWidget(m_tpsPanel);
    } else if (title.contains("MAP", Qt::CaseInsensitive)) {
        m_panel->setFixedSize(820, 520);
        loadMapFromSettings();
        if (m_mapVisual) m_mapVisual->setKpa(0.0);
        m_stack->setCurrentWidget(m_mapPanel);
    } else if (title.contains("Duty", Qt::CaseInsensitive)) {
        m_panel->setFixedSize(780, 520);
        if (m_dutyVisual) m_dutyVisual->setDuty(0.0);
        m_stack->setCurrentWidget(m_dutyPanel);
    } else {
        m_panel->setFixedSize(820, 580);
        const bool isClt = title.contains("CLT", Qt::CaseInsensitive);
        loadTempFromSettings();
        m_tempLiveLbl->setText(isClt ? "CLT: --- °C" : "IAT: --- °C");
        if (m_tempVisual) {
            m_tempVisual->setCltMode(isClt);
            m_tempVisual->setTemp(20.0);
        }
        m_stack->setCurrentWidget(m_tempPanel);
    }

    if (parentWidget()) {
        resize(parentWidget()->size());
        m_panel->move((width() - m_panel->width()) / 2, (height() - m_panel->height()) / 2);
    }

    show();
    raise();
}", RegexOptions.Singleline);

text = Regex.Replace(text, @"connect\(m_dutyGauge, &TunerGauge::clicked, this, \[this\]\(\)\{.*?\}\);", @"connect(m_dutyGauge, &TunerGauge::clicked, this, [=](){ handleCalibClick("Injector Duty %"); });", RegexOptions.Singleline);

File.WriteAllText(filePath, text);
