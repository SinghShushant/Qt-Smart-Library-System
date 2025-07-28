#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include "book.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_addBookButton_clicked();
    void on_borrowBookButton_clicked();
    void on_returnBookButton_clicked();
    void on_showAllBooksButton_clicked();
    void on_chatLineEdit_returnPressed();

private:
    void appendToChat(const QString& sender, const QString& message, bool isUser);
    void saveLibraryToCsv();
    void loadLibraryFromCsv();

    Ui::MainWindow *ui;
    QVector<Book> m_library;
    QString m_csvFilePath;
};
#endif // MAINWINDOW_H
