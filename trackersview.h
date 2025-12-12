#ifndef TRACKERSVIEW_H
#define TRACKERSVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QDate>

class TrackersView : public QWidget
{
    Q_OBJECT

public:
    
    explicit TrackersView(QWidget *parent = nullptr);
    
    void refreshTrackers();

signals:
    
    void backRequested();

public slots:
    
    void onAddHabitClicked();
    
    void onDeleteHabitClicked();

private slots:
    
    void onDayCellClicked(int habitId, const QDate& date);

private:
    
    void createHabitRow(int habitId, const QString& habitName);
    
    int calculateStreak(int habitId) const;
    
    QScrollArea *scrollArea;
    QWidget *contentWidget;
    QVBoxLayout *contentLayout;
    QPushButton *backButton;
    QPushButton *addButton;
    QPushButton *deleteButton;
    
    struct HabitRow {
        QFrame *rowFrame;
        QHBoxLayout *rowLayout;
        QLabel *nameLabel;
        QVector<QPushButton*> dayButtons;
        QLabel *streakLabel;
        int habitId;
    };
    
    QVector<HabitRow> habitRows;
};

#endif
