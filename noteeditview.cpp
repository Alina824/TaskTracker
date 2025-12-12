#include "noteeditview.h"
#include "database.h"
#include <QMessageBox>
#include <QTimer>

NoteEditView::NoteEditView(int noteId, QWidget *parent)
    : QWidget(parent), currentNoteId(noteId), isSaving(false)
{

    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QHBoxLayout *headerLayout = new QHBoxLayout;
    backButton = new QPushButton("← Назад");
    QLabel *titleLabel = new QLabel("Редактирование заметки");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    headerLayout->addWidget(backButton);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    
    layout->addLayout(headerLayout);
    
    QLabel *nameLabel = new QLabel("Имя заметки:");
    nameEdit = new QLineEdit;
    layout->addWidget(nameLabel);
    layout->addWidget(nameEdit);
    
    QLabel *contentLabel = new QLabel("Содержимое:");
    contentEdit = new QTextEdit;
    contentEdit->setAcceptRichText(false);
    layout->addWidget(contentLabel);
    layout->addWidget(contentEdit);
    
    connect(backButton, &QPushButton::clicked, this, &NoteEditView::onBackClicked);
    connect(nameEdit, &QLineEdit::textChanged, this, &NoteEditView::onNameChanged);
    connect(contentEdit, &QTextEdit::textChanged, this, &NoteEditView::onContentChanged);
    
    if (noteId > 0) {
        loadNote(noteId);
    }
}

void NoteEditView::loadNote(int noteId)
{
    currentNoteId = noteId;
    
    Database& db = Database::instance();
    if (!db.isLoggedIn()) {
        return;
    }
    
    auto note = db.getNote(noteId);
    if (note.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заметка не найдена");
        emit backRequested();
        return;
    }
    
    int userId = db.getCurrentUserId();
    if (note["user_id"].toInt() != userId) {
        QMessageBox::warning(this, "Ошибка", "Нет доступа к этой заметке");
        emit backRequested();
        return;
    }
    
    nameEdit->setText(note["name"].toString());
    contentEdit->setPlainText(note["content"].toString());
}

void NoteEditView::onBackClicked()
{
    saveNote();
    emit backRequested();
}

void NoteEditView::onNameChanged()
{
    if (currentNoteId > 0 && !isSaving) {

        QTimer::singleShot(500, this, &NoteEditView::saveNote);
    }
}

void NoteEditView::onContentChanged()
{
    if (currentNoteId > 0 && !isSaving) {

        QTimer::singleShot(1000, this, &NoteEditView::saveNote);
    }
}

void NoteEditView::saveNote()
{
    if (currentNoteId <= 0 || isSaving) {
        return;
    }
    
    Database& db = Database::instance();
    if (!db.isLoggedIn()) {
        return;
    }
    
    QString name = nameEdit->text().trimmed();
    if (name.isEmpty()) {
        return;
    }
    
    QString content = contentEdit->toPlainText();
    
    isSaving = true;
    bool success = db.updateNote(currentNoteId, name, content);
    isSaving = false;
    
    if (!success) {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить заметку");
    }
}
