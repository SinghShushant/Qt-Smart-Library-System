#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Apply Dark Theme
    QString darkTheme = R"(
        QWidget { background-color: #2E2E2E; color: #F0F0F0; font-family: 'Helvetica Neue'; }
        QLineEdit, QTextBrowser { background-color: #3C3C3C; border: 1px solid #555555; border-radius: 5px; padding: 8px; font-size: 14px; }
        QPushButton { background-color: #555555; border: none; border-radius: 5px; padding: 8px 16px; font-size: 14px; text-align: left; }
        QPushButton:hover { background-color: #6A6A6A; } QPushButton:pressed { background-color: #4A4A4A; }
        QLabel { background-color: transparent; }
    )";
    this->setStyleSheet(darkTheme);
    this->setWindowTitle("Smart Library");

    // Set up the path for our CSV file in a standard application data location
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    m_csvFilePath = dataPath + "/library_data.csv";

    // Load data from the CSV file when the application starts
    loadLibraryFromCsv();
    appendToChat("🤖 Chatbot", QString("Welcome! Loaded %1 books from file.").arg(m_library.size()), false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadLibraryFromCsv()
{
    QFile file(m_csvFilePath);
    // On first run, copy the file from the Desktop to our app data folder
    if (!file.exists()) {
        QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/library_data.csv";
        if (QFile::exists(desktopPath)) {
            QFile::copy(desktopPath, m_csvFilePath);
        } else {
            return; // No data file to load
        }
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    if (!in.atEnd()) in.readLine(); // Skip the header row

    m_library.clear(); // Clear any existing data before loading
    while (!in.atEnd()) {
        QStringList fields = in.readLine().split(',');
        if (fields.size() == 5) {
            Book book;
            book.id = fields[0].toInt();
            book.title = fields[1];
            book.author = fields[2];
            book.isBorrowed = (fields[3] == "1");
            book.borrowCount = fields[4].toInt();
            m_library.append(book);
        }
    }
    file.close();
}

void MainWindow::saveLibraryToCsv()
{
    QFile file(m_csvFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) return;

    QTextStream out(&file);
    // Write the header
    out << "id,title,author,isBorrowed,borrowCount\n";

    // Write all books from the m_library vector to the file
    for (const Book &book : m_library) {
        out << book.id << "," << book.title << "," << book.author << ","
            << (book.isBorrowed ? "1" : "0") << "," << book.borrowCount << "\n";
    }
    file.close();
}

void MainWindow::appendToChat(const QString& sender, const QString& message, bool isUser) {
    QString html = isUser ? QString("<p style='color:#a0e0ff;'><b>%1:</b> %2</p>").arg(sender, message) : QString("<p style='color:#F0F0F0;'><b>%1:</b> %2</p>").arg(sender, message);
    ui->chatDisplayBrowser->append(html);
}

// --- All functions that change the library data now call saveLibraryToCsv() ---

void MainWindow::on_addBookButton_clicked()
{
    QString title = ui->titleLineEdit->text();
    QString author = ui->authorLineEdit->text();
    if (title.isEmpty() || author.isEmpty()) {
        appendToChat("🤖 Chatbot", "Please provide a title and author.", false);
        return;
    }
    int maxId = 0;
    for(const auto& book : m_library) {
        if(book.id > maxId) maxId = book.id;
    }
    Book newBook;
    newBook.id = maxId + 1;
    newBook.title = title;
    newBook.author = author;
    m_library.append(newBook);
    appendToChat("🤖 Chatbot", QString("Success! Added '%1' to the library.").arg(title), false);
    ui->titleLineEdit->clear();
    ui->authorLineEdit->clear();
    ui->bookIdLineEdit->clear();
    saveLibraryToCsv(); // Save the new book to the file
}

void MainWindow::on_borrowBookButton_clicked()
{
    bool ok;
    int id = ui->bookIdLineEdit->text().toInt(&ok);
    if (!ok) return;
    for (Book &book : m_library) {
        if (book.id == id) {
            if (!book.isBorrowed) {
                book.isBorrowed = true;
                book.borrowCount++;
                appendToChat("🤖 Chatbot", QString("You have borrowed '%1'.").arg(book.title), false);
                saveLibraryToCsv(); // Save the updated status
            } else {
                appendToChat("🤖 Chatbot", QString("Sorry, '%1' is already borrowed.").arg(book.title), false);
            }
            return;
        }
    }
}

void MainWindow::on_returnBookButton_clicked()
{
    bool ok;
    int id = ui->bookIdLineEdit->text().toInt(&ok);
    if (!ok) return;
    for (Book &book : m_library) {
        if (book.id == id) {
            if (book.isBorrowed) {
                book.isBorrowed = false;
                appendToChat("🤖 Chatbot", QString("Thank you for returning '%1'.").arg(book.title), false);
                saveLibraryToCsv(); // Save the updated status
            } else {
                appendToChat("🤖 Chatbot", QString("Error: '%1' is already in the library.").arg(book.title), false);
            }
            return;
        }
    }
}

// --- Read-only functions do not need to call saveLibraryToCsv() ---

void MainWindow::on_showAllBooksButton_clicked()
{
    if (m_library.isEmpty()) { appendToChat("🤖 Chatbot", "The library is currently empty.", false); return; }
    QString bookListHtml = "<p>--- <b>Library Inventory</b> ---</p><ul>";
    for (const Book &book : m_library) {
        QString status = book.isBorrowed ? "<b style='color:#ff7b7b;'>Borrowed</b>" : "<b style='color:#7dff7d;'>Available</b>";
        bookListHtml += QString("<li><b>%1</b> by %2 (ID: %3) - %4</li>").arg(book.title, book.author, QString::number(book.id), status);
    }
    bookListHtml += "</ul>";
    ui->chatDisplayBrowser->append(bookListHtml);
}

void MainWindow::on_chatLineEdit_returnPressed()
{
    QString userInput = ui->chatLineEdit->text();
    if (userInput.isEmpty()) return;
    appendToChat("You", userInput, true);
    ui->chatLineEdit->clear();
    QString lowerInput = userInput.toLower();
    if (lowerInput.contains("show all")) {
        on_showAllBooksButton_clicked();
    } else if (lowerInput.startsWith("search for ")) {
        QString searchTerm = userInput.mid(11).trimmed();
        bool found = false;
        for (const Book &book : m_library) {
            if (book.title.toLower().contains(searchTerm)) {
                appendToChat("🤖 Chatbot", QString("Found: '%1' by %2.").arg(book.title, book.author), false);
                found = true;
            }
        }
        if (!found) { appendToChat("🤖 Chatbot", "Sorry, no matching books found.", false); }
    } else {
        appendToChat("🤖 Chatbot", "I can help with 'show all' or 'search for [title]'.", false);
    }
}
