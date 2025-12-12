#include "notesview.h"
#include "database.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>

NotesView::NotesView(QWidget *parent)
    : QWidget(parent)
{

    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QHBoxLayout *headerLayout = new QHBoxLayout;
    backButton = new QPushButton("← Назад");
    QLabel *titleLabel = new QLabel("Заметки");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    headerLayout->addWidget(backButton);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    
    layout->addLayout(headerLayout);
    
    notesList = new QListWidget;
    layout->addWidget(notesList);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    addButton = new QPushButton("+ Добавить заметку");
    deleteButton = new QPushButton("Удалить заметку");
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addStretch();
    
    layout->addLayout(buttonLayout);
    
    connect(backButton, &QPushButton::clicked, this, &NotesView::backRequested);
    connect(addButton, &QPushButton::clicked, this, &NotesView::onAddNoteClicked);
    connect(deleteButton, &QPushButton::clicked, this, &NotesView::onDeleteNoteClicked);
    connect(notesList, &QListWidget::itemDoubleClicked, this, &NotesView::onNoteDoubleClicked);
}

void NotesView::refreshNotes()
{
    notesList->clear();
    
    Database& db = Database::instance();
    if (!db.isLoggedIn()) {
        return;
    }
    
    int userId = db.getCurrentUserId();
    auto notes = db.getNotes(userId);
    
    for (const auto& note : notes) {
        QString name = note["name"].toString();
        QListWidgetItem *item = new QListWidgetItem(name);
        item->setData(Qt::UserRole, note["id"].toInt());
        notesList->addItem(item);
    }
}

void NotesView::onAddNoteClicked()
{
    Database& db = Database::instance();
    if (!db.isLoggedIn()) {
        QMessageBox::warning(this, "Ошибка", "Необходимо войти в систему");
        return;
    }
    
    bool ok;
    QString name = QInputDialog::getText(this, "Новая заметка", 
                                         "Введите имя заметки:",
                                         QLineEdit::Normal, "", &ok);
    
    if (!ok || name.isEmpty()) {
        return;
    }
    
    int userId = db.getCurrentUserId();
    int noteId = db.createNote(userId, name);
    
    if (noteId > 0) {
        refreshNotes();

        emit noteClicked(noteId);
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось создать заметку");
    }
}

void NotesView::onDeleteNoteClicked()
{
    QListWidgetItem *selectedItem = notesList->currentItem();
    if (!selectedItem) {
        QMessageBox::information(this, "Информация", "Выберите заметку для удаления");
        return;
    }
    
    int ret = QMessageBox::question(this, "Подтверждение", 
                                     "Вы уверены, что хотите удалить эту заметку?",
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) {
        return;
    }
    
    int noteId = selectedItem->data(Qt::UserRole).toInt();
    
    Database& db = Database::instance();
    if (db.deleteNote(noteId)) {
        refreshNotes();
        QMessageBox::information(this, "Успех", "Заметка удалена");
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось удалить заметку");
    }
}

void NotesView::onNoteDoubleClicked(QListWidgetItem* item)
{
    int noteId = item->data(Qt::UserRole).toInt();
    emit noteClicked(noteId);
}
