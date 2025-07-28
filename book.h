#ifndef BOOK_H
#define BOOK_H
#include <QString>

// This struct defines the data we will store for each book.
struct Book {
    int id;
    QString title;
    QString author;
    bool isBorrowed = false;
    int borrowCount = 0;
};

#endif // BOOK_H
