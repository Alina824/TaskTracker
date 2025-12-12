#include "mainwindow.h"
#include "database.h"
#include "taskdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QLabel>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QStyle>
#include <QSettings>
#include <QSet>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isMonthView(false)
{
    setWindowTitle("Планировщик задач");
    resize(1200, 700);
    
    Database& db = Database::instance();
    if (!db.initialize()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось инициализировать базу данных");
    }
    
    QSettings settings;
    notificationsEnabled = settings.value("notificationsEnabled", false).toBool();
    
    setupUI();
    updateAuthButton();
    updateNotificationsButton();
    setupNotifications();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget;
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    QHBoxLayout *topLayout = new QHBoxLayout;
    trackersButton = new QPushButton("Трекеры");
    topLayout->addWidget(trackersButton);
    notesButton = new QPushButton("Заметки");
    topLayout->addWidget(notesButton);
    
    topLayout->addStretch();
    
    monthButton = new QPushButton("Месяц");
    topLayout->addWidget(monthButton);
    
    authButton = new QPushButton("Войти");
    topLayout->addWidget(authButton);
    mainLayout->addLayout(topLayout);
    
    stackedWidget = new QStackedWidget;
    mainLayout->addWidget(stackedWidget);
    
    QHBoxLayout *bottomLayout = new QHBoxLayout;
    allNotesButton = new QPushButton("Все заметки");
    bottomLayout->addWidget(allNotesButton);
    bottomLayout->addStretch();
    notificationsButton = new QPushButton(notificationsEnabled ? "Уведомления" : "Уведомления");
    bottomLayout->addWidget(notificationsButton);
    mainLayout->addLayout(bottomLayout);
    
    weekView = new WeekView;
    stackedWidget->addWidget(weekView);
    
    monthView = new MonthView;
    stackedWidget->addWidget(monthView);
    
    dayView = new DayView;
    stackedWidget->addWidget(dayView);
    
    notesView = new NotesView;
    stackedWidget->addWidget(notesView);
    
    noteEditView = new NoteEditView;
    stackedWidget->addWidget(noteEditView);
    
    allNotesView = new AllNotesView;
    stackedWidget->addWidget(allNotesView);
    
    trackersView = new TrackersView;
    stackedWidget->addWidget(trackersView);
    
    authDialog = new AuthDialog(this);
    
    connect(authButton, &QPushButton::clicked, this, &MainWindow::onAuthButtonClicked);
    connect(monthButton, &QPushButton::clicked, this, &MainWindow::onMonthButtonClicked);
    connect(trackersButton, &QPushButton::clicked, this, &MainWindow::onTrackersButtonClicked);
    connect(notesButton, &QPushButton::clicked, this, &MainWindow::onNotesButtonClicked);
    connect(notificationsButton, &QPushButton::clicked, this, &MainWindow::onNotificationsButtonClicked);
    connect(authDialog, &AuthDialog::loginRequested, this, &MainWindow::onLoginRequested);
    connect(authDialog, &AuthDialog::registerRequested, this, &MainWindow::onRegisterRequested);
    connect(weekView, &WeekView::dayClicked, this, &MainWindow::onDayClicked);
    connect(weekView, &WeekView::addTaskRequested, this, &MainWindow::onAddTaskRequested);
    connect(monthView, &MonthView::dayClicked, this, &MainWindow::onDayClicked);
    connect(monthView, &MonthView::addTaskRequested, this, &MainWindow::onAddTaskRequested);
    connect(dayView, &DayView::backRequested, this, &MainWindow::onDayViewBack);
    connect(dayView, &DayView::taskAdded, this, &MainWindow::onTaskAdded);
    connect(notesView, &NotesView::noteClicked, this, &MainWindow::onNoteClicked);
    connect(notesView, &NotesView::backRequested, this, &MainWindow::onNotesViewBack);
    connect(noteEditView, &NoteEditView::backRequested, this, &MainWindow::onNoteEditBack);
    connect(allNotesView, &AllNotesView::backRequested, this, &MainWindow::onAllNotesViewBack);
    connect(allNotesButton, &QPushButton::clicked, this, &MainWindow::onAllNotesButtonClicked);
    connect(trackersView, &TrackersView::backRequested, this, &MainWindow::showWeekView);
    
    showWeekView();
}

void MainWindow::updateAuthButton()
{
    Database& db = Database::instance();
    if (db.isLoggedIn()) {
        authButton->setText("Выйти");
    } else {
        authButton->setText("Войти");
    }
    weekView->refreshWeek();
}

void MainWindow::onAuthButtonClicked()
{
    Database& db = Database::instance();
    if (db.isLoggedIn()) {
        db.logout();
        updateAuthButton();
        showWeekView();
    } else {
        authDialog->show();
    }
}

void MainWindow::onLoginRequested(const QString& username, const QString& password)
{
    Database& db = Database::instance();
    
    if (db.authenticateUser(username, password)) {

        authDialog->hide();
        updateAuthButton();
        weekView->refreshWeek();
    } else {

        QMessageBox::warning(this, "Ошибка", "Неверное имя пользователя или пароль");
    }
}

void MainWindow::onRegisterRequested(const QString& username, const QString& password)
{
    Database& db = Database::instance();
    
    if (db.createUser(username, password)) {

        if (db.authenticateUser(username, password)) {
            authDialog->hide();
            updateAuthButton();
            weekView->refreshWeek();
            QMessageBox::information(this, "Успех", "Регистрация прошла успешно");
        }
    } else {

        QMessageBox::warning(this, "Ошибка", "Не удалось создать пользователя. Возможно, имя уже занято.");
    }
}

void MainWindow::onDayClicked(const QDate& date)
{
    showDayView(date);
}

void MainWindow::onAddTaskRequested(const QDate& date)
{
    Database& db = Database::instance();
    if (!db.isLoggedIn()) {
        QMessageBox::information(this, "Авторизация", "Пожалуйста, войдите в систему");
        return;
    }
    
    if (date < QDate::currentDate()) {
        QMessageBox::warning(this, "Ошибка", "Нельзя добавить задачу на прошедшую дату.");
        return;
    }
    
    TaskDialog dialog(date, this);
    if (dialog.exec() == QDialog::Accepted) {
        if (db.createTask(db.getCurrentUserId(), dialog.getTitle(), 
                         dialog.getDateTime(), dialog.isTimeBound(), 
                         dialog.getRecurrence())) {
            weekView->refreshWeek();
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось создать задачу");
        }
    }
}

void MainWindow::onDayViewBack()
{
    showWeekView();
}

void MainWindow::onTaskAdded(const QDate& date)
{
    weekView->refreshWeek();
    if (isMonthView) {
        monthView->refreshMonth();
    }
}

void MainWindow::showWeekView()
{
    stackedWidget->setCurrentWidget(weekView);
    weekView->refreshWeek();
}

void MainWindow::showDayView(const QDate& date)
{
    Database& db = Database::instance();
    
    if (!db.isLoggedIn()) {
        QMessageBox::information(this, "Авторизация", "Пожалуйста, войдите в систему");
        return;
    }
    
    dayView->setDate(date);
    stackedWidget->setCurrentWidget(dayView);
}

void MainWindow::onMonthButtonClicked()
{
    if (isMonthView) {
        isMonthView = false;
        monthButton->setText("Месяц");
        showWeekView();
    } else {
        isMonthView = true;
        monthButton->setText("Неделя");
        showMonthView();
    }
}

void MainWindow::showMonthView()
{
    stackedWidget->setCurrentWidget(monthView);
    monthView->refreshMonth();
}

void MainWindow::setupNotifications()
{
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(this->style()->standardIcon(QStyle::SP_ComputerIcon));
    trayIcon->setVisible(true);
    
    notificationTimer = new QTimer(this);
    connect(notificationTimer, &QTimer::timeout, this, &MainWindow::checkNotifications);
    
    if (notificationsEnabled) {
        notificationTimer->start(10000);
    }
}

void MainWindow::updateNotificationsButton()
{
    if (notificationsEnabled) {
        notificationsButton->setText("Уведомления");
    } else {
        notificationsButton->setText("Уведомления");
    }
}

void MainWindow::onNotificationsButtonClicked()
{
    notificationsEnabled = !notificationsEnabled;
    
    QSettings settings;
    settings.setValue("notificationsEnabled", notificationsEnabled);
    
    updateNotificationsButton();
    
    if (notificationsEnabled) {
        notificationTimer->start(10000);
    } else {
        notificationTimer->stop();
    }
}

void MainWindow::checkNotifications()
{
    if (!notificationsEnabled) {
        return;
    }
    
    Database& db = Database::instance();
    if (!db.isLoggedIn()) {
        return;
    }
    
    QDate today = QDate::currentDate();
    QTime currentTime = QTime::currentTime();
    
    auto tasks = db.getTasksForDay(db.getCurrentUserId(), today);
    
    for (const auto& task : tasks) {
        bool isTimeBound = task["is_time_bound"].toBool();
        
        if (!isTimeBound) {
            continue;
        }
        
        if (task["task_time"].isNull()) {
            continue;
        }
        
        QTime taskTime = task["task_time"].toTime();
        QString title = task["title"].toString();
        
        int taskTotalSeconds = taskTime.hour() * 3600 + taskTime.minute() * 60;
        int currentTotalSeconds = currentTime.hour() * 3600 + currentTime.minute() * 60 + currentTime.second();
        
        int diff = currentTotalSeconds - taskTotalSeconds;
        if (diff >= 0 && diff <= 10) {

            static QSet<int> notifiedTasks;
            
            int taskId = task["id"].toInt();
            
            if (!notifiedTasks.contains(taskId)) {
                notifiedTasks.insert(taskId);
                
                trayIcon->showMessage(
                    "Напоминание о задаче",
                    title,
                    QSystemTrayIcon::Information,
                    5000
                );
                
                static int lastMinute = -1;
                if (currentTime.minute() != lastMinute) {
                    notifiedTasks.clear();
                    lastMinute = currentTime.minute();
                }
            }
        }
    }
}

void MainWindow::showNotesView()
{
    stackedWidget->setCurrentWidget(notesView);
    notesView->refreshNotes();
}

void MainWindow::showNoteEditView(int noteId)
{
    noteEditView->loadNote(noteId);
    stackedWidget->setCurrentWidget(noteEditView);
}

void MainWindow::onNotesButtonClicked()
{
    if (!Database::instance().isLoggedIn()) {
        QMessageBox::information(this, "Информация", "Необходимо войти в систему");
        return;
    }
    showNotesView();
}

void MainWindow::onNoteClicked(int noteId)
{
    showNoteEditView(noteId);
}

void MainWindow::onNotesViewBack()
{
    showWeekView();
}

void MainWindow::onNoteEditBack()
{
    showNotesView();
}

void MainWindow::showAllNotesView()
{
    stackedWidget->setCurrentWidget(allNotesView);
    allNotesView->refreshNotes();
}

void MainWindow::onAllNotesButtonClicked()
{
    if (!Database::instance().isLoggedIn()) {
        QMessageBox::information(this, "Информация", "Необходимо войти в систему");
        return;
    }
    showAllNotesView();
}

void MainWindow::onAllNotesViewBack()
{
    showWeekView();
}

void MainWindow::showTrackersView()
{
    stackedWidget->setCurrentWidget(trackersView);
    trackersView->refreshTrackers();
}

void MainWindow::onTrackersButtonClicked()
{
    if (!Database::instance().isLoggedIn()) {
        QMessageBox::information(this, "Информация", "Необходимо войти в систему");
        return;
    }
    showTrackersView();
}
