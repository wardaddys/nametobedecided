import re

file_path = 'src/widgets/DashboardWidget.cpp'
with open(file_path, 'r', encoding='utf-8') as f:
    text = f.read()

# 1. Includes
text = text.replace('#include "ThrottleBodyWidget.h"', '#include "ThrottleBodyWidget.h"\n#include "MapSensorWidget.h"\n#include "TempSensorWidget.h"\n#include "InjectorVisualWidget.h"')

# 2. constructor call
text = text.replace('    buildTempPanel();\n}', '    buildTempPanel();\n    buildDutyPanel();\n}')

# 3. showCalibration
old_show_calib = re.search(r'void CalibrationOverlay::showCalibration.*?show\(\);\s+raise\(\);\s+}', text, re.DOTALL).group(0)
new_show_calib = '''void CalibrationOverlay::showCalibration(const QString &title) {
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
}'''
text = text.replace(old_show_calib, new_show_calib)

# 4. click handler
old_click = re.search(r'connect\(m_dutyGauge, &TunerGauge::clicked, this, \[this\]\(\)\{.*?\}\);', text, re.DOTALL).group(0)
new_click = 'connect(m_dutyGauge, &TunerGauge::clicked, this, [=](){ handleCalibClick("Injector Duty %"); });'
text = text.replace(old_click, new_click)

# 5. updateLiveData
old_update_live_data = re.search(r'    } else if \(m_stack->currentWidget\(\) == m_tempPanel\) \{.*m_tempLiveBar->maximum\(\)\)\);.*?}', text, re.DOTALL).group(0)

new_update_live_data = '''    } else if (m_stack->currentWidget() == m_tempPanel) {
        const bool isIat = m_currentTitle.contains("IAT", Qt::CaseInsensitive);
        const double tempC = isIat ? data.getIAT() : data.getCoolant();
        m_tempLiveLbl->setText(QString("%1: %2 °C").arg(isIat ? "IAT" : "CLT").arg(tempC, 0, 'f', 1));
        m_tempLiveBar->setValue(qBound(m_tempLiveBar->minimum(),
                                       static_cast<int>(tempC + 0.5),
                                       m_tempLiveBar->maximum()));
        if (m_tempVisual) m_tempVisual->setTemp(tempC);
    } else if (m_stack->currentWidget() == m_dutyPanel) {
        const double rpm = data.getRPM();
        const double pw = data.getPW();
        const double duty = std::max(0.0, std::min(100.0, (rpm * pw) / 1200.0));
        m_dutyLiveLbl->setText(QString("Duty: %1 %").arg(duty, 0, 'f', 1));
        m_dutyLiveBar->setValue(qBound(m_dutyLiveBar->minimum(),
                                       static_cast<int>(duty + 0.5),
                                       m_dutyLiveBar->maximum()));
        if (m_dutyVisual) m_dutyVisual->setDuty(duty);
    }
}'''

text = text.replace(old_update_live_data, new_update_live_data)

# 6. mapVisual in MAP
old_map_live = '''        m_mapLiveBar->setValue(qBound(m_mapLiveBar->minimum(),
                                      static_cast<int>(kpa + 0.5),
                                      m_mapLiveBar->maximum()));'''
new_map_live = old_map_live + '\\n        if (m_mapVisual) m_mapVisual->setKpa(kpa);'
text = text.replace(old_map_live, new_map_live)

with open(file_path, 'w', encoding='utf-8') as f:
    f.write(text)

print("done")
