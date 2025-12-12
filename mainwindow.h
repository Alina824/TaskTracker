#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include "weekview.h"
#include "dayview.h"
#include "monthview.h"
#include "notesview.h"
#include "noteeditview.h"
#include "allnotesview.h"
#include "trackersview.h"
#include "authdialog.h"
#include <QSystemTrayIcon>
#include <QTimer>
#include <QSettings>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAuthButtonClicked();
    void onMonthButtonClicked();
    void onTrackersButtonClicked();
    void onNotesButtonClicked();
    void onNotificationsButtonClicked();
    void onLoginRequested(const QString& username, const QString& password);
    void onRegisterRequested(const QString& username, const QString& password);
    void onDayClicked(const QDate& date);
    void onAddTaskRequested(const QDate& date);
    void onDayViewBack();
    void onTaskAdded(const QDate& date);
    void onNoteClicked(int noteId);
    void onNotesViewBack();
    void onNoteEditBack();
    void onAllNotesButtonClicked();
    void onAllNotesViewBack();
    void checkNotifications();

private:
    void setupUI();
    void updateAuthButton();
    void showWeekView();
    void showDayView(const QDate& date);
    void showMonthView();
    void showNotesView();
    void showNoteEditView(int noteId);
    void showAllNotesView();
    void showTrackersView();
    
    QStackedWidget *stackedWidget;
    WeekView *weekView;
    DayView *dayView;
    MonthView *monthView;
    NotesView *notesView;
    NoteEditView *noteEditView;
    AllNotesView *allNotesView;
    TrackersView *trackersView;
    QPushButton *authButton;
    QPushButton *monthButton;
    QPushButton *trackersButton;
    QPushButton *notesButton;
    QPushButton *allNotesButton;
    QPushButton *notificationsButton;
    AuthDialog *authDialog;
    QSystemTrayIcon *trayIcon;
    QTimer *notificationTimer;
    bool isMonthView;
    bool notificationsEnabled;
    
    void updateNotificationsButton();
    void setupNotifications();
};

#endif
