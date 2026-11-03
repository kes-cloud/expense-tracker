# 💰 Smart Expense Tracker with Budget Alerts and Spending Analysis

A console-based **Smart Expense Tracker** developed using the C programming language as part of the **LTPS (Learning Through Problem Solving)** project.

This application helps users manage their daily income and expenses, monitor their monthly budget, analyze spending patterns, and generate financial reports through a simple menu-driven interface.

## 🚀 Features

- **Add Income & Expenses:** Record transactions with amount, category, date, and description.
- **Transaction Management:** View, search, edit, and delete existing transactions.
- **Financial Summary:** Calculate total income, total expenses, current balance, and savings rate.
- **Category-wise Analysis:** Track spending across seven different expense categories.
- **Monthly Budget Management:** Set and monitor a monthly spending budget.
- **Budget Alerts:** Receive warnings when spending reaches 80% or 100% of the monthly budget.
- **Spending Insights:** Get rule-based financial insights using conditional statements.
- **Monthly Reports:** Generate detailed monthly financial reports.
- **Report Export:** Export reports in TXT and CSV formats.
- **Automatic Data Saving:** Store transactions and budget information in a file and load them automatically when the application starts.
- **Input Validation:** Validate user inputs, dates, and transaction amounts.

## 🛠️ Technologies Used

- **Programming Language:** C
- **Compiler:** GCC
- **Standard Libraries:** stdio.h, stdlib.h, string.h, ctype.h, time.h
- **Concepts Implemented:**
  - Functions
  - Arrays
  - Strings
  - Structures
  - Pointers
  - File Handling
  - Conditional Statements
  - Loops
  - Data Validation

## 📂 Project Structure

```text
Smart-Expense-Tracker/
│
├── expense_tracker.c
├── README.md
│
├── expense_data.txt       # Automatically generated data file
├── expense_data.tmp      # Temporary file during saving
├── expense_data.bak      # Backup file if corrupted data is detected
│
└── report_MM-YYYY.txt    # Exported monthly reports
```

*Note: Data and report files are generated automatically when the application runs and saves information.*

## ⚙️ How to Run the Project

### 1. Clone the Repository

```bash
git clone https://github.com/YOUR-USERNAME/YOUR-REPOSITORY.git
```

### 2. Open the Project Folder

```bash
cd YOUR-REPOSITORY
```

### 3. Compile the Program

```bash
gcc -std=c99 -Wall -Wextra expense_tracker.c -o expense_tracker
```

### 4. Run the Application

**Windows:**
```bash
expense_tracker.exe
```

**Linux / macOS:**
```bash
./expense_tracker
```

## 📋 Application Menu

The application provides the following menu options:

| Option | Function |
|---|---|
| 1 | Add Income |
| 2 | Add Expense |
| 3 | View All Transactions |
| 4 | Search Transactions |
| 5 | Edit Transaction |
| 6 | Delete Transaction |
| 7 | Financial Summary |
| 8 | Category-wise Expense Summary |
| 9 | Set Monthly Budget |
| 10 | Check Budget Status |
| 11 | Generate Monthly Report |
| 12 | Export Report (TXT/CSV) |
| 0 | Save and Exit |

## 🧠 Spending Analysis Logic

The project uses simple rule-based logic (if-else conditions) to generate financial insights.

Some implemented rules include:

- If expenses exceed income, the application identifies the spending deficit.
- If the savings rate is at least 20%, it provides a positive savings insight.
- If one expense category exceeds 40% of total spending, it highlights that category.
- If Shopping and Entertainment together exceed 30% of spending, it identifies higher discretionary spending.
- If monthly budget usage reaches 80% or 100%, it generates an appropriate alert.

**Note:** The spending insights are rule-based and do not use Artificial Intelligence or Machine Learning.

## 💾 Data Storage

The application uses file handling in C to maintain user data.

- Transactions and monthly budget information are stored in `expense_data.txt`.
- Existing records are automatically loaded when the program starts.
- Changes are saved automatically after transaction operations.
- A temporary file is used during saving to reduce the risk of incomplete writes.
- A backup file is created when corrupted or duplicate records are detected.

## 🎯 Project Objectives

1. Apply fundamental C programming concepts to solve a real-world problem.
2. Develop a practical application for managing personal finances.
3. Understand the implementation of structures, arrays, functions, and file handling.
4. Implement budget monitoring and spending analysis using logical conditions.
5. Improve problem-solving and programming skills through practical implementation.

## 🔮 Future Improvements

- Develop a graphical user interface (GUI).
- Add graphical charts for spending visualization.
- Implement password-based user authentication.
- Add database integration.
- Introduce AI-based personalized spending recommendations.

## 👨‍💻 Project Information

**Project Name:** Smart Expense Tracker with Budget Alerts and Spending Analysis

**Subject:** Learning Through Problem Solving (LTPS)

**Programming Language:** C

**Project Type:** Console-Based Application

**Developed By:** Keshav

**Department:** Computer Science and Engineering

## 📜 License

This project is developed for educational purposes as part of the LTPS coursework.
<img width="727" height="421" alt="image" src="https://github.com/user-attachments/assets/dc86f39b-5b75-4ab4-86eb-f8918b25df14" />
<img width="680" height="328" alt="image" src="https://github.com/user-attachments/assets/e50e9673-365a-4f8b-bec6-68fe18ba3921" />
<img width="657" height="329" alt="image" src="https://github.com/user-attachments/assets/9be7b68a-3ae1-4b69-985a-826e9da6a0f3" />
<img width="626" height="601" alt="image" src="https://github.com/user-attachments/assets/9cef2f71-003d-48d6-9262-a0b060c18507" />
<img width="646" height="345" alt="image" src="https://github.com/user-attachments/assets/b2c97032-9d0f-4086-9130-dc4dbaf60b78" />
<img width="631" height="518" alt="image" src="https://github.com/user-attachments/assets/28697a72-29a2-4e27-ab9f-95d189752ead" />






