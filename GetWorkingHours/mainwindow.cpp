#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QSystemTrayIcon>

#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <QTimer>
#include <QMessageBox>
#include <QDialog>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(410, 200);
    setWindowTitle(appTitle);

    createActions();
    createTrayIcon();

    // Interaction
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::iconActivated);
    trayIcon->show();


    std::cout << "Started GetWorkingHours" << std::endl;
    QTimer *timer = new QTimer;
    timer->start(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(calcWorkingTime()));

    const bool hasBreake = mySettings->value(settings_HasBreake, true).toBool();
    ui->checkBox_Break->setChecked(hasBreake);

    const QTime breakeTimeValue = mySettings->value(settings_BreakeTime, QTime(0, 45, 0)).toTime();
    ui->timeEdit_BreakTime->setTime(breakeTimeValue);

    const QTime timeNow = QDateTime::currentDateTime().time();
    ui->timeEdit_ClockIn->setTime(timeNow);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_timeEdit_ClockIn_userTimeChanged(const QTime &time)
{
    calcWorkingTime();
}

void MainWindow::on_timeEdit_BreakTime_userTimeChanged(const QTime &time)
{
    calcWorkingTime();
}

void MainWindow::on_checkBox_Break_stateChanged(int arg1)
{
    if (ui->checkBox_Break->isChecked())
    {
        ui->timeEdit_BreakTime->setEnabled(true);
    }
    else
    {
        ui->timeEdit_BreakTime->setEnabled(false);
    }

    calcWorkingTime();
}

void MainWindow::on_pushButton_Now_clicked()
{
    QTime timeNow = QDateTime::currentDateTime().time();
    ui->timeEdit_ClockIn->setTime(timeNow);
}

void MainWindow::calcWorkingTime()
{
    QTime timeStart = ui->timeEdit_ClockIn->dateTime().time();
    QTime timeNow = QDateTime::currentDateTime().time();

    const qint64 mSecondsDiff = timeStart.msecsTo(timeNow);
    qint64 workingTime = mSecondsDiff;

    const QTime userBreakTime = ui->timeEdit_BreakTime->time();
    const qint64 userBreakTimeMSecondsInput = QTime(0, 0, 0).msecsTo(userBreakTime);
    const qint64 userBreakTimeMSeconds = userBreakTimeMSecondsInput - break_1_duration - break_2_duration;
    qint64 usedBreak_1_TimeMSeconds = QTime(0, 0, 0).msec();
    qint64 usedBreak_2_TimeMSeconds = QTime(0, 0, 0).msec();

    if (ui->checkBox_Break->isChecked())
    {
        if (mSecondsDiff >= break_1_Start)
        {
            const qint64 break_1_diff = mSecondsDiff - break_1_Start;
            if (break_1_diff <= break_1_duration)
            {
                usedBreak_1_TimeMSeconds += break_1_diff;
            }
            else
            {
                usedBreak_1_TimeMSeconds += break_1_duration;
            }
        }

        workingTime -= usedBreak_1_TimeMSeconds;

        if (workingTime >= break_2_Start)
        {
            const qint64 break_2_diff = workingTime - break_2_Start;
            if (break_2_diff <= (break_2_duration + userBreakTimeMSeconds))
            {
                usedBreak_2_TimeMSeconds += break_2_diff;
            }
            else
            {
                usedBreak_2_TimeMSeconds += (break_2_duration + userBreakTimeMSeconds);
            }
        }

        workingTime -= usedBreak_2_TimeMSeconds;
    }

    QTime elapsedTime = QTime::fromMSecsSinceStartOfDay(workingTime);
    ui->timeEdit_TimeWorking->setTime(elapsedTime);

    QTime usedBreakTime = QTime::fromMSecsSinceStartOfDay(usedBreak_1_TimeMSeconds + usedBreak_2_TimeMSeconds);
    ui->timeEdit_UsedTimeBreak->setTime(usedBreakTime);

    // Show message if more than 9 hours at work
    // Also show message 15 minutes before 10h
    const qint64 workingHoursInMs = QTime(0, 0, 0).msecsTo(elapsedTime);
    const int workingHoursInMin = workingHoursInMs / 1000 / 60;

    QTime workingTime8h = timeStart.addMSecs(QTime(0, 0, 0).msecsTo(QTime(8, 0, 0, 0)));
    QTime timeEnd = timeStart.addMSecs(QTime(0, 0, 0).msecsTo(QTime(10, 0, 0, 0)));
    if (ui->checkBox_Break->isChecked())
    {
        workingTime8h = workingTime8h.addMSecs(userBreakTimeMSecondsInput);
        timeEnd = timeEnd.addMSecs(userBreakTimeMSecondsInput);
    }

    QString workingTime8hStr  = "Reached 8h at:     " + workingTime8h.toString();
    QString endTimeStr      = "Clock out until: " + timeEnd.toString();

    QString toolTipMessage = "Working since:\n-> " + elapsedTime.toString() + "\n" + workingTime8hStr + "\n" + endTimeStr;
    // Set the tooltip message
    trayIcon->setToolTip(toolTipMessage);

    if (workingHoursInMin < 480 && (shownWorkingTime8h || shownWorkingTime9h || shownWorkingTime10h || shownWorkingTimeProblem)) // < 8h
    {
        trayIcon->setIcon(iconBlue);
        shownWorkingTime8h = false;
        shownWorkingTime9h = false;
        shownWorkingTime10h = false;
        shownWorkingTimeProblem = false;
    }

    if (!shownWorkingTime8h && workingHoursInMin >= 480) // 8h
    {
        shownWorkingTime8h = true;
        trayIcon->setIcon(iconGreen);
        trayIcon->showMessage(appTitle, "Yay you did it!\n" + endTimeStr, iconGreen, 2700000);
    }

    if (!shownWorkingTime9h && workingHoursInMin >= 540) // 9h
    {
        shownWorkingTime9h = true;
        trayIcon->setIcon(iconYellow);
        trayIcon->showMessage(appTitle, "You are working more than 9 hours!\n" + endTimeStr, iconYellow, 2700000);
    }

    if (!shownWorkingTime10h && workingHoursInMin >= 585) // 9h 45min
    {
        shownWorkingTime10h = true;
        trayIcon->setIcon(iconRed);
        trayIcon->showMessage(appTitle, "You are working more than 9h and 45min!\nGO HOME!!! NOW!!!\n" + endTimeStr, iconRed, 900000);
    }

    if (!shownWorkingTimeProblem && workingHoursInMin >= 600) // 9h 45min
    {
        shownWorkingTimeProblem = true;
        trayIcon->showMessage(appTitle, "Houston,\nwe have a problem...\n", iconRed, 2700000);
    }
}

void MainWindow::createActions()
{
    restoreAction = new QAction(tr("&Wiederherstellen"), this);
    connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);

    quitAction = new QAction(tr("&Schließen"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

void MainWindow::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(iconBlue);
    setWindowIcon(iconBlue);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    mySettings->setValue(settings_BreakeTime, ui->timeEdit_BreakTime->time());
    mySettings->setValue(settings_HasBreake,  ui->checkBox_Break->isChecked());
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason_)
{
    switch (reason_) {
        case QSystemTrayIcon::Trigger:
            // trayIcon->showMessage("Hello", "You clicked me!");
            break;
        case QSystemTrayIcon::DoubleClick:
            QWidget::showNormal();
            break;
        default:
        ;
    }
}
