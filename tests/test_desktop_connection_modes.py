"""Qt integration regression check. Requires a Makefiles NKR_NO_EXTERNAL build.
Run: python tests/test_desktop_connection_modes.py /absolute/path/to/build-ui
OS mode setters are mocked; no proxy settings, privileges or real network are changed.
"""
from pathlib import Path
import shlex, subprocess, sys, tempfile
work = Path(tempfile.mkdtemp(prefix="nekoray-mode-test-"))
for asset in ("geoip.db", "geosite.db"): (work/asset).touch()
repo=Path(__file__).resolve().parents[1]; build=Path(sys.argv[1]).resolve()
s=(repo/'main/main.cpp').read_text().replace('#include "ui/mainwindow_interface.h"', '#include "ui/mainwindow.h"\n#include "ui/ThemeManager.hpp"\n#include "db/Database.hpp"\n#include <QTimer>\n#include <QPushButton>\n#include <QToolButton>\n#include <QLineEdit>\n#include <QTabWidget>\n#include <QHeaderView>\n#include <QDebug>\n#include <QTextBrowser>\n#include <QMenu>\n#include <QCheckBox>\n#include <QElapsedTimer>\n#include <QThread>')
s=s.replace('    return QApplication::exec();',r'''
    QTimer::singleShot(300, &a, [&] {
        auto window = GetMainWindow();
        auto table = window->findChild<QTableWidget *>("proxyListTable");
        auto search = window->findChild<QLineEdit *>("search");
        auto connection = window->findChild<QPushButton *>("systemProxyConnectionButton");
        auto groups = NekoGui::profileManager->CurrentGroup();
        groups->name = QString::fromUtf8("示例订阅");
        const QStringList names = {QString::fromUtf8("香港 · 日常使用"), QString::fromUtf8("日本 · 东京"), QString::fromUtf8("新加坡 · 备用"), QString::fromUtf8("美国 · 洛杉矶"), QString::fromUtf8("德国 · 法兰克福")};
        for (int i = 0; i < names.size(); ++i) {
            auto entity = NekoGui::ProfileManager::NewProxyEntity("socks");
            entity->bean->name = names[i];
            entity->bean->serverAddress = QStringLiteral("node%1.example.com").arg(i + 1);
            entity->bean->serverPort = 443;
            entity->latency = 38 + i * 42;
            NekoGui::profileManager->AddProfile(entity);
        }
        window->refresh_groups();
        auto check = [](bool ok, const char *message) {
            if (!ok) { qCritical() << "FAIL:" << message; std::exit(2); }
            qInfo() << "PASS:" << message;
        };
        check(search->isVisible(), "Search is visible at startup");
        check(table->rowCount() == 5, "Existing profile model populates redesigned list");
        check(table->horizontalHeader()->visualIndex(2) == 0, "Name column comes first");
        search->setText(QString::fromUtf8("东京"));
        int visible = 0; for (int i = 0; i < table->rowCount(); ++i) if (!table->isRowHidden(i)) ++visible;
        check(visible == 1, "Search filters real profile rows");
        window->refresh_proxy_list();
        visible = 0; for (int i = 0; i < table->rowCount(); ++i) if (!table->isRowHidden(i)) ++visible;
        check(visible == 1, "Search remains applied after refresh");
        search->clear();
        qInfo() << "locale" << QLocale().name();
        for (auto button : window->findChildren<QPushButton *>()) qInfo() << "button" << button->text();
        for (auto button : window->findChildren<QPushButton *>()) {
            if (button->text() == QString::fromUtf8("连接记录")) button->click();
        }
        check(window->findChild<QTableWidget *>("tableWidget_conn")->isVisible(), "Connections navigation shows existing view");
        for (auto button : window->findChildren<QPushButton *>()) {
            if (button->text() == QString::fromUtf8("日志")) button->click();
        }
        check(window->findChild<QTextBrowser *>("masterLogBrowser")->isVisible(), "Logs navigation shows existing view");
        for (auto button : window->findChildren<QPushButton *>()) {
            if (button->text() == QString::fromUtf8("首页")) button->click();
        }
        check(table->isVisible(), "Home navigation restores node list");
        check(window->findChild<QToolButton *>("addNodeButton")->menu()->actions().size() == 4, "Add menu retains import and subscription commands");
        table->selectRow(0);
        themeManager->ApplyTheme("4");
        auto stylesheet = a.styleSheet();
        themeManager->ApplyTheme("4");
        check(a.styleSheet() == stylesheet, "Repeated theme application does not duplicate styles");
        a.processEvents();
        window->grab().save("NEKO_SCREENSHOT_DIR/nekoray-ui-light.png");
        themeManager->ApplyTheme("5");
        a.processEvents();
        check(a.palette().color(QPalette::Window).lightness() < 128, "Dark theme sets dark palette");
        window->grab().save("NEKO_SCREENSHOT_DIR/nekoray-ui-dark.png");
        window->resize(800, 600);
        a.processEvents();
        check(connection->isVisible() && connection->geometry().right() <= connection->parentWidget()->width(), "Connection control fits minimum window");
        window->grab().save("NEKO_SCREENSHOT_DIR/nekoray-ui-compact.png");

        auto tunButton = window->findChild<QPushButton *>("tunConnectionButton");
        check(tunButton->isVisible(), "Both connection modes are visible");
        check(!window->findChild<QCheckBox *>("checkBox_VPN")->isVisible() && !window->findChild<QCheckBox *>("checkBox_SystemProxy")->isVisible(), "Mode checkboxes are hidden from home");
        auto ds = NekoGui::dataStore;
        ds->core_running = true;
        ds->vpn_internal_tun = true;
        auto waitFor = [&](const std::function<bool()> &predicate) {
            QElapsedTimer timer; timer.start();
            while (!predicate() && timer.elapsed() < 3000) { a.processEvents(); QThread::msleep(5); }
            return predicate();
        };
        connection->click();
        check(!connection->isEnabled() && !tunButton->isEnabled(), "Repeated clicks blocked while starting");
        check(waitFor([&] { return connection->isEnabled(); }), "System proxy start completes asynchronously");
        check(ds->started_id >= 0 && ds->spmode_system_proxy && !ds->spmode_vpn, "System proxy enabled after core start");
        tunButton->click();
        check(waitFor([&] { return tunButton->isEnabled(); }), "Switch from system proxy to TUN completes");
        check(ds->started_id >= 0 && ds->spmode_vpn && !ds->spmode_system_proxy, "TUN and system proxy are mutually exclusive");
        connection->click();
        check(waitFor([&] { return connection->isEnabled(); }), "Switch from TUN to system proxy completes");
        check(ds->spmode_system_proxy && !ds->spmode_vpn, "Reverse switch clears TUN");
        connection->click();
        check(waitFor([&] { return connection->isEnabled(); }), "Disconnect completes");
        check(ds->started_id < 0 && !ds->spmode_system_proxy && !ds->spmode_vpn, "Disconnect clears core and both modes");
        qputenv("NEKO_TEST_TUN_FAIL", "1");
        tunButton->click();
        check(tunButton->isEnabled() && ds->started_id < 0 && !ds->spmode_vpn, "Denied TUN setup restores disconnected controls");
        qunsetenv("NEKO_TEST_TUN_FAIL");
        tunButton->click();
        check(waitFor([&] { return tunButton->isEnabled(); }), "TUN starts after canceled setup");
        window->neko_stop(true);
        check(waitFor([&] { return ds->started_id < 0 && !ds->spmode_vpn; }), "Core crash clears managed TUN mode");

        qputenv("NEKO_TEST_START_FAIL", "1");
        tunButton->click();
        check(waitFor([&] { return tunButton->isEnabled(); }), "Failed core configuration request completes");
        check(ds->started_id < 0 && !ds->spmode_vpn && !ds->spmode_system_proxy, "Failed core start rolls back TUN and proxy settings");
        qunsetenv("NEKO_TEST_START_FAIL");
        ds->core_running = false;
        tunButton->click();
        check(waitFor([&] { return tunButton->isEnabled(); }), "Missing core executable resets pending start");
        check(ds->started_id < 0 && !ds->spmode_vpn, "Failed process launch clears pending TUN");
        qInfo() << "UI and asynchronous mode integration checks completed with mocked OS mode setters; networking is not tested.";
        std::exit(0);
    });
    return QApplication::exec();''')
s=s.replace('    UI_InitMainWindow();', '    loadTranslate("zh_CN");\n    UI_InitMainWindow();')
p=work/'integration_main.cpp'; s=s.replace('NEKO_SCREENSHOT_DIR', str(work)); p.write_text(s)
flags={}
for line in (build/'CMakeFiles/nekobox.dir/flags.make').read_text().splitlines():
    if line.startswith('CXX_'): k,v=line.split(' = ',1);flags[k]=shlex.split(v)
obj=work/'integration_main.o'
subprocess.run(['/usr/bin/c++',*flags['CXX_DEFINES'],*flags['CXX_INCLUDES'],*flags['CXX_FLAGS'],'-c',str(p),'-o',str(obj)],check=True)
# Compile a test-only copy with OS side effects replaced. Production asynchronous start/stop stays intact.
original=(repo/'ui/mainwindow.cpp').read_text()
a=original.index('void MainWindow::neko_set_spmode_system_proxy('); b=original.index('void MainWindow::refresh_status(',a)
original=original[:a]+r'''void MainWindow::neko_set_spmode_system_proxy(bool enable, bool) { NekoGui::dataStore->spmode_system_proxy = enable; refresh_status(); }
void MainWindow::neko_set_spmode_vpn(bool enable, bool, bool) { NekoGui::dataStore->spmode_vpn = enable && !qEnvironmentVariableIsSet("NEKO_TEST_TUN_FAIL"); refresh_status(); }
'''+original[b:]
mock=work/'mode_test_mainwindow.cpp'; mock.write_text(original)
mockobj=work/'mode_test_mainwindow.o'
subprocess.run(['/usr/bin/c++',*flags['CXX_DEFINES'],*flags['CXX_INCLUDES'], '-I'+str(repo/'ui'), '-I'+str(build),*flags['CXX_FLAGS'],'-c',str(mock),'-o',str(mockobj)],check=True)
grpc=(repo/'ui/mainwindow_grpc.cpp').read_text().replace('auto neko_start_stage2 = [=] {','auto neko_start_stage2 = [=] { if (qEnvironmentVariableIsSet("NEKO_TEST_START_FAIL")) return false;')
grpcfile=work/'mode_test_grpc.cpp'; grpcfile.write_text(grpc)
grpcobj=work/'mode_test_grpc.o'
subprocess.run(['/usr/bin/c++',*flags['CXX_DEFINES'],*flags['CXX_INCLUDES'], '-I'+str(repo/'ui'), '-I'+str(build),*flags['CXX_FLAGS'],'-c',str(grpcfile),'-o',str(grpcobj)],check=True)
cmd=shlex.split((build/'CMakeFiles/nekobox.dir/link.txt').read_text())
cmd=[str(obj) if x=='CMakeFiles/nekobox.dir/main/main.cpp.o' else x for x in cmd]
cmd=[str(mockobj) if x=='CMakeFiles/nekobox.dir/ui/mainwindow.cpp.o' else x for x in cmd]
cmd=[str(grpcobj) if x=='CMakeFiles/nekobox.dir/ui/mainwindow_grpc.cpp.o' else x for x in cmd]
i=cmd.index('-o');cmd[i+1]=str(work/'nekoray-mode-test')
subprocess.run(cmd,cwd=build,check=True)

subprocess.run([str(work/'nekoray-mode-test'), '-appdata', str(work/'appdata'), '-many'], env={**__import__('os').environ, 'QT_QPA_PLATFORM':'offscreen'}, timeout=25, check=True)
print('Test screenshots:', work)
