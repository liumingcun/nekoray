#pragma once

#include <QString>
#include <QPalette>

class ThemeManager {
public:
    QString system_style_name = "";
    QPalette system_palette;
    QString current_theme; // int: 0:system 1+:builtin string: QStyleFactory

    void ApplyTheme(const QString &theme);
};

extern ThemeManager *themeManager;
