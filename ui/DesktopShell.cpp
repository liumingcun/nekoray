#include "DesktopShell.hpp"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QMenu>
#include <QTabBar>
#include <QCoreApplication>

DesktopConnectionButtons DesktopShell::Build(QMainWindow *window, const DesktopShellControls &c) {
    // Keep the designer widget alive: existing slots still reference its controls.
    auto legacy = window->takeCentralWidget();
    legacy->setParent(window);
    legacy->hide();

    auto root = new QWidget(window);
    root->setObjectName("desktopShell");
    auto outer = new QHBoxLayout(root);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto sidebar = new QWidget(root);
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(168);
    auto nav = new QVBoxLayout(sidebar);
    nav->setContentsMargins(16, 26, 16, 18);
    nav->setSpacing(8);
    auto brand = new QLabel("NekoBox", sidebar);
    brand->setObjectName("brand");
    nav->addWidget(brand);
    auto caption = new QLabel(tr("PROXY MANAGER"), sidebar);
    caption->setObjectName("mutedLabel");
    nav->addWidget(caption);
    nav->addSpacing(24);

    auto body = new QWidget(root);
    auto content = new QVBoxLayout(body);
    content->setContentsMargins(24, 24, 24, 18);
    content->setSpacing(16);
    auto heading = new QLabel(tr("Home"), body);
    heading->setObjectName("pageHeading");
    content->addWidget(heading);

    auto status = new QWidget(body);
    status->setObjectName("connectionPanel");
    auto statusLayout = new QVBoxLayout(status);
    statusLayout->setContentsMargins(20, 16, 20, 16);
    statusLayout->setSpacing(12);
    auto summary = new QHBoxLayout;
    auto labels = new QVBoxLayout;
    c.running->setObjectName("connectionTitle");
    c.running->setWordWrap(true);
    c.running->setMinimumWidth(160);
    c.inbound->setObjectName("mutedLabel");
    labels->addWidget(c.running);
    labels->addWidget(c.inbound);
    summary->addLayout(labels, 1);
    summary->addWidget(c.speed);
    summary->addSpacing(16);

    statusLayout->addLayout(summary);
    auto modes = new QHBoxLayout;
    c.systemProxy->hide();
    c.tun->hide();
    auto systemProxy = new QPushButton(tr("Connect with system proxy"), status);
    auto tun = new QPushButton(tr("Connect with TUN"), status);
    systemProxy->setObjectName("systemProxyConnectionButton");
    tun->setObjectName("tunConnectionButton");
    systemProxy->setProperty("connectionMode", true);
    tun->setProperty("connectionMode", true);
    systemProxy->setToolTip(tr("Start the selected node and enable the system proxy."));
    tun->setToolTip(tr("Start the selected node with TUN. Administrator permission may be required."));
    modes->addWidget(systemProxy);
    modes->addWidget(tun);
    modes->addStretch();
    auto routing = new QPushButton(tr("Routing settings"), status);
    routing->setObjectName("quietButton");
    QObject::connect(routing, &QPushButton::clicked, c.routing, &QAction::trigger);
    modes->addWidget(routing);
    statusLayout->addLayout(modes);
    content->addWidget(status);

    auto pages = new QStackedWidget(body);
    pages->setObjectName("contentPages");
    auto home = new QWidget(pages);
    auto homeLayout = new QVBoxLayout(home);
    homeLayout->setContentsMargins(0, 0, 0, 0);
    homeLayout->setSpacing(12);
    auto tools = new QHBoxLayout;
    c.search->setPlaceholderText(tr("Search nodes…"));
    c.search->setAccessibleName(tr("Search nodes"));
    c.search->setMinimumWidth(160);
    c.search->setMaximumWidth(360);
    tools->addWidget(c.search, 1);
    tools->addStretch();
    c.test->setText(tr("Test latency"));
    c.test->setToolButtonStyle(Qt::ToolButtonTextOnly);
    tools->addWidget(c.test);
    auto add = new QToolButton(home);
    add->setObjectName("addNodeButton");
    add->setText(tr("+ Add node"));
    add->setPopupMode(QToolButton::InstantPopup);
    auto addMenu = new QMenu(add);
    addMenu->addAction(c.add);
    addMenu->addAction(c.clipboard);
    addMenu->addSeparator();
    addMenu->addAction(c.subscriptions);
    add->setMenu(addMenu);
    tools->addWidget(add);
    homeLayout->addLayout(tools);
    auto empty = new QLabel(tr("No nodes yet. Add a node or import a subscription."), home);
    empty->setObjectName("nodeEmptyState");
    empty->setWordWrap(true);
    homeLayout->addWidget(empty);
    homeLayout->addWidget(c.groups, 1);
    pages->addWidget(home);
    pages->addWidget(c.activity);
    c.activity->tabBar()->hide();
    content->addWidget(pages, 1);

    auto navIcon = [&](const QString &text) {
        QString name;
        if (text == tr("Home")) name = "home";
        else if (text == tr("Subscriptions")) name = "subscriptions";
        else if (text == tr("Routing")) name = "routing";
        else if (text == tr("Connections")) name = "connections";
        else if (text == tr("Logs")) name = "logs";
        else name = "settings";
        return QIcon(":/icon/navigation/" + name + ".svg");
    };
    auto selection = new QButtonGroup(root);
    selection->setExclusive(true);
    auto pageButton = [&](const QString &text, int page, int activityIndex) {
        auto button = new QPushButton(text, sidebar);
        button->setProperty("navigation", true);
        button->setIcon(navIcon(text));
        button->setIconSize(QSize(20, 20));
        button->setCheckable(true);
        selection->addButton(button);
        nav->addWidget(button);
        QObject::connect(button, &QPushButton::clicked, root, [=] {
            pages->setCurrentIndex(page);
            if (page == 1) c.activity->setCurrentIndex(activityIndex);
            heading->setText(text);
        });
        return button;
    };
    pageButton(tr("Home"), 0, 0)->setChecked(true);
    auto actionButton = [&](const QString &text, QAction *action) {
        auto button = new QPushButton(text, sidebar);
        button->setProperty("navigation", true);
        button->setIcon(navIcon(text));
        button->setIconSize(QSize(20, 20));
        nav->addWidget(button);
        QObject::connect(button, &QPushButton::clicked, action, &QAction::trigger);
    };
    actionButton(tr("Subscriptions"), c.subscriptions);
    actionButton(tr("Routing"), c.routing);
    pageButton(tr("Connections"), 1, 1);
    pageButton(tr("Logs"), 1, 0);
    actionButton(tr("Settings"), c.settings);
    nav->addStretch();

    // Retain all existing menu commands, including advanced profile operations.
    for (auto button : {c.servers, c.preferences, c.program, c.document, c.update}) {
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setIconSize(QSize(16, 16));
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        nav->addWidget(button);
    }
    outer->addWidget(sidebar);
    outer->addWidget(body, 1);
    window->setCentralWidget(root);
    window->resize(1060, 720);
    return {systemProxy, tun};
}
