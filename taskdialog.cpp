#include "taskdialog.h"
#include <QTime>
#include <QMessageBox>

TaskDialog::TaskDialog(const QDate& defaultDate, QWidget *parent)
    : QDialog(parent), defaultDate(defaultDate)
{
    setupUI();
    
    if (defaultDate < QDate::currentDate()) {

        QMessageBox::warning(this, "Ошибка", "Нельзя добавить задачу на прошедшую дату.");
        reject();
        return;
    }
    
    dateTimeEdit->setDate(defaultDate);
    
    if (defaultDate == QDate::currentDate()) {
        QTime currentTime = QTime::currentTime();
        QTime defaultTime = QTime(9, 0);

        QTime timeToSet = (defaultTime < currentTime) ? currentTime.addSecs(3600) : defaultTime;
        dateTimeEdit->setTime(timeToSet);
    } else {
        dateTimeEdit->setTime(QTime(9, 0));
    }
    
    onTimeBoundChanged();
}

TaskDialog::TaskDialog(int taskId, const QString& title, const QDateTime& dateTime,
                       bool isTimeBound, const QString& recurrence, QWidget *parent)
    : QDialog(parent), defaultDate(dateTime.date())
{
    Q_UNUSED(taskId);
    setupUI();
    titleEdit->setText(title);
    dateTimeEdit->setDateTime(dateTime);
    timeBoundCheckBox->setChecked(!isTimeBound);
    onTimeBoundChanged();
    
    if (!recurrence.isEmpty()) {
        recurringCheckBox->setChecked(true);
        onRecurringChanged();
        int index = recurrenceCombo->findText(recurrence);
        if (index >= 0) {
            recurrenceCombo->setCurrentIndex(index);
        }
    }
}

void TaskDialog::setupUI()
{
    setWindowTitle("Добавить задачу");
    setModal(true);
    resize(400, 300);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    layout->addWidget(new QLabel("Название задачи:"));
    titleEdit = new QLineEdit;
    layout->addWidget(titleEdit);
    
    layout->addWidget(new QLabel("Дата:"));
    dateTimeEdit = new QDateTimeEdit;
    dateTimeEdit->setCalendarPopup(true);
    dateTimeEdit->setDisplayFormat("dd.MM.yyyy HH:mm");

    dateTimeEdit->setMinimumDate(QDate::currentDate());

    layout->addWidget(dateTimeEdit);
    
    timeBoundCheckBox = new QCheckBox("Не привязана ко времени");
    connect(timeBoundCheckBox, &QCheckBox::toggled, this, &TaskDialog::onTimeBoundChanged);
    layout->addWidget(timeBoundCheckBox);
    
    recurringCheckBox = new QCheckBox("Повторяющаяся задача");
    connect(recurringCheckBox, &QCheckBox::toggled, this, &TaskDialog::onRecurringChanged);
    layout->addWidget(recurringCheckBox);
    
    recurrenceCombo = new QComboBox;
    recurrenceCombo->addItems({"Раз в час", "Раз в 2 часа", "Раз в сутки", "Раз в неделю", "Раз в месяц"});
    recurrenceCombo->setEnabled(false);
    layout->addWidget(recurrenceCombo);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    okButton = new QPushButton("OK");
    cancelButton = new QPushButton("Отмена");
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    layout->addStretch();
    layout->addLayout(buttonLayout);
    
    connect(okButton, &QPushButton::clicked, this, &TaskDialog::onOkClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void TaskDialog::onTimeBoundChanged()
{
    bool isNotTimeBound = timeBoundCheckBox->isChecked();
    QTime currentTime = dateTimeEdit->time();
    QDate selectedDate = dateTimeEdit->date();
    
    if (isNotTimeBound) {

        dateTimeEdit->setDisplayFormat("dd.MM.yyyy");

        dateTimeEdit->setMinimumTime(QTime());
        dateTimeEdit->setTime(QTime());
    } else {

        dateTimeEdit->setDisplayFormat("dd.MM.yyyy HH:mm");

        if (currentTime.isNull() || !currentTime.isValid()) {
            if (selectedDate == QDate::currentDate()) {
                QTime defaultTime = QTime(9, 0);
                QTime currentTimeNow = QTime::currentTime();

                QTime timeToSet = (defaultTime < currentTimeNow) ? currentTimeNow.addSecs(3600) : defaultTime;
                dateTimeEdit->setTime(timeToSet);
            } else {
                dateTimeEdit->setTime(QTime(9, 0));
            }
        }

        if (selectedDate == QDate::currentDate()) {
            QTime currentTimeNow = QTime::currentTime();
            dateTimeEdit->setMinimumTime(currentTimeNow);
        } else {
            dateTimeEdit->setMinimumTime(QTime());
        }
    }
}

void TaskDialog::onRecurringChanged()
{
    recurrenceCombo->setEnabled(recurringCheckBox->isChecked());
}

void TaskDialog::onOkClicked()
{
    QDate selectedDate = dateTimeEdit->date();
    bool isNotTimeBound = timeBoundCheckBox->isChecked();
    
    if (selectedDate < QDate::currentDate()) {
        QMessageBox::warning(this, "Ошибка", "Нельзя добавить задачу на прошедшую дату.");
        return;
    }
    
    if (!isNotTimeBound) {
        QTime selectedTime = dateTimeEdit->time();
        QDateTime selectedDateTime = QDateTime(selectedDate, selectedTime);
        QDateTime currentDateTime = QDateTime::currentDateTime();
        
        if (selectedDateTime < currentDateTime) {
            QMessageBox::warning(this, "Ошибка", "Нельзя добавить задачу на прошедшую дату или время.");
            return;
        }
    }
    
    accept();
}

QDateTime TaskDialog::getDateTime() const
{
    if (isTimeBound()) {
        return dateTimeEdit->dateTime();
    } else {
        return QDateTime(dateTimeEdit->date(), QTime());
    }
}

QString TaskDialog::getRecurrence() const
{
    if (recurringCheckBox->isChecked()) {
        return recurrenceCombo->currentText();
    }
    return QString();
}
