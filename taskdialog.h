#ifndef TASKDIALOG_H
#define TASKDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class TaskDialog : public QDialog
{
    Q_OBJECT

public:
    
    explicit TaskDialog(const QDate& defaultDate, QWidget *parent = nullptr);
    
    explicit TaskDialog(int taskId, const QString& title, const QDateTime& dateTime,
                       bool isTimeBound, const QString& recurrence, QWidget *parent = nullptr);
    
    QString getTitle() const { return titleEdit->text(); }
    
    QDateTime getDateTime() const;
    
    bool isTimeBound() const { return !timeBoundCheckBox->isChecked(); }
    
    QString getRecurrence() const;

private slots:
    void onTimeBoundChanged();
    void onRecurringChanged();
    void onOkClicked();

private:
    void setupUI();
    
    QLineEdit *titleEdit;
    QDateTimeEdit *dateTimeEdit;
    QCheckBox *timeBoundCheckBox;
    QCheckBox *recurringCheckBox;
    QComboBox *recurrenceCombo;
    QPushButton *okButton;
    QPushButton *cancelButton;
    
    QDate defaultDate;
};

#endif
