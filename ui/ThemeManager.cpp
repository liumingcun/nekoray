#include <QStyle>
#include <QApplication>
#include <QStyleFactory>
#include <QMap>
#include <map>

#include "ThemeManager.hpp"

ThemeManager *themeManager = new ThemeManager;

extern QString ReadFileText(const QString &path);

void ThemeManager::ApplyTheme(const QString &theme) {
    if (current_theme == theme) return;
    auto internal = [=] {
        if (this->system_style_name.isEmpty()) {
            this->system_style_name = qApp->style()->objectName();
            system_palette = qApp->palette();
        }
        if (this->current_theme == theme) {
            return;
        }

        bool ok;
        auto themeId = theme.toInt(&ok);

        if (ok) {
            // System & Built-in
            QString qss;

            if (themeId >= 1 && themeId <= 3) {
                QString path;
                std::map<QString, QString> replace;
                switch (themeId) {
                    case 1:
                        path = ":/themes/feiyangqingyun/qss/flatgray.css";
                        replace[":/qss/"] = ":/themes/feiyangqingyun/qss/";
                        break;
                    case 2:
                        path = ":/themes/feiyangqingyun/qss/lightblue.css";
                        replace[":/qss/"] = ":/themes/feiyangqingyun/qss/";
                        break;
                    case 3:
                        path = ":/themes/feiyangqingyun/qss/blacksoft.css";
                        replace[":/qss/"] = ":/themes/feiyangqingyun/qss/";
                        break;
                    default:
                        return;
                }
                qss = ReadFileText(path);
                for (auto const &[a, b]: replace) {
                    qss = qss.replace(a, b);
                }
            }

            if (themeId < 0 || themeId > 5) return;
            auto system_style = QStyleFactory::create(this->system_style_name);

            if (themeId == 0 || themeId == 4 || themeId == 5) {
                delete system_style;
                qApp->setStyle(QStyleFactory::create("Fusion"));
                const bool dark = themeId == 5 || (themeId == 0 && system_palette.color(QPalette::Window).lightness() < 128);
                const QMap<QString, QString> colors = {
                    {"@background", dark ? "#17181C" : "#F2F3F7"},
                    {"@surface", dark ? "#23252B" : "#FFFFFF"},
                    {"@sidebar", dark ? "#1E2025" : "#E9ECF2"},
                    {"@text", dark ? "#F1F3F7" : "#202634"},
                    {"@muted", dark ? "#A8B0BF" : "#626D80"},
                    {"@border", dark ? "#353A45" : "#DCE1E9"},
                    {"@accent", dark ? "#4295FF" : "#0866D9"},
                    {"@selection", dark ? "#263E60" : "#E6F0FF"},
                    {"@hover", dark ? "#2B303A" : "#F4F7FC"},
                    {"@success", dark ? "#3FBC86" : "#168153"}
                };
                QPalette palette = qApp->style()->standardPalette();
                palette.setColor(QPalette::Window, QColor(colors["@background"]));
                palette.setColor(QPalette::WindowText, QColor(colors["@text"]));
                palette.setColor(QPalette::Base, QColor(colors["@surface"]));
                palette.setColor(QPalette::AlternateBase, QColor(colors["@hover"]));
                palette.setColor(QPalette::Text, QColor(colors["@text"]));
                palette.setColor(QPalette::Button, QColor(colors["@surface"]));
                palette.setColor(QPalette::ButtonText, QColor(colors["@text"]));
                palette.setColor(QPalette::ToolTipBase, QColor(colors["@surface"]));
                palette.setColor(QPalette::ToolTipText, QColor(colors["@text"]));
                palette.setColor(QPalette::Link, QColor(colors["@accent"]));
                palette.setColor(QPalette::Highlight, QColor(colors["@selection"]));
                palette.setColor(QPalette::HighlightedText, QColor(colors["@text"]));
                palette.setColor(QPalette::Disabled, QPalette::Text, QColor(colors["@muted"]));
                palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(colors["@muted"]));
                qApp->setPalette(palette);
                qss = ReadFileText(":/neko/theme/shadowrocket.qss");
                for (auto it = colors.cbegin(); it != colors.cend(); ++it) qss.replace(it.key(), it.value());
                qApp->setStyleSheet(qss);
            } else {
                if (themeId == 1 || themeId == 2 || themeId == 3) {
                    // feiyangqingyun theme
                    QString paletteColor = qss.mid(20, 7);
                    qApp->setPalette(QPalette(paletteColor));
                } else {
                    // other theme
                    qApp->setPalette(system_style->standardPalette());
                }
                qApp->setStyle(system_style);
                qApp->setStyleSheet(qss);
            }
        } else {
            // QStyleFactory
            const auto &_style = QStyleFactory::create(theme);
            if (_style != nullptr) {
                qApp->setPalette(_style->standardPalette());
                qApp->setStyle(_style);
                qApp->setStyleSheet("");
            }
        }

        current_theme = theme;
    };
    internal();

    auto nekoray_css = ReadFileText(":/neko/neko.css");
    qApp->setStyleSheet(qApp->styleSheet().append("\n").append(nekoray_css));
}
