#include "allnotesview.h"
#include "database.h"
#include <QLabel>
#include <QFrame>
#include <QDateTime>

AllNotesView::AllNotesView(QWidget *parent)
    : QWidget(parent)
{

    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QHBoxLayout *headerLayout = new QHBoxLayout;
    backButton = new QPushButton("← Назад");
    QLabel *titleLabel = new QLabel("Все заметки");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    headerLayout->addWidget(backButton);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    
    layout->addLayout(headerLayout);
    
    scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    contentWidget = new QWidget;
    contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setAlignment(Qt::AlignTop);
    contentLayout->setSpacing(15);
    contentLayout->setContentsMargins(10, 10, 10, 10);
    
    scrollArea->setWidget(contentWidget);
    layout->addWidget(scrollArea);
    
    connect(backButton, &QPushButton::clicked, this, &AllNotesView::onBackClicked);
}

void AllNotesView::refreshNotes()
{

    QLayoutItem* item;
    while ((item = contentLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    Database& db = Database::instance();
    if (!db.isLoggedIn()) {
        QLabel *noAuthLabel = new QLabel("Необходимо войти в систему");
        noAuthLabel->setAlignment(Qt::AlignCenter);
        contentLayout->addWidget(noAuthLabel);
        return;
    }
    
    int userId = db.getCurrentUserId();
    auto notes = db.getNotes(userId);
    
    if (notes.isEmpty()) {
        QLabel *noNotesLabel = new QLabel("Заметок пока нет");
        noNotesLabel->setAlignment(Qt::AlignCenter);
        noNotesLabel->setStyleSheet("font-size: 14px; color: #666;");
        contentLayout->addWidget(noNotesLabel);
        return;
    }
    
    for (const auto& note : notes) {

        QFrame *noteFrame = new QFrame;
        noteFrame->setFrameStyle(QFrame::Box);
        noteFrame->setStyleSheet("QFrame { background-color: #f9f9f9; border: 1px solid #ddd; border-radius: 5px; padding: 10px; }");
        
        QVBoxLayout *noteLayout = new QVBoxLayout(noteFrame);
        noteLayout->setSpacing(5);
        
        QString name = note["name"].toString();
        QLabel *nameLabel = new QLabel(name);
        nameLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #333;");
        noteLayout->addWidget(nameLabel);
        
        QString content = note["content"].toString();
        if (content.isEmpty()) {
            content = "(пусто)";
        }
        QTextEdit *contentEdit = new QTextEdit;
        contentEdit->setPlainText(content);
        contentEdit->setReadOnly(true);
        contentEdit->setMaximumHeight(200);
        contentEdit->setStyleSheet("QTextEdit { background-color: white; border: 1px solid #ccc; }");
        noteLayout->addWidget(contentEdit);
        
        QDateTime createdAt = note["created_at"].toDateTime();
        QDateTime updatedAt = note["updated_at"].toDateTime();
        QString dateText = "Создано: " + createdAt.toString("dd.MM.yyyy HH:mm");
        if (updatedAt != createdAt) {
            dateText += " | Обновлено: " + updatedAt.toString("dd.MM.yyyy HH:mm");
        }
        QLabel *dateLabel = new QLabel(dateText);
        dateLabel->setStyleSheet("font-size: 10px; color: #888;");
        noteLayout->addWidget(dateLabel);
        
        QFrame *separator = new QFrame;
        separator->setFrameShape(QFrame::HLine);
        separator->setStyleSheet("color: #ddd;");
        noteLayout->addWidget(separator);
        
        contentLayout->addWidget(noteFrame);
    }
    
    contentLayout->addStretch();
}

void AllNotesView::onBackClicked()
{
    emit backRequested();
}
