#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QStandardPaths>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QSystemtrayIcon;

class MainWindow : public QMainWindow
{
    Q_OBJECT   

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_timeEdit_ClockIn_userTimeChanged(const QTime &time);

    void on_pushButton_Now_clicked();

    void on_timeEdit_BreakTime_userTimeChanged(const QTime &time);

    void calcWorkingTime();

    void on_checkBox_Break_stateChanged(int arg1);

    void iconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    QString iconBlueName = ":/images/icon_blue_small.png";
    QString iconGreenName = ":/images/icon_green_small.png";
    QString iconYellowName = ":/images/icon_yellow_small.png";
    QString iconRedName = ":/images/icon_red_small.png";

    QIcon iconBlue = QIcon(iconBlueName);
    QIcon iconGreen = QIcon(iconGreenName);
    QIcon iconYellow = QIcon(iconYellowName);
    QIcon iconRed = QIcon(iconRedName);

    void createActions();
    void createTrayIcon();

    QAction *restoreAction;
    QAction *quitAction;

    QString appTitle = "Arbeitszeitrechner - 1.0";

    QString m_path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QString m_filename = "config.ini";
    QSettings * mySettings = new QSettings(m_path + "/"+ m_filename, QSettings::IniFormat);

    QString settings_BreakeTime = "breakeTime";
    QString settings_HasBreake  = "hasBreake";

    bool shownWorkingTime8h = false;
    bool shownWorkingTime9h = false;
    bool shownWorkingTime10h = false;
    bool shownWorkingTimeProblem = false;

    int break_1_Start       = 2     * 60 * 60 * 1000; // h -> ms
    int break_1_duration    = 15    * 60 * 1000; // min -> ms
    int break_2_Start       = 6     * 60 * 60 * 1000; // h -> ms
    int break_2_duration    = 30    * 60 * 1000; // min -> ms
};
#endif // MAINWINDOW_H
