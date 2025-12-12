#ifndef DAYVIEW_H
#define DAYVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QDate>

class DayView : public QWidget
{
    Q_OBJECT

public:
    explicit DayView(QWidget *parent = nullptr);

    void setDate(const QDate& date);
    
    void refreshTasks();

signals:
    void backRequested();
    void taskAdded(const QDate& date);

public slots:
    void onAddTaskClicked();

private slots:
    void onTaskDoubleClicked(QListWidgetItem* item);
    void onDeleteTaskClicked();
    void onMoveTaskClicked();

private:
    QDate currentDate;
    QLabel *dateLabel;
    QListWidget *taskList;
    QPushButton *addButton;
    QPushButton *backButton;
    
    QString formatRecurrence(const QString& recurrence) const;
};

#endif
