#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QToolButton>
#include <QPushButton>
#include <QAction>
#include <QCoreApplication>

// Reuse the existing views and actions, keeping profile and core logic in MainWindow.
struct DesktopShellControls {
    QTabWidget *groups;
    QTabWidget *activity;
    QLabel *running;
    QLabel *inbound;
    QLabel *speed;
    QLineEdit *search;
    QCheckBox *systemProxy;
    QCheckBox *tun;
    QToolButton *test;
    QToolButton *program;
    QToolButton *preferences;
    QToolButton *servers;
    QToolButton *document;
    QToolButton *update;
    QAction *subscriptions;
    QAction *routing;
    QAction *settings;
    QAction *add;
    QAction *clipboard;
};

struct DesktopConnectionButtons { QPushButton *systemProxy; QPushButton *tun; };

class DesktopShell {
    Q_DECLARE_TR_FUNCTIONS(DesktopShell)
public:
    static DesktopConnectionButtons Build(QMainWindow *window, const DesktopShellControls &controls);
};
