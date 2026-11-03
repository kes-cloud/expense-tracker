/*
 * ====================================================================
 *  SMART EXPENSE TRACKER WITH BUDGET ALERTS AND SPENDING ANALYSIS
 * ====================================================================
 *  Language : C (standard libraries only)
 *  Compiler : GCC (tested style: gcc -std=c99 -Wall -Wextra)
 *  Concepts : functions, arrays, strings, structures, file handling
 *
 *  What it does:
 *    - Add income and expenses (ID, amount, category, date, description)
 *    - View, search, edit and delete transactions
 *    - Totals, balance, savings rate, category summary
 *    - Monthly budget with alerts at 80% and 100%
 *    - Rule-based spending insights (simple if-else rules, NO AI)
 *    - Monthly report (screen) + export to TXT or CSV
 *    - Data is saved in expense_data.txt and loaded on start-up
 *
 *  Money note: amounts are typed in rupees (e.g. 250.50) but all
 *  additions/comparisons are done in PAISE (whole numbers), because
 *  double numbers can have tiny rounding errors (0.1 + 0.2 != 0.3).
 * ====================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* ------------------------- Constants ------------------------------- */
#define MAX_TRANSACTIONS 500          /* size of the transaction array   */
#define CATEGORY_COUNT   7            /* number of expense categories    */
#define CATEGORY_LEN     20           /* max category text (with '\0')   */
#define DATE_LEN         11           /* "DD-MM-YYYY" + '\0'             */
#define MONTH_LEN        8            /* "MM-YYYY"    + '\0'             */
#define DESC_LEN         50           /* max description (with '\0')     */
#define INPUT_LEN        100          /* size of every input buffer      */
#define LINE_LEN         256          /* size of one line read from file */
#define MAX_AMOUNT       9999999.99   /* largest amount accepted         */

#define DATA_FILE   "expense_data.txt"
#define TEMP_FILE   "expense_data.tmp"
#define BACKUP_FILE "expense_data.bak"

#define INCOME_CATEGORY "Income"

/* Set to 0 if your console shows '?' instead of the rupee sign.        */
#define USE_RUPEE_SYMBOL 1
#if USE_RUPEE_SYMBOL
#define CURRENCY "\xE2\x82\xB9"       /* UTF-8 bytes of the rupee sign   */
#else
#define CURRENCY "Rs."
#endif

/* ------------------------- Data structure -------------------------- */
typedef struct
{
    int    id;                        /* unique transaction number       */
    char   type;                      /* 'I' = income, 'E' = expense     */
    double amount;                    /* rupees, 2 decimal places        */
    char   category[CATEGORY_LEN];    /* Food, Travel, ... or Income     */
    char   date[DATE_LEN];            /* DD-MM-YYYY                      */
    char   description[DESC_LEN];     /* short note                      */
} Transaction;

/* ------------------------- Global data ----------------------------- */
Transaction transactions[MAX_TRANSACTIONS];   /* the main array          */
int         transactionCount = 0;             /* how many are stored     */
double      monthlyBudget    = 0.0;           /* 0 means "not set"       */

const char *categoryNames[CATEGORY_COUNT] =
{
    "Food", "Travel", "Education", "Shopping",
    "Bills", "Entertainment", "Other"
};

/* ------------------------- Prototypes ------------------------------ */
/* utilities */
void   setupConsole(void);
void   printRepeat(char ch, int count);
void   printHeading(const char *title);
void   trimString(char *text);
void   toLowerText(const char *source, char *destination);
long long toPaise(double rupees);
double paiseToRupees(long long paise);
double roundMoney(double value);
void   printMoney(FILE *out, long long paise);

/* input */
void   readLine(const char *prompt, char *buffer, int size);
int    readInt(const char *prompt, int min, int max);
double readAmount(const char *prompt, int allowZero);
int    readYesNo(const char *prompt);
void   readDate(const char *prompt, char *out);
void   readMonth(const char *prompt, char *out, int allowAll);
void   readDescription(char *out);
void   chooseCategory(char *out);
void   pauseScreen(void);

/* validation / dates */
int    isValidAmountText(const char *text);
int    isLeapYear(int year);
int    daysInMonth(int month, int year);
int    isValidDate(const char *date);
int    isValidMonth(const char *month);
void   getTodayDate(char *out);

/* data helpers */
int    findIndexById(int id);
int    generateNewId(void);
int    findCategoryIndex(const char *name);
int    matchesMonth(const Transaction *t, const char *month);
int    countTransactionsInMonth(const char *month);
void   calculateTotals(const char *month, long long *incomePaise, long long *expensePaise);
void   calculateCategoryTotals(const char *month, long long paise[]);
int    findHighestCategory(const long long paise[]);
int    calculateSavingsRate(long long incomePaise, long long expensePaise, double *rate);

/* file handling */
int    parseTransactionLine(const char *line, Transaction *t);
void   loadData(void);
int    saveData(void);
void   backupDataFile(void);

/* table output */
void   printTableLine(FILE *out);
void   printTableHeader(FILE *out);
void   printTableRow(FILE *out, const Transaction *t);

/* features */
void   addTransaction(char type);
void   showAllTransactions(void);
void   searchTransactions(void);
void   editTransaction(void);
void   deleteTransaction(void);
void   printSummaryBlock(const char *label, const char *month);
void   showFinancialSummary(void);
void   printCategoryBreakdown(FILE *out, const char *month);
void   showCategorySummary(void);
void   setMonthlyBudget(void);
void   printProgressBar(FILE *out, double percent);
void   checkBudgetAlert(const char *month);
void   showBudgetStatus(void);
void   writeInsight(FILE *out, int csv, const char *text);
void   printInsights(FILE *out, const char *month, int csv);
void   writeTextReport(FILE *out, const char *month);
void   writeCsvReport(FILE *out, const char *month);
void   showMonthlyReport(void);
void   exportReport(void);
void   showMainMenu(void);

/* ===================================================================
 *  UTILITY FUNCTIONS
 * =================================================================== */

/* On Windows, switch the console to UTF-8 so the rupee sign displays. */
void setupConsole(void)
{
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif
}

/* Print one character 'count' times (used to draw lines and boxes). */
void printRepeat(char ch, int count)
{
    int i;
    for (i = 0; i < count; i++)
    {
        putchar(ch);
    }
}

/* Print a title inside a box, centred. Title must be 60 chars or less. */
void printHeading(const char *title)
{
    int length = (int)strlen(title);
    int left   = (60 - length) / 2;
    int right;

    if (left < 0)
    {
        left = 0;
    }
    right = 60 - length - left;
    if (right < 0)
    {
        right = 0;
    }

    printf("\n+");
    printRepeat('=', 62);
    printf("+\n| ");
    printRepeat(' ', left);
    printf("%s", title);
    printRepeat(' ', right);
    printf(" |\n+");
    printRepeat('=', 62);
    printf("+\n");
}

/* Remove spaces / tabs / newline from both ends of a string. */
void trimString(char *text)
{
    int start = 0;
    int end   = (int)strlen(text);

    while (end > 0 && isspace((unsigned char)text[end - 1]))
    {
        end--;
    }
    text[end] = '\0';

    while (text[start] != '\0' && isspace((unsigned char)text[start]))
    {
        start++;
    }
    if (start > 0)
    {
        memmove(text, text + start, strlen(text + start) + 1);
    }
}

/* Copy a string in lower case (destination must be as big as source). */
void toLowerText(const char *source, char *destination)
{
    int i;
    for (i = 0; source[i] != '\0'; i++)
    {
        destination[i] = (char)tolower((unsigned char)source[i]);
    }
    destination[i] = '\0';
}

/* Rupees -> paise (whole number). Adds 0.5 so the value is rounded. */
long long toPaise(double rupees)
{
    if (rupees >= 0)
    {
        return (long long)(rupees * 100.0 + 0.5);
    }
    return (long long)(rupees * 100.0 - 0.5);
}

double paiseToRupees(long long paise)
{
    return (double)paise / 100.0;
}

/* Round a rupee value to exactly 2 decimal places. */
double roundMoney(double value)
{
    return paiseToRupees(toPaise(value));
}

/* Print an amount given in paise, e.g. 12550 -> "<rupee>125.50".
 * Uses only whole-number maths, so it is exact. Negative is allowed. */
void printMoney(FILE *out, long long paise)
{
    const char *sign = "";

    if (paise < 0)
    {
        sign  = "-";
        paise = -paise;
    }
    fprintf(out, "%s%s%lld.%02lld", sign, CURRENCY, paise / 100, paise % 100);
}

/* ===================================================================
 *  SAFE INPUT FUNCTIONS (all use fgets, never gets/scanf)
 * =================================================================== */

/* Show a prompt and read one line safely into 'buffer'. */
void readLine(const char *prompt, char *buffer, int size)
{
    int length;
    int ch;

    printf("%s", prompt);
    fflush(stdout);

    if (fgets(buffer, size, stdin) == NULL)
    {
        /* Input ended (Ctrl+Z / Ctrl+D): save and leave politely. */
        printf("\n  Input closed. Saving data and exiting...\n");
        saveData();
        exit(0);
    }

    length = (int)strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n')
    {
        buffer[length - 1] = '\0';
    }
    else
    {
        /* The user typed more than the buffer holds: throw away the rest. */
        while ((ch = getchar()) != '\n' && ch != EOF)
        {
        }
    }
    trimString(buffer);
}

/* Keep asking until the user enters a whole number from min to max. */
int readInt(const char *prompt, int min, int max)
{
    char  input[INPUT_LEN];
    char *end;
    long  value;

    while (1)
    {
        readLine(prompt, input, sizeof(input));

        if (input[0] == '\0')
        {
            printf("  Invalid input: please enter a number.\n");
            continue;
        }
        value = strtol(input, &end, 10);
        if (*end != '\0')
        {
            printf("  Invalid input: '%s' is not a whole number.\n", input);
            continue;
        }
        if (value < min || value > max)
        {
            printf("  Please enter a number between %d and %d.\n", min, max);
            continue;
        }
        return (int)value;
    }
}

/* Is the text a plain amount like 250, 99.5 or 1200.75 ?
 * Rules: digits only, one optional '.', at most 2 decimals,
 * at most 7 digits before the '.'. (No letters, commas, minus.)  */
int isValidAmountText(const char *text)
{
    int i;
    int seenDot      = 0;
    int digitsBefore = 0;
    int digitsAfter  = 0;

    if (text[0] == '\0')
    {
        return 0;
    }
    for (i = 0; text[i] != '\0'; i++)
    {
        if (isdigit((unsigned char)text[i]))
        {
            if (seenDot)
            {
                digitsAfter++;
            }
            else
            {
                digitsBefore++;
            }
        }
        else if (text[i] == '.' && !seenDot)
        {
            seenDot = 1;
        }
        else
        {
            return 0;
        }
    }
    if (digitsBefore + digitsAfter == 0)
    {
        return 0;
    }
    if (seenDot && digitsAfter == 0)
    {
        return 0;                          /* "5." is not allowed */
    }
    if (digitsAfter > 2 || digitsBefore > 7)
    {
        return 0;
    }
    return 1;
}

/* Keep asking until a valid amount is typed. allowZero=1 lets 0 pass. */
double readAmount(const char *prompt, int allowZero)
{
    char   input[INPUT_LEN];
    double value;

    while (1)
    {
        readLine(prompt, input, sizeof(input));

        if (!isValidAmountText(input))
        {
            printf("  Invalid amount. Use digits with up to 2 decimals (e.g. 250 or 99.50).\n");
            continue;
        }
        value = roundMoney(atof(input));
        if (value <= 0 && !allowZero)
        {
            printf("  Amount must be greater than zero.\n");
            continue;
        }
        if (value > MAX_AMOUNT)
        {
            printf("  Amount is too large (maximum is %.2f).\n", MAX_AMOUNT);
            continue;
        }
        return value;
    }
}

/* Ask a yes/no question. Returns 1 for yes, 0 for no. */
int readYesNo(const char *prompt)
{
    char input[INPUT_LEN];
    char lower[INPUT_LEN];

    while (1)
    {
        readLine(prompt, input, sizeof(input));
        toLowerText(input, lower);
        if (strcmp(lower, "y") == 0 || strcmp(lower, "yes") == 0)
        {
            return 1;
        }
        if (strcmp(lower, "n") == 0 || strcmp(lower, "no") == 0)
        {
            return 0;
        }
        printf("  Please type y or n.\n");
    }
}

/* ===================================================================
 *  DATE FUNCTIONS
 * =================================================================== */

int isLeapYear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int daysInMonth(int month, int year)
{
    int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month == 2 && isLeapYear(year))
    {
        return 29;
    }
    return days[month - 1];
}

/* Check the format DD-MM-YYYY and that the date really exists. */
int isValidDate(const char *date)
{
    int i;
    int day, month, year;

    if (strlen(date) != 10)
    {
        return 0;
    }
    for (i = 0; i < 10; i++)
    {
        if (i == 2 || i == 5)
        {
            if (date[i] != '-')
            {
                return 0;
            }
        }
        else if (!isdigit((unsigned char)date[i]))
        {
            return 0;
        }
    }
    day   = (date[0] - '0') * 10 + (date[1] - '0');
    month = (date[3] - '0') * 10 + (date[4] - '0');
    year  = atoi(date + 6);

    if (year < 2000 || year > 2100)
    {
        return 0;
    }
    if (month < 1 || month > 12)
    {
        return 0;
    }
    if (day < 1 || day > daysInMonth(month, year))
    {
        return 0;
    }
    return 1;
}

/* Check the format MM-YYYY. */
int isValidMonth(const char *month)
{
    int i, m, y;

    if (strlen(month) != 7 || month[2] != '-')
    {
        return 0;
    }
    for (i = 0; i < 7; i++)
    {
        if (i != 2 && !isdigit((unsigned char)month[i]))
        {
            return 0;
        }
    }
    m = (month[0] - '0') * 10 + (month[1] - '0');
    y = atoi(month + 3);
    return (m >= 1 && m <= 12 && y >= 2000 && y <= 2100);
}

/* Write today's date as DD-MM-YYYY into 'out' (size DATE_LEN). */
void getTodayDate(char *out)
{
    time_t     now = time(NULL);
    struct tm *local = localtime(&now);

    if (local == NULL || strftime(out, DATE_LEN, "%d-%m-%Y", local) == 0
        || !isValidDate(out))
    {
        strcpy(out, "01-01-2026");         /* safe fallback */
    }
}

/* Ask for a date. Enter = today. '/' is accepted and changed to '-'. */
void readDate(const char *prompt, char *out)
{
    char input[INPUT_LEN];
    int  i;

    while (1)
    {
        readLine(prompt, input, sizeof(input));
        if (input[0] == '\0')
        {
            getTodayDate(out);
            return;
        }
        for (i = 0; input[i] != '\0'; i++)
        {
            if (input[i] == '/')
            {
                input[i] = '-';
            }
        }
        if (isValidDate(input))
        {
            strcpy(out, input);            /* validated: exactly 10 chars */
            return;
        }
        printf("  Invalid date. Use DD-MM-YYYY (example: 25-12-2026).\n");
    }
}

/* Ask for a month (MM-YYYY). Enter = current month, or ALL if allowAll. */
void readMonth(const char *prompt, char *out, int allowAll)
{
    char input[INPUT_LEN];
    char today[DATE_LEN];
    int  i;

    while (1)
    {
        readLine(prompt, input, sizeof(input));
        if (input[0] == '\0')
        {
            if (allowAll)
            {
                out[0] = '\0';             /* empty text means "all time" */
            }
            else
            {
                getTodayDate(today);
                strcpy(out, today + 3);    /* skip "DD-" -> "MM-YYYY"     */
            }
            return;
        }
        for (i = 0; input[i] != '\0'; i++)
        {
            if (input[i] == '/')
            {
                input[i] = '-';
            }
        }
        if (isValidMonth(input))
        {
            strcpy(out, input);
            return;
        }
        printf("  Invalid month. Use MM-YYYY (example: 05-2026).\n");
    }
}

/* Read a description. '|' is replaced because it separates file fields. */
void readDescription(char *out)
{
    char input[INPUT_LEN];
    int  i;

    readLine("  Description (max 49 characters): ", input, sizeof(input));
    if (input[0] == '\0')
    {
        strcpy(input, "No description");
    }
    for (i = 0; input[i] != '\0'; i++)
    {
        if (input[i] == '|')
        {
            input[i] = '/';
        }
    }
    strncpy(out, input, DESC_LEN - 1);
    out[DESC_LEN - 1] = '\0';
    trimString(out);
}

/* Show the 7 categories and let the user pick one. */
void chooseCategory(char *out)
{
    int i;
    int choice;

    printf("\n  Select a category:\n");
    for (i = 0; i < CATEGORY_COUNT; i++)
    {
        printf("    %d. %s\n", i + 1, categoryNames[i]);
    }
    choice = readInt("  Enter choice (1-7): ", 1, CATEGORY_COUNT);
    strcpy(out, categoryNames[choice - 1]);    /* names are < 20 chars */
}

void pauseScreen(void)
{
    char dummy[INPUT_LEN];
    readLine("\n  Press Enter to continue...", dummy, sizeof(dummy));
}

/* ===================================================================
 *  DATA HELPER FUNCTIONS
 * =================================================================== */

/* Return the array position of a transaction ID, or -1 if not found. */
int findIndexById(int id)
{
    int i;
    for (i = 0; i < transactionCount; i++)
    {
        if (transactions[i].id == id)
        {
            return i;
        }
    }
    return -1;
}

/* New ID = (largest existing ID) + 1, so it can never be a duplicate. */
int generateNewId(void)
{
    int i;
    int maxId = 0;

    for (i = 0; i < transactionCount; i++)
    {
        if (transactions[i].id > maxId)
        {
            maxId = transactions[i].id;
        }
    }
    return maxId + 1;
}

/* Position of a category name in categoryNames[], or -1. */
int findCategoryIndex(const char *name)
{
    int i;
    for (i = 0; i < CATEGORY_COUNT; i++)
    {
        if (strcmp(categoryNames[i], name) == 0)
        {
            return i;
        }
    }
    return -1;
}

/* Does the transaction belong to 'month' (MM-YYYY)?  "" = every month. */
int matchesMonth(const Transaction *t, const char *month)
{
    if (month[0] == '\0')
    {
        return 1;
    }
    return strcmp(t->date + 3, month) == 0;     /* date+3 is "MM-YYYY" */
}

int countTransactionsInMonth(const char *month)
{
    int i;
    int count = 0;

    for (i = 0; i < transactionCount; i++)
    {
        if (matchesMonth(&transactions[i], month))
        {
            count++;
        }
    }
    return count;
}

/* Total income and total expenses (in paise) for a month or all time. */
void calculateTotals(const char *month, long long *incomePaise, long long *expensePaise)
{
    int i;

    *incomePaise  = 0;
    *expensePaise = 0;
    for (i = 0; i < transactionCount; i++)
    {
        if (!matchesMonth(&transactions[i], month))
        {
            continue;
        }
        if (transactions[i].type == 'I')
        {
            *incomePaise += toPaise(transactions[i].amount);
        }
        else
        {
            *expensePaise += toPaise(transactions[i].amount);
        }
    }
}

/* Fill paise[0..6] with the spending of each expense category. */
void calculateCategoryTotals(const char *month, long long paise[])
{
    int i;
    int index;

    for (i = 0; i < CATEGORY_COUNT; i++)
    {
        paise[i] = 0;
    }
    for (i = 0; i < transactionCount; i++)
    {
        if (transactions[i].type != 'E' || !matchesMonth(&transactions[i], month))
        {
            continue;
        }
        index = findCategoryIndex(transactions[i].category);
        if (index < 0)
        {
            index = CATEGORY_COUNT - 1;        /* unknown -> "Other" */
        }
        paise[index] += toPaise(transactions[i].amount);
    }
}

/* Index of the category with the largest spending, or -1 if all zero. */
int findHighestCategory(const long long paise[])
{
    int i;
    int best = -1;
    long long bestAmount = 0;

    for (i = 0; i < CATEGORY_COUNT; i++)
    {
        if (paise[i] > bestAmount)
        {
            bestAmount = paise[i];
            best = i;
        }
    }
    return best;
}

/* Savings rate = (income - expenses) / income x 100.
 * Returns 1 and sets *rate when income > 0, otherwise returns 0. */
int calculateSavingsRate(long long incomePaise, long long expensePaise, double *rate)
{
    if (incomePaise <= 0)
    {
        return 0;
    }
    *rate = (double)(incomePaise - expensePaise) * 100.0 / (double)incomePaise;
    return 1;
}

/* ===================================================================
 *  FILE HANDLING
 * =================================================================== */

/* Convert one text line of the data file into a Transaction.
 * Line format:  id|type|amount|category|date|description
 * Returns 1 if the line is valid, 0 if it is corrupted.            */
int parseTransactionLine(const char *line, Transaction *t)
{
    char typeChar;

    if (sscanf(line, "%d|%c|%lf|%19[^|]|%10[^|]|%49[^\n]",
               &t->id, &typeChar, &t->amount,
               t->category, t->date, t->description) != 6)
    {
        return 0;
    }
    t->type = typeChar;
    trimString(t->description);

    if (t->id <= 0)
    {
        return 0;
    }
    if (t->type != 'I' && t->type != 'E')
    {
        return 0;
    }
    if (!(t->amount > 0 && t->amount <= MAX_AMOUNT))   /* also rejects NaN */
    {
        return 0;
    }
    t->amount = roundMoney(t->amount);
    if (!isValidDate(t->date))
    {
        return 0;
    }
    if (t->description[0] == '\0')
    {
        return 0;
    }
    if (t->type == 'I' && strcmp(t->category, INCOME_CATEGORY) != 0)
    {
        return 0;
    }
    if (t->type == 'E' && findCategoryIndex(t->category) < 0)
    {
        return 0;
    }
    return 1;
}

/* Keep a copy of the original file when some lines were corrupted. */
void backupDataFile(void)
{
    FILE  *source = fopen(DATA_FILE, "rb");
    FILE  *copy;
    char   buffer[512];
    size_t bytes;

    if (source == NULL)
    {
        return;
    }
    copy = fopen(BACKUP_FILE, "wb");
    if (copy == NULL)
    {
        fclose(source);
        return;
    }
    while ((bytes = fread(buffer, 1, sizeof(buffer), source)) > 0)
    {
        fwrite(buffer, 1, bytes, copy);
    }
    fclose(source);
    fclose(copy);
}

/* Load the budget and all valid transactions from DATA_FILE. */
void loadData(void)
{
    FILE       *fp;
    char        line[LINE_LEN];
    Transaction t;
    double      budget;
    int         skipped = 0;

    fp = fopen(DATA_FILE, "r");
    if (fp == NULL)
    {
        printf("\n  No saved data file found. Starting with an empty tracker.\n");
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        trimString(line);
        if (line[0] == '\0')
        {
            continue;                          /* ignore blank lines */
        }

        if (strncmp(line, "BUDGET|", 7) == 0)
        {
            if (sscanf(line, "BUDGET|%lf", &budget) == 1
                && budget >= 0 && budget <= MAX_AMOUNT)
            {
                monthlyBudget = roundMoney(budget);
            }
            else
            {
                skipped++;
            }
            continue;
        }

        if (transactionCount >= MAX_TRANSACTIONS)
        {
            skipped++;                         /* array is full */
            continue;
        }
        if (parseTransactionLine(line, &t) && findIndexById(t.id) < 0)
        {
            transactions[transactionCount] = t;
            transactionCount++;
        }
        else
        {
            skipped++;                         /* corrupted or duplicate ID */
        }
    }
    fclose(fp);

    printf("\n  Loaded %d transaction(s) from %s.\n", transactionCount, DATA_FILE);
    if (skipped > 0)
    {
        backupDataFile();
        printf("  WARNING: %d corrupted/duplicate line(s) were skipped.\n", skipped);
        printf("  A copy of the original file was saved as %s.\n", BACKUP_FILE);
    }
}

/* Save everything. Writes to a temporary file first, so the old data
 * file is never damaged if something goes wrong half-way.
 * Returns 1 on success, 0 on failure.                              */
int saveData(void)
{
    FILE *fp;
    int   i;

    fp = fopen(TEMP_FILE, "w");
    if (fp == NULL)
    {
        printf("  ERROR: could not create %s. Data was NOT saved.\n", TEMP_FILE);
        return 0;
    }

    fprintf(fp, "BUDGET|%.2f\n", monthlyBudget);
    for (i = 0; i < transactionCount; i++)
    {
        fprintf(fp, "%d|%c|%.2f|%s|%s|%s\n",
                transactions[i].id, transactions[i].type, transactions[i].amount,
                transactions[i].category, transactions[i].date,
                transactions[i].description);
    }

    if (ferror(fp))
    {
        fclose(fp);
        remove(TEMP_FILE);
        printf("  ERROR: writing to file failed. Data was NOT saved.\n");
        return 0;
    }
    if (fclose(fp) != 0)
    {
        remove(TEMP_FILE);
        printf("  ERROR: could not finish writing the file. Data was NOT saved.\n");
        return 0;
    }

    remove(DATA_FILE);          /* Windows cannot rename over an existing file */
    if (rename(TEMP_FILE, DATA_FILE) != 0)
    {
        printf("  ERROR: could not replace %s. Your data is safe in %s.\n",
               DATA_FILE, TEMP_FILE);
        return 0;
    }
    return 1;
}

/* ===================================================================
 *  TABLE DISPLAY (all functions take a FILE* so they can print on the
 *  screen (stdout) or into a report file)
 * =================================================================== */

void printTableLine(FILE *out)
{
    fprintf(out, "+------+------------+------+---------------+-------------+---------------------+\n");
}

void printTableHeader(FILE *out)
{
    printTableLine(out);
    fprintf(out, "| %-4s | %-10s | %-4s | %-13s | %11s | %-19s |\n",
            "ID", "Date", "Type", "Category", "Amount", "Description");
    printTableLine(out);
}

void printTableRow(FILE *out, const Transaction *t)
{
    fprintf(out, "| %-4d | %-10s | %-4s | %-13s | %11.2f | %-19.19s |\n",
            t->id, t->date, (t->type == 'I') ? "INC" : "EXP",
            t->category, t->amount, t->description);
}

/* ===================================================================
 *  FEATURES 1-2: ADD INCOME / EXPENSE
 * =================================================================== */
void addTransaction(char type)
{
    Transaction t;

    printHeading(type == 'I' ? "ADD INCOME" : "ADD EXPENSE");

    if (transactionCount >= MAX_TRANSACTIONS)
    {
        printf("\n  Storage is full (%d transactions). Delete some first.\n", MAX_TRANSACTIONS);
        return;
    }

    t.id   = generateNewId();
    t.type = type;
    t.amount = readAmount("  Amount (" CURRENCY "): ", 0);

    if (type == 'I')
    {
        strcpy(t.category, INCOME_CATEGORY);
    }
    else
    {
        chooseCategory(t.category);
    }
    readDate("  Date DD-MM-YYYY (Enter = today): ", t.date);
    readDescription(t.description);

    transactions[transactionCount] = t;
    transactionCount++;

    if (saveData())
    {
        printf("\n  Saved! Transaction ID %d added.\n", t.id);
    }
    if (type == 'E')
    {
        checkBudgetAlert(t.date + 3);          /* budget check for that month */
    }
}

/* ===================================================================
 *  FEATURE 3: DISPLAY ALL
 * =================================================================== */
void showAllTransactions(void)
{
    int i;

    printHeading("ALL TRANSACTIONS");
    if (transactionCount == 0)
    {
        printf("\n  No transactions recorded yet.\n");
        return;
    }
    printf("  All amounts are in Indian Rupees (%s)\n", CURRENCY);
    printTableHeader(stdout);
    for (i = 0; i < transactionCount; i++)
    {
        printTableRow(stdout, &transactions[i]);
    }
    printTableLine(stdout);
    printf("  Total records: %d\n", transactionCount);
}

/* ===================================================================
 *  FEATURE 4: SEARCH (category or description, ignoring upper/lower)
 * =================================================================== */
void searchTransactions(void)
{
    char keyword[INPUT_LEN];
    char lowerKeyword[INPUT_LEN];
    char lowerCategory[CATEGORY_LEN];
    char lowerDescription[DESC_LEN];
    int  i;
    int  found = 0;

    printHeading("SEARCH TRANSACTIONS");
    if (transactionCount == 0)
    {
        printf("\n  No transactions to search.\n");
        return;
    }

    readLine("  Enter category or description keyword: ", keyword, sizeof(keyword));
    if (keyword[0] == '\0')
    {
        printf("  Search keyword cannot be empty.\n");
        return;
    }
    toLowerText(keyword, lowerKeyword);

    for (i = 0; i < transactionCount; i++)
    {
        toLowerText(transactions[i].category, lowerCategory);
        toLowerText(transactions[i].description, lowerDescription);

        if (strstr(lowerCategory, lowerKeyword) != NULL
            || strstr(lowerDescription, lowerKeyword) != NULL)
        {
            if (found == 0)
            {
                printTableHeader(stdout);
            }
            printTableRow(stdout, &transactions[i]);
            found++;
        }
    }

    if (found == 0)
    {
        printf("\n  No transaction matches '%s'.\n", keyword);
    }
    else
    {
        printTableLine(stdout);
        printf("  %d record(s) found.\n", found);
    }
}

/* ===================================================================
 *  FEATURE 5a: EDIT
 * =================================================================== */
void editTransaction(void)
{
    int          id, index, choice;
    Transaction *t;

    showAllTransactions();
    if (transactionCount == 0)
    {
        return;
    }

    id    = readInt("\n  Enter the ID to edit: ", 1, 99999999);
    index = findIndexById(id);
    if (index < 0)
    {
        printf("  No transaction with ID %d exists.\n", id);
        return;
    }
    t = &transactions[index];             /* pointer to the record to change */

    printTableHeader(stdout);
    printTableRow(stdout, t);
    printTableLine(stdout);

    printf("\n  What do you want to change?\n");
    printf("    1. Amount\n    2. Category\n    3. Date\n    4. Description\n    0. Cancel\n");
    choice = readInt("  Enter choice (0-4): ", 0, 4);

    switch (choice)
    {
        case 0:
            printf("  Edit cancelled.\n");
            return;
        case 1:
            t->amount = readAmount("  New amount (" CURRENCY "): ", 0);
            break;
        case 2:
            if (t->type == 'I')
            {
                printf("  Income always uses the category '%s'. Nothing changed.\n", INCOME_CATEGORY);
                return;
            }
            chooseCategory(t->category);
            break;
        case 3:
            readDate("  New date DD-MM-YYYY (Enter = today): ", t->date);
            break;
        case 4:
            readDescription(t->description);
            break;
    }

    if (saveData())
    {
        printf("\n  Transaction %d updated and saved.\n", id);
    }
    if (t->type == 'E')
    {
        checkBudgetAlert(t->date + 3);
    }
}

/* ===================================================================
 *  FEATURE 5b: DELETE
 * =================================================================== */
void deleteTransaction(void)
{
    int id, index, i;

    showAllTransactions();
    if (transactionCount == 0)
    {
        return;
    }

    id    = readInt("\n  Enter the ID to delete: ", 1, 99999999);
    index = findIndexById(id);
    if (index < 0)
    {
        printf("  No transaction with ID %d exists.\n", id);
        return;
    }

    printTableHeader(stdout);
    printTableRow(stdout, &transactions[index]);
    printTableLine(stdout);

    if (!readYesNo("  Delete this transaction permanently? (y/n): "))
    {
        printf("  Delete cancelled.\n");
        return;
    }

    /* Shift every later record one place to the left. */
    for (i = index; i < transactionCount - 1; i++)
    {
        transactions[i] = transactions[i + 1];
    }
    transactionCount--;

    if (saveData())
    {
        printf("\n  Transaction %d deleted and file updated.\n", id);
    }
}

/* ===================================================================
 *  FEATURES 6 & 12: TOTALS, BALANCE, SAVINGS RATE
 * =================================================================== */
void printSummaryBlock(const char *label, const char *month)
{
    long long income, expense;
    double    rate;

    calculateTotals(month, &income, &expense);

    printf("\n  --- %s ---\n", label);
    printf("  Total Income   : ");
    printMoney(stdout, income);
    printf("\n  Total Expenses : ");
    printMoney(stdout, expense);
    printf("\n  Balance        : ");
    printMoney(stdout, income - expense);
    printf("\n  Savings Rate   : ");
    if (calculateSavingsRate(income, expense, &rate))
    {
        printf("%.1f%%", rate);
        if (rate < 0)
        {
            printf("  (spending is more than income)");
        }
        printf("\n");
    }
    else
    {
        printf("Not available (no income recorded)\n");
    }
}

void showFinancialSummary(void)
{
    char today[DATE_LEN];
    char label[40];

    printHeading("FINANCIAL SUMMARY");
    if (transactionCount == 0)
    {
        printf("\n  No transactions recorded yet.\n");
        return;
    }
    printSummaryBlock("ALL TIME", "");

    getTodayDate(today);
    snprintf(label, sizeof(label), "THIS MONTH (%s)", today + 3);
    printSummaryBlock(label, today + 3);
}

/* ===================================================================
 *  FEATURES 7, 10, 11: CATEGORY SUMMARY + HIGHEST CATEGORY
 * =================================================================== */
void printCategoryBreakdown(FILE *out, const char *month)
{
    long long paise[CATEGORY_COUNT];
    long long total = 0;
    int       i, b, bars, best;
    double    percent;

    calculateCategoryTotals(month, paise);
    for (i = 0; i < CATEGORY_COUNT; i++)
    {
        total += paise[i];
    }
    if (total == 0)
    {
        fprintf(out, "  No expenses found for this period.\n");
        return;
    }

    fprintf(out, "  %-14s %12s %8s  %s\n", "Category", "Amount", "Share", "Graph (each # = 5%)");
    fprintf(out, "  -------------------------------------------------------------\n");
    for (i = 0; i < CATEGORY_COUNT; i++)
    {
        percent = (double)paise[i] * 100.0 / (double)total;
        bars    = (int)(percent / 5.0);
        fprintf(out, "  %-14s %12.2f %7.1f%%  ", categoryNames[i], paiseToRupees(paise[i]), percent);
        for (b = 0; b < bars; b++)
        {
            fputc('#', out);
        }
        fputc('\n', out);
    }
    fprintf(out, "  -------------------------------------------------------------\n");
    fprintf(out, "  %-14s %12.2f %7.1f%%\n", "TOTAL", paiseToRupees(total), 100.0);

    best = findHighestCategory(paise);
    if (best >= 0)
    {
        fprintf(out, "\n  Highest spending category: %s (", categoryNames[best]);
        printMoney(out, paise[best]);
        fprintf(out, ", %.1f%% of expenses)\n", (double)paise[best] * 100.0 / (double)total);
    }
}

void showCategorySummary(void)
{
    char month[MONTH_LEN];

    printHeading("CATEGORY-WISE EXPENSE SUMMARY");
    if (transactionCount == 0)
    {
        printf("\n  No transactions recorded yet.\n");
        return;
    }
    readMonth("  Month MM-YYYY (Enter = all time): ", month, 1);

    printf("\n  Period: %s   (amounts in Indian Rupees, %s)\n",
           month[0] == '\0' ? "All time" : month, CURRENCY);
    printCategoryBreakdown(stdout, month);
}

/* ===================================================================
 *  FEATURES 8 & 9: BUDGET + ALERTS (80% and 100%)
 * =================================================================== */
void setMonthlyBudget(void)
{
    printHeading("SET MONTHLY BUDGET");
    if (monthlyBudget > 0)
    {
        printf("  Current budget: ");
        printMoney(stdout, toPaise(monthlyBudget));
        printf("\n");
    }
    else
    {
        printf("  No budget is set yet.\n");
    }
    monthlyBudget = readAmount("  New monthly budget (" CURRENCY ", enter 0 to turn off): ", 1);

    if (saveData())
    {
        if (monthlyBudget > 0)
        {
            printf("\n  Budget set to ");
            printMoney(stdout, toPaise(monthlyBudget));
            printf(" per month. Alerts appear at 80%% and 100%%.\n");
        }
        else
        {
            printf("\n  Budget turned off.\n");
        }
    }
}

/* Draw a 20-character bar, e.g. [########............] */
void printProgressBar(FILE *out, double percent)
{
    int filled = (int)(percent / 5.0);
    int i;

    if (filled > 20)
    {
        filled = 20;
    }
    if (filled < 0)
    {
        filled = 0;
    }
    fputc('[', out);
    for (i = 0; i < 20; i++)
    {
        fputc(i < filled ? '#' : '.', out);
    }
    fputc(']', out);
}

/* Print an alert if spending in 'month' reached 80% or 100% of budget.
 * Comparisons use whole paise, so there are no rounding surprises:
 *   spent >= budget            -> 100% alert
 *   spent * 100 >= budget * 80 -> 80% warning                      */
void checkBudgetAlert(const char *month)
{
    long long income, spent;
    long long budgetPaise;
    double    percent;

    if (monthlyBudget <= 0)
    {
        return;
    }
    calculateTotals(month, &income, &spent);
    budgetPaise = toPaise(monthlyBudget);
    percent     = (double)spent * 100.0 / (double)budgetPaise;

    if (spent >= budgetPaise)
    {
        printf("\n  !!!!!!!!!!!!!!!! BUDGET ALERT (100%%) !!!!!!!!!!!!!!!!\n");
        printf("  You have used %.1f%% of your budget for %s.\n", percent, month);
        printf("  Budget: ");
        printMoney(stdout, budgetPaise);
        printf("   Spent: ");
        printMoney(stdout, spent);
        printf("   Over by: ");
        printMoney(stdout, spent - budgetPaise);
        printf("\n");
    }
    else if (spent * 100 >= budgetPaise * 80)
    {
        printf("\n  ** WARNING (80%%): you have used %.1f%% of your budget for %s. **\n",
               percent, month);
        printf("  Remaining: ");
        printMoney(stdout, budgetPaise - spent);
        printf("\n");
    }
}

void showBudgetStatus(void)
{
    char      month[MONTH_LEN];
    long long income, spent, budgetPaise;
    double    percent;

    printHeading("BUDGET STATUS");
    if (monthlyBudget <= 0)
    {
        printf("\n  No monthly budget is set. Use the 'Set monthly budget' option first.\n");
        return;
    }
    readMonth("  Month MM-YYYY (Enter = current month): ", month, 0);

    calculateTotals(month, &income, &spent);
    budgetPaise = toPaise(monthlyBudget);
    percent     = (double)spent * 100.0 / (double)budgetPaise;

    printf("\n  Month    : %s\n  Budget   : ", month);
    printMoney(stdout, budgetPaise);
    printf("\n  Spent    : ");
    printMoney(stdout, spent);
    printf("\n  Remaining: ");
    printMoney(stdout, budgetPaise - spent);
    printf("\n  Used     : %.1f%%  ", percent);
    printProgressBar(stdout, percent);
    printf("\n");

    if (spent * 100 < budgetPaise * 80)
    {
        printf("\n  Status: OK - you are below 80%% of your budget.\n");
    }
    else
    {
        checkBudgetAlert(month);
    }
}

/* ===================================================================
 *  RULE-BASED INSIGHTS (plain if-else rules, no AI, no prediction)
 *  Every message also names the rule that produced it.
 * =================================================================== */
void writeInsight(FILE *out, int csv, const char *text)
{
    if (csv)
    {
        fprintf(out, "Insight,\"%s\"\n", text);
    }
    else
    {
        fprintf(out, "  - %s\n", text);
    }
}

void printInsights(FILE *out, const char *month, int csv)
{
    long long   income, expense, categoryPaise[CATEGORY_COUNT];
    long long   budgetPaise, deficit;
    double      rate, share, leisureShare;
    int         best;
    char        text[250];
    const char *cur = csv ? "Rs." : CURRENCY;   /* CSV: avoid symbol problems in Excel */

    calculateTotals(month, &income, &expense);
    calculateCategoryTotals(month, categoryPaise);

    if (income == 0 && expense == 0)
    {
        writeInsight(out, csv, "No transactions found, so no insights can be made.");
        return;
    }

    /* Rule 1 and 2: income versus expenses, savings rate */
    if (income == 0)
    {
        writeInsight(out, csv, "No income recorded for this period, so the savings rate cannot be calculated.");
    }
    else if (expense > income)
    {
        deficit = expense - income;
        snprintf(text, sizeof(text),
                 "Rule 'expenses > income': you spent %s%lld.%02lld more than you earned.",
                 cur, deficit / 100, deficit % 100);
        writeInsight(out, csv, text);
    }
    else
    {
        calculateSavingsRate(income, expense, &rate);
        if (rate >= 20.0)
        {
            snprintf(text, sizeof(text),
                     "Rule 'savings rate >= 20%%': good! You saved %.1f%% of your income.", rate);
        }
        else
        {
            snprintf(text, sizeof(text),
                     "Rule 'savings rate < 20%%': you saved only %.1f%% of your income. Try to cut small expenses.",
                     rate);
        }
        writeInsight(out, csv, text);
    }

    /* Rule 3: one category takes more than 40% of all spending */
    best = findHighestCategory(categoryPaise);
    if (best >= 0)
    {
        share = (double)categoryPaise[best] * 100.0 / (double)expense;
        if (share > 40.0)
        {
            snprintf(text, sizeof(text),
                     "Rule 'one category > 40%%': %s is %.1f%% of your expenses. Check if it can be reduced.",
                     categoryNames[best], share);
            writeInsight(out, csv, text);
        }
    }

    /* Rule 4: Entertainment + Shopping (wants) above 30% of spending */
    if (expense > 0)
    {
        leisureShare = (double)(categoryPaise[findCategoryIndex("Entertainment")]
                              + categoryPaise[findCategoryIndex("Shopping")])
                       * 100.0 / (double)expense;
        if (leisureShare > 30.0)
        {
            snprintf(text, sizeof(text),
                     "Rule 'Shopping + Entertainment > 30%%': these 'wants' are %.1f%% of your spending.",
                     leisureShare);
            writeInsight(out, csv, text);
        }
    }

    /* Rule 5: budget usage (only when a month is chosen and budget exists) */
    if (month[0] != '\0' && monthlyBudget > 0)
    {
        budgetPaise = toPaise(monthlyBudget);
        if (expense >= budgetPaise)
        {
            writeInsight(out, csv, "Rule 'spent >= 100% of budget': you have reached or crossed your monthly budget.");
        }
        else if (expense * 100 >= budgetPaise * 80)
        {
            writeInsight(out, csv, "Rule 'spent >= 80% of budget': you are close to your monthly budget.");
        }
        else
        {
            writeInsight(out, csv, "Rule 'spent < 80% of budget': spending is within your budget.");
        }
    }
}

/* ===================================================================
 *  FEATURES 13 & 14: MONTHLY REPORT + EXPORT
 * =================================================================== */
void writeTextReport(FILE *out, const char *month)
{
    long long income, expense, budgetPaise;
    double    rate, percent;
    int       i;

    calculateTotals(month, &income, &expense);

    fprintf(out, "==============================================================\n");
    fprintf(out, "      SMART EXPENSE TRACKER - MONTHLY REPORT  (%s)\n", month);
    fprintf(out, "==============================================================\n");
    fprintf(out, "  Transactions in month : %d\n", countTransactionsInMonth(month));
    fprintf(out, "  Total Income          : ");
    printMoney(out, income);
    fprintf(out, "\n  Total Expenses        : ");
    printMoney(out, expense);
    fprintf(out, "\n  Balance               : ");
    printMoney(out, income - expense);
    fprintf(out, "\n  Savings Rate          : ");
    if (calculateSavingsRate(income, expense, &rate))
    {
        fprintf(out, "%.1f%%\n", rate);
    }
    else
    {
        fprintf(out, "Not available (no income recorded)\n");
    }

    fprintf(out, "\n  BUDGET\n");
    if (monthlyBudget > 0)
    {
        budgetPaise = toPaise(monthlyBudget);
        percent     = (double)expense * 100.0 / (double)budgetPaise;
        fprintf(out, "  Monthly budget        : ");
        printMoney(out, budgetPaise);
        fprintf(out, "\n  Used                  : %.1f%%  ", percent);
        printProgressBar(out, percent);
        fprintf(out, "\n  %s: ", expense > budgetPaise ? "Over budget by        " : "Remaining             ");
        printMoney(out, expense > budgetPaise ? expense - budgetPaise : budgetPaise - expense);
        fprintf(out, "\n");
    }
    else
    {
        fprintf(out, "  No monthly budget is set.\n");
    }

    fprintf(out, "\n  CATEGORY-WISE EXPENSES (amounts in Indian Rupees)\n");
    printCategoryBreakdown(out, month);

    fprintf(out, "\n  TRANSACTIONS OF THE MONTH\n");
    printTableHeader(out);
    for (i = 0; i < transactionCount; i++)
    {
        if (matchesMonth(&transactions[i], month))
        {
            printTableRow(out, &transactions[i]);
        }
    }
    printTableLine(out);

    fprintf(out, "\n  RULE-BASED INSIGHTS (simple if-else rules, not AI)\n");
    printInsights(out, month, 0);
    fprintf(out, "==============================================================\n");
}

/* Write text in CSV style: wrap in quotes and double any quote inside. */
static void writeCsvText(FILE *out, const char *text)
{
    int i;

    fputc('"', out);
    for (i = 0; text[i] != '\0'; i++)
    {
        if (text[i] == '"')
        {
            fputc('"', out);
        }
        fputc(text[i], out);
    }
    fputc('"', out);
}

void writeCsvReport(FILE *out, const char *month)
{
    long long income, expense, budgetPaise;
    long long categoryPaise[CATEGORY_COUNT];
    long long totalExpense = 0;
    double    rate;
    int       i;

    calculateTotals(month, &income, &expense);
    calculateCategoryTotals(month, categoryPaise);
    for (i = 0; i < CATEGORY_COUNT; i++)
    {
        totalExpense += categoryPaise[i];
    }

    fprintf(out, "Smart Expense Tracker - Monthly Report\n");
    fprintf(out, "Month,%s\n", month);
    fprintf(out, "Currency,INR\n\n");

    fprintf(out, "Summary,Value\n");
    fprintf(out, "Total Income,%.2f\n", paiseToRupees(income));
    fprintf(out, "Total Expenses,%.2f\n", paiseToRupees(expense));
    fprintf(out, "Balance,%.2f\n", paiseToRupees(income - expense));
    if (calculateSavingsRate(income, expense, &rate))
    {
        fprintf(out, "Savings Rate (%%),%.1f\n", rate);
    }
    else
    {
        fprintf(out, "Savings Rate (%%),N/A\n");
    }
    if (monthlyBudget > 0)
    {
        budgetPaise = toPaise(monthlyBudget);
        fprintf(out, "Monthly Budget,%.2f\n", paiseToRupees(budgetPaise));
        fprintf(out, "Budget Used (%%),%.1f\n", (double)expense * 100.0 / (double)budgetPaise);
    }
    else
    {
        fprintf(out, "Monthly Budget,Not set\n");
    }

    fprintf(out, "\nCategory,Amount,Share (%%)\n");
    for (i = 0; i < CATEGORY_COUNT; i++)
    {
        fprintf(out, "%s,%.2f,%.1f\n", categoryNames[i], paiseToRupees(categoryPaise[i]),
                totalExpense > 0 ? (double)categoryPaise[i] * 100.0 / (double)totalExpense : 0.0);
    }
    i = findHighestCategory(categoryPaise);
    fprintf(out, "Highest Spending Category,%s\n", i >= 0 ? categoryNames[i] : "N/A");

    fprintf(out, "\nID,Date,Type,Category,Amount,Description\n");
    for (i = 0; i < transactionCount; i++)
    {
        if (!matchesMonth(&transactions[i], month))
        {
            continue;
        }
        fprintf(out, "%d,%s,%s,%s,%.2f,", transactions[i].id, transactions[i].date,
                transactions[i].type == 'I' ? "Income" : "Expense",
                transactions[i].category, transactions[i].amount);
        writeCsvText(out, transactions[i].description);
        fputc('\n', out);
    }

    fprintf(out, "\n");
    printInsights(out, month, 1);
}

void showMonthlyReport(void)
{
    char month[MONTH_LEN];

    printHeading("MONTHLY REPORT");
    readMonth("  Month MM-YYYY (Enter = current month): ", month, 0);

    if (countTransactionsInMonth(month) == 0)
    {
        printf("\n  No transactions found for %s.\n", month);
        return;
    }
    printf("\n");
    writeTextReport(stdout, month);
}

void exportReport(void)
{
    char  month[MONTH_LEN];
    char  filename[40];
    int   choice;
    FILE *fp;

    printHeading("EXPORT REPORT");
    readMonth("  Month MM-YYYY (Enter = current month): ", month, 0);

    if (countTransactionsInMonth(month) == 0)
    {
        printf("\n  No transactions found for %s. Nothing to export.\n", month);
        return;
    }
    printf("\n  1. Text file (.txt)\n  2. CSV file (.csv, opens in Excel)\n");
    choice = readInt("  Enter choice (1-2): ", 1, 2);

    snprintf(filename, sizeof(filename), "report_%s.%s", month, choice == 1 ? "txt" : "csv");
    fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("  ERROR: could not create %s (is it open in another program?).\n", filename);
        return;
    }
    if (choice == 1)
    {
        writeTextReport(fp, month);
    }
    else
    {
        writeCsvReport(fp, month);
    }
    if (ferror(fp) || fclose(fp) != 0)
    {
        printf("  ERROR: writing %s failed.\n", filename);
        return;
    }
    printf("\n  Report exported successfully to %s\n", filename);
}

/* ===================================================================
 *  MAIN MENU
 * =================================================================== */
void showMainMenu(void)
{
    long long income, expense;

    calculateTotals("", &income, &expense);
    printHeading("SMART EXPENSE TRACKER");

    printf("  Balance: ");
    printMoney(stdout, income - expense);
    printf("   |   Records: %d/%d   |   Budget: ", transactionCount, MAX_TRANSACTIONS);
    if (monthlyBudget > 0)
    {
        printMoney(stdout, toPaise(monthlyBudget));
    }
    else
    {
        printf("Not set");
    }
    printf("\n\n");

    printf("   [1]  Add income               [7]  Financial summary\n");
    printf("   [2]  Add expense              [8]  Category summary\n");
    printf("   [3]  View all transactions    [9]  Set monthly budget\n");
    printf("   [4]  Search transactions      [10] Budget status\n");
    printf("   [5]  Edit transaction         [11] Monthly report\n");
    printf("   [6]  Delete transaction       [12] Export report\n");
    printf("   [0]  Save and exit\n\n");
}

int main(void)
{
    int choice;

    setupConsole();
    printf("\n  Welcome to the Smart Expense Tracker (all amounts in Indian Rupees)");
    loadData();

    do
    {
        showMainMenu();
        choice = readInt("  Enter your choice (0-12): ", 0, 12);

        switch (choice)
        {
            case 1:  addTransaction('I');     break;
            case 2:  addTransaction('E');     break;
            case 3:  showAllTransactions();   break;
            case 4:  searchTransactions();    break;
            case 5:  editTransaction();       break;
            case 6:  deleteTransaction();     break;
            case 7:  showFinancialSummary();  break;
            case 8:  showCategorySummary();   break;
            case 9:  setMonthlyBudget();      break;
            case 10: showBudgetStatus();      break;
            case 11: showMonthlyReport();     break;
            case 12: exportReport();          break;
            case 0:                           break;
        }

        if (choice != 0)
        {
            pauseScreen();
        }
    } while (choice != 0);

    if (saveData())
    {
        printf("\n  Data saved to %s. Goodbye!\n\n", DATA_FILE);
    }
    return 0;
}
