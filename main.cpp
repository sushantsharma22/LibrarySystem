#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <limits>

class Book {
private:
    int id;
    std::string title;
    std::string author;
    bool isBorrowed;

public:
    // Default constructor needed for operator[]
    Book() : id(0), title(""), author(""), isBorrowed(false) {}

    Book(int id, const std::string &title, const std::string &author)
        : id(id), title(title), author(author), isBorrowed(false) {}

    int getId() const { return id; }
    const std::string &getTitle() const { return title; }
    const std::string &getAuthor() const { return author; }
    bool getIsBorrowed() const { return isBorrowed; }

    void borrowBook() {
        if (isBorrowed) throw std::runtime_error("Book already borrowed.");
        isBorrowed = true;
    }

    void returnBook() {
        if (!isBorrowed) throw std::runtime_error("Book was not borrowed.");
        isBorrowed = false;
    }

    void setBorrowed(bool val) {
        isBorrowed = val;
    }
};

class Member {
private:
    int memberId;
    std::string name;
    std::vector<int> borrowedBooks;

public:
    // Default constructor needed for operator[]
    Member() : memberId(0), name("") {}

    Member(int memberId, const std::string &name)
        : memberId(memberId), name(name) {}

    int getMemberId() const { return memberId; }
    const std::string &getName() const { return name; }

    void borrowBook(int bookId) {
        borrowedBooks.push_back(bookId);
    }

    void returnBook(int bookId) {
        for (auto it = borrowedBooks.begin(); it != borrowedBooks.end(); ++it) {
            if (*it == bookId) {
                borrowedBooks.erase(it);
                return;
            }
        }
        throw std::runtime_error("Member did not borrow this book.");
    }

    const std::vector<int> &getBorrowedBooks() const {
        return borrowedBooks;
    }
};

class Library {
private:
    std::unordered_map<int, Book> books;
    std::unordered_map<int, Member> members;
    int nextBookId;
    int nextMemberId;

public:
    Library() : nextBookId(1000), nextMemberId(5000) {}

    int addBook(const std::string &title, const std::string &author) {
        int id = nextBookId++;
        books.emplace(id, Book(id, title, author));
        return id;
    }

    int addMember(const std::string &name) {
        int id = nextMemberId++;
        members.emplace(id, Member(id, name));
        return id;
    }

    void borrowBook(int memberId, int bookId) {
        auto mIt = members.find(memberId);
        if (mIt == members.end()) throw std::runtime_error("Member not found.");

        auto bIt = books.find(bookId);
        if (bIt == books.end()) throw std::runtime_error("Book not found.");

        bIt->second.borrowBook();
        mIt->second.borrowBook(bookId);
    }

    void returnBook(int memberId, int bookId) {
        auto mIt = members.find(memberId);
        if (mIt == members.end()) throw std::runtime_error("Member not found.");

        auto bIt = books.find(bookId);
        if (bIt == books.end()) throw std::runtime_error("Book not found.");

        bIt->second.returnBook();
        mIt->second.returnBook(bookId);
    }

    void printAllBooks() const {
        if (books.empty()) {
            std::cout << "No books in the library.\n";
            return;
        }
        std::cout << "Books:\n";
        for (const auto &kv : books) {
            std::cout << "ID: " << kv.second.getId()
                      << " | Title: " << kv.second.getTitle()
                      << " | Author: " << kv.second.getAuthor()
                      << " | Borrowed: " << (kv.second.getIsBorrowed() ? "Yes" : "No") << "\n";
        }
    }

    void printAllMembers() const {
        if (members.empty()) {
            std::cout << "No members.\n";
            return;
        }
        std::cout << "Members:\n";
        for (const auto &kv : members) {
            std::cout << "Member ID: " << kv.second.getMemberId()
                      << " | Name: " << kv.second.getName()
                      << " | Borrowed Book IDs: ";
            const auto &b = kv.second.getBorrowedBooks();
            for (auto id : b) std::cout << id << " ";
            std::cout << "\n";
        }
    }

    void saveData(const std::string &booksFile, const std::string &membersFile) {
        std::ofstream bf(booksFile);
        if (!bf.is_open()) throw std::runtime_error("Failed to open books file for writing.");
        for (auto &kv : books) {
            bf << kv.second.getId() << ","
               << kv.second.getTitle() << ","
               << kv.second.getAuthor() << ","
               << (kv.second.getIsBorrowed() ? "1" : "0") << "\n";
        }
        bf.close();

        std::ofstream mf(membersFile);
        if (!mf.is_open()) throw std::runtime_error("Failed to open members file for writing.");
        for (auto &kv : members) {
            mf << kv.second.getMemberId() << "," << kv.second.getName();
            const auto &b = kv.second.getBorrowedBooks();
            for (auto id : b) mf << "," << id;
            mf << "\n";
        }
        mf.close();
    }

    void loadData(const std::string &booksFile, const std::string &membersFile) {
        books.clear();
        members.clear();

        nextBookId = 1000;
        nextMemberId = 5000;

        std::ifstream bf(booksFile);
        if (bf.is_open()) {
            std::string line;
            while (std::getline(bf, line)) {
                if (line.empty()) continue;
                std::stringstream ss(line);
                std::string idStr, title, author, borrowedStr;

                std::getline(ss, idStr, ',');
                std::getline(ss, title, ',');
                std::getline(ss, author, ',');
                std::getline(ss, borrowedStr, ',');

                int id = std::stoi(idStr);
                bool isB = (borrowedStr == "1");
                Book b(id, title, author);
                b.setBorrowed(isB);

                // Use insert to avoid default construction issues
                books.insert({id, b});

                if (id >= nextBookId) nextBookId = id + 1;
            }
            bf.close();
        }

        std::ifstream mf(membersFile);
        if (mf.is_open()) {
            std::string line;
            while (std::getline(mf, line)) {
                if (line.empty()) continue;
                std::stringstream ss(line);
                std::string memberIdStr, name;
                std::getline(ss, memberIdStr, ',');
                std::getline(ss, name, ',');

                int mid = std::stoi(memberIdStr);
                Member m(mid, name);

                std::string bookIdStr;
                while (std::getline(ss, bookIdStr, ',')) {
                    int bid = std::stoi(bookIdStr);
                    m.borrowBook(bid);
                }

                // Use insert here too
                members.insert({mid, m});

                if (mid >= nextMemberId) nextMemberId = mid + 1;
            }
            mf.close();
        }
    }
};

// Utility function to clear input buffer after invalid input
void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main() {
    Library lib;

    int choice = 0;
    while (true) {
        std::cout << "\n===== Library Management System =====\n";
        std::cout << "1. Add Book\n";
        std::cout << "2. Add Member\n";
        std::cout << "3. Borrow Book\n";
        std::cout << "4. Return Book\n";
        std::cout << "5. List All Books\n";
        std::cout << "6. List All Members\n";
        std::cout << "7. Save Data\n";
        std::cout << "8. Load Data\n";
        std::cout << "9. Exit\n";
        std::cout << "Enter your choice: ";
        if (!(std::cin >> choice)) {
            clearInput();
            std::cout << "Invalid input. Try again.\n";
            continue;
        }

        clearInput(); // Clear remaining input buffer

        try {
            if (choice == 1) {
                std::string title, author;
                std::cout << "Enter book title: ";
                std::getline(std::cin, title);
                std::cout << "Enter author name: ";
                std::getline(std::cin, author);
                int id = lib.addBook(title, author);
                std::cout << "Book added with ID: " << id << "\n";
            } else if (choice == 2) {
                std::string name;
                std::cout << "Enter member name: ";
                std::getline(std::cin, name);
                int id = lib.addMember(name);
                std::cout << "Member added with ID: " << id << "\n";
            } else if (choice == 3) {
                int memberId, bookId;
                std::cout << "Enter member ID: ";
                std::cin >> memberId;
                std::cout << "Enter book ID: ";
                std::cin >> bookId;
                lib.borrowBook(memberId, bookId);
                std::cout << "Book borrowed successfully.\n";
            } else if (choice == 4) {
                int memberId, bookId;
                std::cout << "Enter member ID: ";
                std::cin >> memberId;
                std::cout << "Enter book ID: ";
                std::cin >> bookId;
                lib.returnBook(memberId, bookId);
                std::cout << "Book returned successfully.\n";
            } else if (choice == 5) {
                lib.printAllBooks();
            } else if (choice == 6) {
                lib.printAllMembers();
            } else if (choice == 7) {
                lib.saveData("books.csv", "members.csv");
                std::cout << "Data saved to books.csv and members.csv.\n";
            } else if (choice == 8) {
                lib.loadData("books.csv", "members.csv");
                std::cout << "Data loaded from files.\n";
            } else if (choice == 9) {
                std::cout << "Exiting... Goodbye!\n";
                break;
            } else {
                std::cout << "Invalid choice.\n";
            }
        } catch (const std::exception &ex) {
            std::cout << "Error: " << ex.what() << "\n";
        }
    }

    return 0;
}
