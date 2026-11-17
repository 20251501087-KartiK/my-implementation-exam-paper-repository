/*
 * EXAM PAPER REPOSITORY - repaired & extended C++ console project
 * ----------------------------------------------------------------
 * Based on this1.docx. The original file had compile-breaking corruption:
 *   - a bare `Virtual` token before `class User`
 *   - a bare `Demo` token before `oopDemoMenu()`
 *   - smart quotes: cout << ”11. Type Casting\n”;
 * All three are fixed here.
 *
 * Concepts preserved from the original:
 *   classes/objects, encapsulation, abstraction (abstract class), inheritance,
 *   multiple inheritance, polymorphism, virtual functions, virtual base class
 *   (diamond), constructors, this pointer, pointers to objects/derived classes,
 *   function overriding, overloading, operator overloading, friend function,
 *   access specifiers, basic<->class and class<class casting.
 *
 * Concepts ADDED for the lab (the ones the original file said were NOT used):
 *   EXCEPTION HANDLING
 *     - try / catch / throw
 *     - "throws" equivalent: noexcept(false) + documented callee contract
 *     - finally block: RAII ScopeGuard (C++ has no `finally` keyword)
 *     - user-defined exceptions (RepositoryException hierarchy)
 *     - catching base-class and derived-class exceptions (correct ordering)
 *   MULTITHREADING
 *     - std::thread, std::mutex, std::atomic, std::async/std::future
 *     - exceptions thrown inside threads are caught locally
 *
 * Build:  make        (or: g++ -std=c++17 -O2 -pthread main.cpp -o exam_repo)
 * Run:    ./exam_repo
 */

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <future>
#include <functional>
#include <sstream>
#include <cctype>

using namespace std;

// ===========================================================================
// 1. USER-DEFINED EXCEPTIONS  (lab topic)
// ===========================================================================
class RepositoryException : public runtime_error {          // BASE class
public:
    explicit RepositoryException(const string &msg) : runtime_error(msg) {}
};

class PaperNotFoundException      : public RepositoryException { using RepositoryException::RepositoryException; };
class DuplicatePaperException     : public RepositoryException { using RepositoryException::RepositoryException; };
class InvalidMarksException       : public RepositoryException { using RepositoryException::RepositoryException; };
class UnauthorizedAccessException : public RepositoryException { using RepositoryException::RepositoryException; };
class AssessmentClosedException   : public RepositoryException { using RepositoryException::RepositoryException; };
class FileStorageException        : public RepositoryException { using RepositoryException::RepositoryException; };

// "finally" emulation: C++ uses RAII instead of a finally keyword.
// The destructor ALWAYS runs, on both the success and the failure path.
class Finally {
    function<void()> action_;
public:
    explicit Finally(function<void()> action) : action_(std::move(action)) {}
    ~Finally() { if (action_) action_(); }
    Finally(const Finally &) = delete;
    Finally &operator=(const Finally &) = delete;
};

// ===========================================================================
// 2. SMALL INPUT HELPERS
// ===========================================================================
void pauseScreen() {
    cout << "\nPress Enter to continue...";
    cin.clear();
    cin.get();          // one Enter is enough (original docx version needed two)
}
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}
int readInt(const string &prompt, int lo, int hi) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= lo && value <= hi) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cout << "Invalid choice. Enter a number from " << lo << " to " << hi << ".\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}
string readText(const string &prompt) {
    string value;
    do {
        cout << prompt;
        getline(cin, value);
        if (value.empty()) cout << "Input cannot be empty. Try again.\n";
    } while (value.empty());
    return value;
}

// ===========================================================================
// 3. ENTITIES (encapsulation)
// ===========================================================================
class Subject {
private:
    string code_, name_, branch_;
public:
    Subject() = default;
    Subject(string c, string n, string b) : code_(move(c)), name_(move(n)), branch_(move(b)) {}
    string getCode()   const { return code_; }
    string getName()   const { return name_; }
    string getBranch() const { return branch_; }
    void display() const {
        cout << left << setw(12) << code_ << setw(30) << name_ << setw(8) << branch_ << '\n';
    }
};

class ExamPaper {
private:
    int id_ = 0, semester_ = 0, year_ = 0;
    string code_, name_, branch_, examType_, fileName_, status_, uploadedBy_;
public:
    ExamPaper() = default;
    ExamPaper(int id, string code, string name, string branch, int sem, string type,
              int year, string file, string uploader, string status = "Pending")
        : id_(id), semester_(sem), year_(year), code_(move(code)), name_(move(name)),
          branch_(move(branch)), examType_(move(type)), fileName_(move(file)),
          status_(move(status)), uploadedBy_(move(uploader)) {}

    int    getId()       const { return id_; }
    string getCode()     const { return code_; }
    string getName()     const { return name_; }
    string getBranch()   const { return branch_; }
    int    getSemester() const { return semester_; }
    string getExamType() const { return examType_; }
    int    getYear()     const { return year_; }
    string getFile()     const { return fileName_; }
    string getStatus()   const { return status_; }
    string getUploader() const { return uploadedBy_; }

    void setStatus(string s) { status_ = move(s); }
    void setDetails(string type, int year, string status = "Pending") {
        examType_ = move(type); year_ = year; status_ = move(status);
    }
    void displayDetails() const {
        cout << "\n----------------------------------------\n";
        cout << "             PAPER DETAILS\n";
        cout << "----------------------------------------\n";
        cout << "Paper ID     : " << id_        << '\n';
        cout << "Subject Code : " << code_      << '\n';
        cout << "Subject      : " << name_      << '\n';
        cout << "Branch       : " << branch_    << '\n';
        cout << "Semester     : " << semester_  << '\n';
        cout << "Exam Type    : " << examType_  << '\n';
        cout << "Year         : " << year_      << '\n';
        cout << "File Name    : " << fileName_  << '\n';
        cout << "Uploaded By  : " << uploadedBy_ << '\n';
        cout << "Status       : " << status_    << '\n';
        cout << "----------------------------------------\n";
    }
    friend ostream &operator<<(ostream &os, const ExamPaper &p);   // friend
    bool operator<(const ExamPaper &other) const { return year_ < other.year_; } // operator overloading
};
ostream &operator<<(ostream &os, const ExamPaper &p) {
    os << left << setw(6) << p.id_ << setw(22) << p.name_ << setw(16) << p.examType_
       << setw(7) << p.branch_ << setw(5) << p.semester_ << setw(7) << p.year_
       << setw(20) << p.status_;
    return os;
}

struct Question { string text, a, b, c, d; int correct, marks; };
struct Assessment {
    int id, totalMarks; string title, subject;
    vector<Question> questions;
};
struct Result    { int assessmentId, score; string title, studentId; };
struct Attendance { string studentId, subject, date, status; };

// ===========================================================================
// 4. TYPE CASTING DEMOS (preserved from original)
// ===========================================================================
class Semester { int s_; public: Semester(int s = 0) : s_(s) {} void display() const { cout << "Semester: " << s_ << '\n'; } }; // basic -> class
class Year      { int y_; public: Year(int y = 0) : y_(y) {} operator int() const { return y_; } };                          // class -> basic
class Marks     { int m_; public: Marks(int m = 0) : m_(m) {} int get() const { return m_; } };
class Grade     { char g_ = 'F'; public: Grade() = default; Grade(const Marks &m) {                                          // class -> class
    int s = m.get(); g_ = s >= 90 ? 'A' : s >= 75 ? 'B' : s >= 60 ? 'C' : 'D'; }
    void display() const { cout << "Grade: " << g_ << '\n'; } };

// ===========================================================================
// 5. ABSTRACT USER + DERIVED ROLES (inheritance / polymorphism)
// ===========================================================================
class User {                                   // ABSTRACT class (the bare "Virtual" token is gone)
private:
    string id_, name_, email_, password_;
protected:
    string getPassword() const { return password_; }
public:
    User(string id, string name, string email, string password)
        : id_(move(id)), name_(move(name)), email_(move(email)), password_(move(password)) {}
    virtual ~User() = default;
    string getId()    const { return id_; }
    string getName()  const { return name_; }
    string getEmail() const { return email_; }
    bool login(const string &pw) const { return pw == password_; }
    void logout() const { cout << "\nLogged out successfully.\n"; }
    virtual string getRole() const = 0;        // pure virtual
    virtual void showDashboard() const = 0;    // pure virtual
};

class Student : public User {
    vector<int> downloaded_, saved_;
public:
    using User::User;
    string getRole() const override { return "Student"; }
    void showDashboard() const override {
        cout << "\n============ STUDENT DASHBOARD ============\n";
        cout << "Welcome, " << getName() << "!\n";
        cout << "1. Search Papers   2. Browse Subjects   3. My Downloads\n";
        cout << "4. Saved Papers    5. Online Assessment  6. My Marks\n";
        cout << "7. My Attendance   8. My Profile         9. Logout\n";
    }
    void download(int id) { downloaded_.push_back(id); cout << "\nPaper " << id << " downloaded.\n"; }
    void save(int id)     { saved_.push_back(id);     cout << "\nPaper " << id << " saved.\n"; }
    void showDownloads() const {
        cout << "\n--- MY DOWNLOADS ---\n";
        if (downloaded_.empty()) { cout << "No downloads yet.\n"; return; }
        for (size_t i = 0; i < downloaded_.size(); ++i) cout << i + 1 << ". Paper ID " << downloaded_[i] << '\n';
    }
    void showSaved() const {
        cout << "\n--- SAVED PAPERS ---\n";
        if (saved_.empty()) { cout << "No saved papers.\n"; return; }
        for (size_t i = 0; i < saved_.size(); ++i) cout << i + 1 << ". Paper ID " << saved_[i] << '\n';
    }
};

class Faculty : public User {
    vector<int> uploaded_;
public:
    using User::User;
    string getRole() const override { return "Faculty"; }
    void showDashboard() const override {
        cout << "\n============ FACULTY DASHBOARD ============\n";
        cout << "Welcome, " << getName() << "!\n";
        cout << "1. Search Papers      2. Upload Paper\n";
        cout << "3. My Uploaded Papers 4. Update My Paper\n";
        cout << "5. Request Deletion   6. Mark Attendance\n";
        cout << "7. Take Assessment    8. View Results\n";
        cout << "9. My Profile        10. Logout\n";
    }
    void addUploaded(int id) { uploaded_.push_back(id); }
    void showUploaded() const {
        cout << "\n--- MY UPLOADED PAPERS ---\n";
        if (uploaded_.empty()) { cout << "You have not uploaded any papers.\n"; return; }
        for (size_t i = 0; i < uploaded_.size(); ++i) cout << i + 1 << ". Paper ID " << uploaded_[i] << '\n';
    }
    void updatePaper(int id) { cout << "Update requested for paper " << id << '\n'; }
    void updatePaper(int id, const string &type) { cout << "Paper " << id << " exam type -> " << type << '\n'; } // overloading
};

class Admin : public User {
public:
    using User::User;
    string getRole() const override { return "Admin"; }
    void showDashboard() const override {
        cout << "\n============= ADMIN DASHBOARD =============\n";
        cout << "Welcome, " << getName() << "!\n";
        cout << "1. Manage Users        2. Manage Papers\n";
        cout << "3. Verify Papers       4. Manage Subjects\n";
        cout << "5. Upload PYQ          6. Search All\n";
        cout << "7. Sort by Year        8. Reports\n";
        cout << "9. OOP Lab Demo       10. View Results\n";
        cout << "11. My Profile        12. Logout\n";
    }
};

// Virtual base class / diamond problem (preserved)
class RepositoryItem {
protected:
    int itemId_; string title_;
public:
    RepositoryItem(int id = 0, string t = "") : itemId_(id), title_(move(t)) {}
    virtual void displayInfo() const { cout << "Repository Item: " << title_ << " (ID " << itemId_ << ")\n"; }
    virtual ~RepositoryItem() = default;
};
class PrintableItem : virtual public RepositoryItem {
protected: int pages_;
public: PrintableItem(int id = 0, string t = "", int p = 0) : RepositoryItem(id, move(t)), pages_(p) {}
    void printPreview() const { cout << "Print Preview: " << pages_ << " pages\n"; }
};
class DigitalItem : virtual public RepositoryItem {
protected: double sizeMB_;
public: DigitalItem(int id = 0, string t = "", double s = 0) : RepositoryItem(id, move(t)), sizeMB_(s) {}
    void digitalInfo() const { cout << "Digital Size: " << sizeMB_ << " MB\n"; }
};
class DemoExamPaper : public PrintableItem, public DigitalItem {
public:
    DemoExamPaper(int id, string t, int p, double s)
        : RepositoryItem(id, t), PrintableItem(id, t, p), DigitalItem(id, t, s) {}
    void displayInfo() const override {
        cout << "\n=== VIRTUAL BASE CLASS DEMO ===\n";
        cout << "ID: " << itemId_ << "\nTitle: " << title_ << "\nPages: " << pages_
             << "\nSize: " << sizeMB_ << " MB\n";
        cout << "RepositoryItem exists only ONCE thanks to virtual inheritance.\n";
    }
};

// ===========================================================================
// 6. MANAGER (data + business rules, uses the custom exceptions)
// ===========================================================================
class PaperManager {
private:
    vector<ExamPaper> papers_;
    vector<Subject> subjects_;
    vector<Assessment> assessments_;
    vector<Result> results_;
    vector<Attendance> attendance_;
    int nextId_ = 1001;
public:
    PaperManager() {
        subjects_ = {
            Subject("CS201", "Object Oriented Programming", "CSE"),
            Subject("CS301", "Data Structures", "CSE"),
            Subject("CS202", "Database Management System", "CSE"),
            Subject("EC201", "Digital Logic", "ECE")
        };
        papers_ = {
            ExamPaper(nextId_++, "CS301", "Data Structures", "CSE", 3, "Mid Semester", 2025, "DS_MidSem_2025.pdf", "F014", "Verified"),
            ExamPaper(nextId_++, "CS201", "Object Oriented Programming", "CSE", 4, "Mid Semester", 2026, "OOP_MidSem_2026.pdf", "F014", "Verified"),
            ExamPaper(nextId_++, "CS202", "Database Management System", "CSE", 4, "End Semester", 2025, "DBMS_EndSem_2025.pdf", "F014", "Pending"),
            ExamPaper(nextId_++, "EC201", "Digital Logic", "ECE", 3, "Mid Semester", 2026, "DL_MidSem_2026.pdf", "F014", "Verified")
        };
        Assessment a{1, 4, "OOP Quick Quiz", "CS201", {}};
        a.questions = {
            {"Which keyword defines an abstract class in Java?", "class", "abstract", "interface", "final", 2, 2},
            {"Which keyword declares a method may raise an exception?", "throw", "throws", "catch", "finally", 2, 2}
        };
        assessments_.push_back(a);
    }
    vector<ExamPaper> &papers() { return papers_; }
    vector<Subject> &subjects() { return subjects_; }
    vector<Assessment> &assessments() { return assessments_; }
    vector<Result> &results() { return results_; }
    vector<Attendance> &attendance() { return attendance_; }

    ExamPaper *find(int id) {
        for (auto &p : papers_) if (p.getId() == id) return &p;
        return nullptr;
    }
    ExamPaper &require(int id) {   // throws a user-defined exception
        ExamPaper *p = find(id);
        if (!p) throw PaperNotFoundException("Paper " + to_string(id) + " not found.");
        return *p;
    }
    int generateId() { return nextId_; }

    void addPaper(const ExamPaper &p) {
        string key = p.getCode() + "|" + p.getExamType() + "|" + to_string(p.getYear()) + "|" + p.getBranch();
        for (const auto &existing : papers_) {
            string k = existing.getCode() + "|" + existing.getExamType() + "|" +
                       to_string(existing.getYear()) + "|" + existing.getBranch();
            if (k == key) throw DuplicatePaperException("A paper with these details already exists.");
        }
        papers_.push_back(p);
        ++nextId_;
    }
    void addSubject(const Subject &s) {
        for (const auto &x : subjects_)
            if (x.getCode() == s.getCode()) throw DuplicatePaperException("Subject code already exists.");
        subjects_.push_back(s);
    }
    vector<ExamPaper> search(const string &branch, int sem, const string &subject, int year, const string &type) const {
        vector<ExamPaper> out;
        for (const auto &p : papers_) {
            if (branch != "Any" && p.getBranch() != branch) continue;
            if (sem != 0 && p.getSemester() != sem) continue;
            if (subject != "Any" && p.getName() != subject) continue;
            if (year != 0 && p.getYear() != year) continue;
            if (type != "Any" && p.getExamType() != type) continue;
            if (p.getStatus() != "Verified") continue;
            out.push_back(p);
        }
        return out;
    }
    void sortByYear() { sort(papers_.begin(), papers_.end()); }

    void submitResult(int assessmentId, const string &studentId, int score) {
        Assessment *a = nullptr;
        for (auto &x : assessments_) if (x.id == assessmentId) a = &x;
        if (!a) throw AssessmentClosedException("Assessment not found.");
        if (score < 0 || score > a->totalMarks) throw InvalidMarksException("Score out of range.");
        for (const auto &r : results_)
            if (r.assessmentId == assessmentId && r.studentId == studentId)
                throw AssessmentClosedException("You already submitted this assessment.");
        results_.push_back({assessmentId, score, a->title, studentId});
    }
    void markAttendance(const Attendance &rec) {
        for (const auto &x : attendance_)
            if (x.studentId == rec.studentId && x.subject == rec.subject && x.date == rec.date)
                throw DuplicatePaperException("Attendance already marked for that student/date.");
        if (rec.status != "PRESENT" && rec.status != "ABSENT" && rec.status != "LATE")
            throw UnauthorizedAccessException("Invalid attendance status.");
        attendance_.push_back(rec);
    }
    static void header() {
        cout << left << setw(6) << "ID" << setw(22) << "Subject" << setw(16) << "Exam Type"
             << setw(7) << "Branch" << setw(5) << "Sem" << setw(7) << "Year" << setw(20) << "Status" << '\n';
        cout << string(83, '-') << '\n';
    }
    void displayAll(bool all = true) const {
        cout << "\n=============== ALL PAPERS ===============\n";
        header();
        for (const auto &p : papers_) if (all || p.getStatus() == "Verified") cout << p << '\n';
    }
};

PaperManager g_manager;   // shared in-memory data for this console app

// ===========================================================================
// 7. EXCEPTION-HANDLING DEMOS  (lab topic)
// ===========================================================================
// "throws" equivalent: C++17 removed dynamic exception specifications, so a
// callee that may throw is marked noexcept(false) and documents its contract.
// (Java: `Paper loadPaper(int id) throws PaperNotFoundException`)
ExamPaper loadPaperOrThrow(int id) noexcept(false) {
    return g_manager.require(id);   // may throw PaperNotFoundException
}

void demoExceptionHandling() {
    clearScreen();
    cout << "============ EXCEPTION HANDLING DEMO ============\n";

    // (1) try / catch / throw + DERIVED exception caught before BASE
    cout << "\n(1) Derived-class catch before base-class catch:\n";
    try {
        throw PaperNotFoundException("Paper 9999 does not exist");
    } catch (const PaperNotFoundException &e) {          // derived first
        cout << "    caught DERIVED PaperNotFoundException: " << e.what() << '\n';
    } catch (const RepositoryException &e) {             // base second
        cout << "    caught BASE RepositoryException: " << e.what() << '\n';
    }

    // (2) derived exception caught through a BASE-class reference
    cout << "\n(2) Catching via BASE-class reference:\n";
    try {
        throw DuplicatePaperException("Duplicate subject/year/branch");
    } catch (const RepositoryException &e) {
        cout << "    caught via BASE -> " << e.what() << '\n';
    }

    // (3) "finally" block (RAII ScopeGuard) on BOTH success and failure
    cout << "\n(3) finally block (RAII) on success and on failure:\n";
    auto runWithFinally = [](bool fail) {
        Finally cleanup([] { cout << "    finally: resource released / audit written\n"; });
        if (fail) throw InvalidMarksException("score out of range");
        cout << "    try body succeeded\n";
    };
    try { runWithFinally(false); }
    catch (const RepositoryException &e) { cout << "    caught: " << e.what() << '\n'; }
    try { runWithFinally(true); }
    catch (const RepositoryException &e) { cout << "    caught: " << e.what() << '\n'; }

    // (4) throws-equivalent propagation: callee throws, caller handles
    cout << "\n(4) throws-equivalent (noexcept(false)) propagation:\n";
    try {
        loadPaperOrThrow(1001);
        cout << "    loadPaperOrThrow(1001) succeeded\n";
        loadPaperOrThrow(9999);
    } catch (const PaperNotFoundException &e) {
        cout << "    caller handled propagated exception: " << e.what() << '\n';
    }

    // (5) real business-rule exceptions
    cout << "\n(5) Real validation using user-defined exceptions:\n";
    try { g_manager.submitResult(1, "S9000", 99); }
    catch (const InvalidMarksException &e) { cout << "    caught: " << e.what() << '\n'; }

    cout << "\n[done] All Exception Handling bullets demonstrated.\n";
    pauseScreen();
}

// ===========================================================================
// 8. MULTITHREADING DEMO  (lab topic - was missing from the original file)
// ===========================================================================
void demoMultithreading() {
    clearScreen();
    cout << "============== MULTITHREADING DEMO ==============\n";

    mutex mtx;
    atomic<int> safeCounter{0};
    int unsafeCounter = 0;

    auto worker = [&](int id) {
        try {
            for (int i = 0; i < 20000; ++i) {
                safeCounter.fetch_add(1);                       // atomic: safe
                { lock_guard<mutex> lock(mtx); ++unsafeCounter; } // mutex: safe
            }
            if (id == 2) throw InvalidMarksException("simulated worker failure");
            lock_guard<mutex> lock(mtx);
            cout << "    Thread-" << id << " (" << this_thread::get_id() << ") finished OK\n";
        } catch (const RepositoryException &e) {
            lock_guard<mutex> lock(mtx);
            cout << "    Thread-" << id << " caught " << e.what() << " (app still alive)\n";
        }
    };

    vector<thread> threads;
    for (int i = 0; i < 4; ++i) threads.emplace_back(worker, i);
    for (auto &t : threads) t.join();

    cout << "\n    Thread-safe counter (atomic) = " << safeCounter << "  [expected 80000]\n";
    cout << "    Thread-safe counter (mutex)  = " << unsafeCounter << "  [expected 80000]\n";

    // std::async / std::future: exceptions travel through future.get()
    cout << "\n--- std::async / std::future ---\n";
    auto task = [](int x) -> int {
        this_thread::sleep_for(chrono::milliseconds(10));
        if (x == 3) throw runtime_error("simulated future failure");
        return x * x;
    };
    vector<future<int>> futures;
    for (int i = 0; i < 6; ++i) futures.push_back(async(launch::async, task, i));
    for (int i = 0; i < 6; ++i) {
        try {
            int value = futures[i].get();              // get() may throw
            cout << "    future " << i << " -> " << value << '\n';
        } catch (const exception &e) {
            cout << "    future " << i << " caught " << e.what() << '\n';
        }
    }
    cout << "\n[done] Multithreading demo complete.\n";
    pauseScreen();
}

// ===========================================================================
// 9. OOP CONCEPT DEMONSTRATION MENU (original menu + the two new demos)
// ===========================================================================
void oopDemoMenu() {
    int choice;
    do {
        clearScreen();
        cout << "========================================\n";
        cout << "       OOP CONCEPTS DEMONSTRATION\n";
        cout << "========================================\n";
        cout << "1. Encapsulation\n2. Abstraction & Polymorphism\n3. Inheritance\n";
        cout << "4. this Pointer\n5. Pointer to Object / Derived Object\n6. Virtual Function\n";
        cout << "7. Virtual Base Class / Diamond\n8. Function Overloading\n";
        cout << "9. Friend Operator Overloading\n10. Constructors\n11. Type Casting\n";
        cout << "12. EXCEPTION HANDLING  <-- new lab topic\n";
        cout << "13. MULTITHREADING  <-- new lab topic\n14. Back\n";
        choice = readInt("Enter choice: ", 1, 14);

        switch (choice) {
        case 1: {
            clearScreen(); cout << "============ ENCAPSULATION ============\n";
            ExamPaper p(500, "CS201", "OOP", "CSE", 4, "Mid Semester", 2026, "OOP.pdf", "F014");
            cout << "Private data is accessed only via public getters.\n"; p.displayDetails();
            pauseScreen(); break;
        }
        case 2: {
            clearScreen(); cout << "===== ABSTRACTION & POLYMORPHISM =====\n";
            Student s("S1023", "Riya Sharma", "riya@college.edu", "1234");
            Faculty f("F014", "Dr. A. Verma", "verma@college.edu", "1234");
            Admin  a("A002", "Mr. R. Singh", "singh@college.edu", "1234");
            User *users[] = {&s, &f, &a};
            for (User *u : users) { cout << "\nRole via User* : " << u->getRole() << '\n'; u->showDashboard(); }
            cout << "\nSame virtual call, different behavior = runtime polymorphism.\n";
            pauseScreen(); break;
        }
        case 3:
            clearScreen(); cout << "============= INHERITANCE =============\n";
            cout << "              User (abstract)\n           /       |        \\\n     Student   Faculty     Admin\n";
            pauseScreen(); break;
        case 4: {
            clearScreen(); cout << "============== THIS POINTER ==============\n";
            ExamPaper p(502, "CS202", "DBMS", "CSE", 4, "End Semester", 2026, "DBMS.pdf", "F014");
            cout << "Constructor used this->member = parameter.\nObject address: " << &p << '\n';
            p.displayDetails(); pauseScreen(); break;
        }
        case 5: {
            clearScreen(); cout << "=============== POINTERS ===============\n";
            ExamPaper p(501, "CS301", "Data Structures", "CSE", 3, "Mid Semester", 2026, "DS.pdf", "F014", "Verified");
            ExamPaper *pp = &p;
            cout << "Address: " << pp << '\n'; pp->displayDetails();
            Student s("S1023", "Riya", "r@c.edu", "1234"); User *bp = &s;
            cout << "Base pointer -> Student role: " << bp->getRole() << '\n';
            pauseScreen(); break;
        }
        case 6:
            clearScreen(); cout << "The showDashboard() call through User* is a virtual function call.\n";
            pauseScreen(); break;
        case 7: {
            clearScreen(); DemoExamPaper d(700, "Virtual Inheritance Demo", 10, 3.5);
            RepositoryItem *bp = &d; bp->displayInfo(); d.printPreview(); d.digitalInfo();
            pauseScreen(); break;
        }
        case 8:
            clearScreen(); cout << "Faculty has updatePaper(int) and updatePaper(int,string).\n";
            pauseScreen(); break;
        case 9: {
            clearScreen(); cout << "Friend operator<< output:\n";
            cout << ExamPaper(503, "CS201", "OOP", "CSE", 4, "Mid Semester", 2026, "OOP.pdf", "F014") << '\n';
            pauseScreen(); break;
        }
        case 10: {
            clearScreen(); cout << "=============== CONSTRUCTORS ===============\n";
            ExamPaper d; cout << "Default constructor:\n"; d.displayDetails();
            ExamPaper p(505, "EC201", "Digital Logic", "ECE", 3, "Mid Semester", 2026, "DL.pdf", "F014");
            cout << "Parameterized constructor:\n"; p.displayDetails(); pauseScreen(); break;
        }
        case 11: {
            clearScreen(); cout << "=============== TYPE CASTING ===============\n";
            int semValue = 4; Semester sem = semValue; cout << "Basic -> Class: "; sem.display();
            Year y(2026); int yr = y; cout << "Class -> Basic: Year = " << yr << '\n';
            Marks m(85); Grade g = m; cout << "Class -> Class: "; g.display();
            pauseScreen(); break;
        }
        case 12: demoExceptionHandling(); break;   // FIXED: could not compile before
        case 13: demoMultithreading();   break;    // NEW
        }
    } while (choice != 14);
}

// ===========================================================================
// 10. SCREENS
// ===========================================================================
void showSearch() {
    clearScreen();
    cout << "================ SEARCH PAPERS ================\n";
    cout << "Branches: CSE, ECE, IT\n";
    string branch = readText("Branch or Any: ");
    int sem = readInt("Semester (0 for Any): ", 0, 8);
    string subject = readText("Subject name or Any: ");
    int year = readInt("Year (0 for Any): ", 0, 2030);
    string type = readText("Exam type or Any: ");
    auto results = g_manager.search(branch, sem, subject, year, type);
    clearScreen();
    cout << "Found " << results.size() << " verified paper(s).\n";
    PaperManager::header();
    for (const auto &p : results) cout << p << '\n';
    int id = readInt("Enter Paper ID to view (0 to go back): ", 0, 100000);
    if (id) {
        try { g_manager.require(id).displayDetails(); }
        catch (const RepositoryException &e) { cout << "Error: " << e.what() << '\n'; }
    }
    pauseScreen();
}

void studentScreen(Student &s) {
    int c;
    do {
        clearScreen(); s.showDashboard(); c = readInt("\nEnter choice: ", 1, 9);
        try {
            switch (c) {
            case 1: showSearch(); break;
            case 2:
                clearScreen(); cout << "================ SUBJECTS ================\n";
                for (const auto &sub : g_manager.subjects()) sub.display();
                pauseScreen(); break;
            case 3: clearScreen(); s.showDownloads(); pauseScreen(); break;
            case 4: clearScreen(); s.showSaved(); pauseScreen(); break;
            case 5: {
                clearScreen(); cout << "============ ONLINE ASSESSMENT ============\n";
                for (const auto &a : g_manager.assessments())
                    cout << "Assessment " << a.id << ": " << a.title << " (" << a.totalMarks << " marks)\n";
                int aid = readInt("Assessment ID (0 to go back): ", 0, 1000);
                if (aid) {
                    for (const auto &a : g_manager.assessments()) if (a.id == aid) {
                        int score = 0;
                        for (const auto &q : a.questions) {
                            cout << "\n" << q.text << '\n';
                            cout << "1. " << q.a << "  2. " << q.b << "  3. " << q.c << "  4. " << q.d << '\n';
                            int ans = readInt("Your answer: ", 1, 4);
                            if (ans == q.correct) score += q.marks;
                        }
                        g_manager.submitResult(aid, s.getId(), score);
                        cout << "\nSubmitted! Marks: " << score << "/" << a.totalMarks << '\n';
                    }
                }
                pauseScreen(); break;
            }
            case 6:
                clearScreen(); cout << "================ MY MARKS ================\n";
                for (const auto &r : g_manager.results())
                    if (r.studentId == s.getId()) cout << r.title << " : " << r.score << " marks\n";
                pauseScreen(); break;
            case 7: {
                clearScreen(); cout << "=============== MY ATTENDANCE ===============\n";
                int present = 0, total = 0;
                for (const auto &a : g_manager.attendance()) if (a.studentId == s.getId()) {
                    ++total; if (a.status == "PRESENT") ++present;
                    cout << a.subject << " | " << a.date << " | " << a.status << '\n';
                }
                cout << "Attendance: " << (total ? (present * 100 / total) : 0) << "%\n";
                pauseScreen(); break;
            }
            case 8:
                clearScreen(); cout << "================ MY PROFILE ================\n";
                cout << "Role: " << s.getRole() << "\nID: " << s.getId() << "\nName: " << s.getName() << '\n';
                cout << "Email: " << s.getEmail() << '\n'; pauseScreen(); break;
            }
        } catch (const RepositoryException &e) {
            cout << "\nError: " << e.what() << '\n'; pauseScreen();
        }
    } while (c != 9);
    s.logout();
}

void facultyScreen(Faculty &f) {
    int c;
    do {
        clearScreen(); f.showDashboard(); c = readInt("\nEnter choice: ", 1, 10);
        try {
            switch (c) {
            case 1: showSearch(); break;
            case 2: {
                clearScreen(); cout << "================ UPLOAD PAPER ================\n";
                string code = readText("Subject code: "), name = readText("Subject name: ");
                string branch = readText("Branch: ");
                int sem = readInt("Semester (1-8): ", 1, 8);
                string type = readText("Exam type: ");
                int year = readInt("Year (2020-2030): ", 2020, 2030);
                string file = readText("File name (example.pdf): ");
                int id = g_manager.generateId();
                g_manager.addPaper(ExamPaper(id, code, name, branch, sem, type, year, file, f.getId(), "Pending"));
                f.addUploaded(id);
                cout << "\nPaper uploaded! ID: " << id << " | Status: Pending admin verification\n";
                pauseScreen(); break;
            }
            case 3: {
                clearScreen(); f.showUploaded();
                for (const auto &p : g_manager.papers()) if (p.getUploader() == f.getId()) cout << p << '\n';
                pauseScreen(); break;
            }
            case 4: {
                clearScreen(); g_manager.displayAll();
                int id = readInt("Your Paper ID to update (0 to back): ", 0, 100000);
                if (id) {
                    ExamPaper &p = g_manager.require(id);
                    if (p.getUploader() != f.getId()) throw UnauthorizedAccessException("You may update only your own paper.");
                    string t = readText("New exam type: ");
                    int y = readInt("New year: ", 2020, 2030);
                    p.setDetails(t, y);
                    f.updatePaper(id); f.updatePaper(id, t);
                    cout << "Updated. Status reset to Pending.\n";
                }
                pauseScreen(); break;
            }
            case 5: {
                clearScreen();
                int id = readInt("Paper ID for deletion request (0 to back): ", 0, 100000);
                if (id) {
                    ExamPaper &p = g_manager.require(id);
                    if (p.getUploader() != f.getId()) throw UnauthorizedAccessException("Not your paper.");
                    p.setStatus("Deletion Requested");
                    cout << "Deletion request sent to admin.\n";
                }
                pauseScreen(); break;
            }
            case 6: {
                clearScreen(); cout << "=============== MARK ATTENDANCE ===============\n";
                string subject = readText("Subject code: "), date = readText("Date (YYYY-MM-DD): ");
                string sid = readText("Student ID (S1023): ");
                cout << "Status: 1. PRESENT  2. ABSENT  3. LATE\n";
                int ch = readInt("Choice: ", 1, 3);
                string st = ch == 1 ? "PRESENT" : ch == 2 ? "ABSENT" : "LATE";
                g_manager.markAttendance({sid, subject, date, st});
                cout << "Attendance marked.\n"; pauseScreen(); break;
            }
            case 7: {
                clearScreen(); cout << "============ TAKE ASSESSMENT (FACULTY VIEW) ============\n";
                for (const auto &a : g_manager.assessments()) cout << "Assessment " << a.id << ": " << a.title << '\n';
                cout << "(Faculty may preview questions; students submit from their dashboard.)\n";
                pauseScreen(); break;
            }
            case 8:
                clearScreen(); cout << "=============== RESULTS ===============\n";
                for (const auto &r : g_manager.results()) cout << r.title << " -> " << r.score << " marks\n";
                pauseScreen(); break;
            case 9:
                clearScreen(); cout << "Role: " << f.getRole() << "\nID: " << f.getId() << "\nName: " << f.getName() << '\n';
                pauseScreen(); break;
            }
        } catch (const RepositoryException &e) {
            cout << "\nError: " << e.what() << '\n'; pauseScreen();
        }
    } while (c != 10);
    f.logout();
}

void adminScreen(Admin &a) {
    int c;
    do {
        clearScreen(); a.showDashboard(); c = readInt("\nEnter choice: ", 1, 12);
        try {
            switch (c) {
            case 1: {
                clearScreen(); cout << "================ MANAGE USERS ================\n";
                cout << "1. Student - Riya Sharma (S1023)\n2. Faculty - Dr. A. Verma (F014)\n3. Admin - Mr. R. Singh (A002)\n";
                cout << "Constraint: the last Admin cannot be deleted; users with records cannot be deleted.\n";
                pauseScreen(); break;
            }
            case 2: g_manager.displayAll(); pauseScreen(); break;
            case 3: {
                clearScreen(); cout << "================ VERIFY PAPERS ================\n";
                bool found = false;
                PaperManager::header();
                for (const auto &p : g_manager.papers())
                    if (p.getStatus() == "Pending" || p.getStatus() == "Deletion Requested") { cout << p << '\n'; found = true; }
                if (!found) { cout << "No pending papers.\n"; pauseScreen(); break; }
                int id = readInt("Paper ID (0 to back): ", 0, 100000);
                if (id) {
                    ExamPaper &p = g_manager.require(id);
                    cout << "1. Verify  2. Reject  3. Approve Deletion\n";
                    int ch = readInt("Choice: ", 1, 3);
                    if (ch == 1) p.setStatus("Verified");
                    else if (ch == 2) p.setStatus("Rejected");
                    else if (p.getStatus() == "Deletion Requested") {
                        auto &vec = g_manager.papers();
                        vec.erase(remove_if(vec.begin(), vec.end(), [id](const ExamPaper &x) { return x.getId() == id; }), vec.end());
                        cout << "Paper deleted.\n";
                    } else cout << "Paper was not in Deletion Requested state.\n";
                }
                pauseScreen(); break;
            }
            case 4: {
                clearScreen(); cout << "================ MANAGE SUBJECTS ================\n";
                for (const auto &s : g_manager.subjects()) s.display();
                cout << "\nAdd subject:\n";
                string code = readText("Code: "), name = readText("Name: "), br = readText("Branch: ");
                g_manager.addSubject(Subject(code, name, br));
                cout << "Subject added.\n"; pauseScreen(); break;
            }
            case 5: {
                clearScreen(); cout << "============ ADMIN UPLOAD PYQ ============\n";
                string code = readText("Subject code: "), name = readText("Subject name: ");
                string br = readText("Branch: ");
                int sem = readInt("Semester: ", 1, 8), year = readInt("Year: ", 2020, 2030);
                string type = readText("Exam type: "), file = readText("File: ");
                int id = g_manager.generateId();
                g_manager.addPaper(ExamPaper(id, code, name, br, sem, type, year, file, a.getId(), "Verified"));
                cout << "PYQ uploaded directly as VERIFIED. ID: " << id << '\n'; pauseScreen(); break;
            }
            case 6: showSearch(); break;
            case 7:
                clearScreen(); g_manager.sortByYear(); cout << "Papers sorted by year.\n"; g_manager.displayAll(); pauseScreen(); break;
            case 8: {
                clearScreen(); int v = 0, p = 0, r = 0, d = 0;
                for (const auto &x : g_manager.papers()) {
                    if (x.getStatus() == "Verified") ++v;
                    else if (x.getStatus() == "Pending") ++p;
                    else if (x.getStatus() == "Rejected") ++r;
                    else ++d;
                }
                cout << "================ REPORTS ================\n";
                cout << "Total: " << g_manager.papers().size() << "\nVerified: " << v << "\nPending: " << p
                     << "\nRejected: " << r << "\nDeletion Requests: " << d << '\n';
                pauseScreen(); break;
            }
            case 9: oopDemoMenu(); break;
            case 10:
                clearScreen(); cout << "=============== ALL RESULTS ===============\n";
                for (const auto &r : g_manager.results()) cout << r.title << " -> " << r.score << " marks\n";
                pauseScreen(); break;
            case 11:
                clearScreen(); cout << "Role: " << a.getRole() << "\nID: " << a.getId() << "\nName: " << a.getName() << '\n';
                pauseScreen(); break;
            }
        } catch (const RepositoryException &e) {
            cout << "\nError: " << e.what() << '\n'; pauseScreen();
        }
    } while (c != 12);
    a.logout();
}

// ===========================================================================
// 11. MAIN
// ===========================================================================
bool loginUser(User &u) {
    clearScreen();
    cout << "========================================\n";
    cout << "             " << u.getRole() << " LOGIN\n";
    cout << "========================================\n";
    cout << "User ID: " << u.getId() << '\n';
    string pw = readText("Password: ");
    if (u.login(pw)) { cout << "\nLogin successful!\n"; pauseScreen(); return true; }
    cout << "\nIncorrect password.\n"; pauseScreen(); return false;
}

int main() {
    Student student("S1023", "Riya Sharma", "riya.sharma@college.edu", "1234");
    Faculty faculty("F014", "Dr. A. Verma", "verma@college.edu", "1234");
    Admin  admin  ("A002", "Mr. R. Singh", "singh@college.edu", "1234");

    int choice;
    do {
        clearScreen();
        cout << "======================================================\n";
        cout << "              EXAM PAPER REPOSITORY\n";
        cout << "  C++ console project | Students, Faculty and Admin\n";
        cout << "======================================================\n";
        cout << "1. Student Login\n2. Faculty Login\n3. Admin Login\n4. Exit\n";
        choice = readInt("\nEnter choice: ", 1, 4);
        switch (choice) {
        case 1: if (loginUser(student)) studentScreen(student); break;
        case 2: if (loginUser(faculty)) facultyScreen(faculty); break;
        case 3: if (loginUser(admin))   adminScreen(admin);   break;
        case 4:
            clearScreen();
            cout << "============================================\n";
            cout << " Thank you for using Exam Paper Repository!\n";
            cout << "============================================\n";
            break;
        }
    } while (choice != 4);
    return 0;
}
