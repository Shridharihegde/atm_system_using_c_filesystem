/*Authors name:shridhari Hegde
            Radhika Nayak
date created:23/10/2023
title :ATM requirements*/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

// Define a structure to store card information
struct CardInfo
{
    char name[50];
    char accountno[12];
    char serialNumber[11];
    char bankCode[5];
    char password[20];
    double accountBalance;
    char expiryDate[8]; // Format: MM/YYYY
};
struct Transaction
{
    char serialNumber[11];
    double amount;
    double balance; // Add the balance field
    char transactionDate[20];
};

struct ATMInfo
{
    double atmBalance;
    int dailyTransactionCount; // Keep track of daily transactions
    double dailyTransactionLimit;
    // Limit for daily transactions
};

// Initialize ATM balance and daily transaction count
double atmBalance = 10000.00; // Set the initial ATM balance
#define TRANSACTION_LIMIT 15000.00
struct ATMInfo atmInfo;

void initializeATM()
{
    atmInfo.dailyTransactionLimit = 25000.0;
    FILE *atmFile = fopen("atm_balance.txt", "r");
    if (atmFile != NULL)
    {
        fscanf(atmFile, "%lf", &atmBalance);
        fclose(atmFile);
    }

    // Read the last transaction date from the transaction log
    char lastTransactionDate[20];
    FILE *logFile = fopen("transactionlog.txt", "r");
    if (logFile != NULL)
    {
        while (fscanf(logFile, "%s", lastTransactionDate) == 1)
        {
            // Keep reading until you reach the end of the file to get the date of the last transaction
        }
        fclose(logFile);
    }

    // Get the current date
    time_t now;
    struct tm *tm_info;
    time(&now);
    tm_info = localtime(&now);
    char currentDate[20];
    strftime(currentDate, sizeof(currentDate), "%Y-%m-%d", tm_info);

    if (strcmp(currentDate, lastTransactionDate) != 0)
    {
        // It's a new day, reset the daily transaction count
        atmInfo.dailyTransactionCount = 0;
    }

    atmInfo.dailyTransactionLimit = 25000.0; // Set the daily transaction limit to 25,000
}

// Function prototypes
void initializeATM();
double getATMBalance();
void updateATMBalance(double amount);
void updateCardFile(const char *serialNumber, const struct CardInfo *newCardInfo);
void logDeposit(const char *serialNumber, double amount, double balance);
void performTransaction(struct CardInfo *cardInfo, double amount, int isDeposit);

// Function to update ATM balance
void updateATMBalance(double amount)
{
    atmBalance += amount;
    FILE *atmFile = fopen("atm_balance.txt", "w");
    if (atmFile != NULL)
    {
        fprintf(atmFile, "%.2lf\n", atmBalance);
        fclose(atmFile);
    }
}
// Function to get ATM balance
double getATMBalance()
{
    return atmBalance;
}
// Function to check if a card is valid
int isCardValid(const char *serialNumber, const char *bankCode, const char *password, struct CardInfo *cardInfo)
{
    FILE *file = fopen("carddatabase.txt", "r");
    if (file == NULL)
    {
        perror("Error opening card database file");
        return 0;
    }

    int cardFound = 0;

    // Read card information from the database file
    while (fscanf(file, "%s %s %s %s %s %s %lf",
                  cardInfo->name, cardInfo->accountno, cardInfo->serialNumber, cardInfo->bankCode, cardInfo->password, cardInfo->expiryDate, &cardInfo->accountBalance) == 7)
    {
        if (strcmp(serialNumber, cardInfo->serialNumber) == 0 &&
            strcmp(bankCode, cardInfo->bankCode) == 0 &&
            strcmp(password, cardInfo->password) == 0)
        {
            cardFound = 1;
            break; // Card is found
        }
    }

    fclose(file);

    if (cardFound)
    {
        // Check if the card is not expired
        time_t now;
        struct tm *tm_info;
        time(&now);
        tm_info = localtime(&now);

        int currentMonth = tm_info->tm_mon + 1;
        int currentYear = tm_info->tm_year + 1900;

        int cardMonth, cardYear;
        sscanf(cardInfo->expiryDate, "%2d/%4d", &cardMonth, &cardYear);

        if (cardYear > currentYear || (cardYear == currentYear && cardMonth >= currentMonth))
        {
            return 1; // Card is valid
        }
        else
        {
            return -1; // Card is expired
        }
    }
    else
    {
        return 0; // Card not found
    }
}
void readAdminIDs(char adminIDs[][20], int *adminCount)
{
    FILE *adminFile = fopen("admin_ids.txt", "r");
    if (adminFile != NULL)
    {
        *adminCount = 0;
        while (*adminCount < 10 && fscanf(adminFile, "%s", adminIDs[*adminCount]) == 1)
        {
            (*adminCount)++;
        }
        fclose(adminFile);
    }
}

int isAdmin(char *userID, char adminIDs[][20], int adminCount)
{
    for (int i = 0; i < adminCount; i++)
    {
        if (strcmp(userID, adminIDs[i]) == 0)
        {
            return 1; // User is an admin
        }
    }
    return 0; // User is not an admin
}

// Function to update the card's file with the new balance
// Function to update the card's file with the new balance
void updateCardFile(const char *serialNumber, const struct CardInfo *newCardInfo)
{
    FILE *file = fopen("carddatabase.txt", "r");
    FILE *tempFile = fopen("temp_card_database.txt", "w");

    if (file == NULL || tempFile == NULL)
    {
        perror("Error opening card files");
        return;
    }

    struct CardInfo cardInfo;
    int cardUpdated = 0;

    while (fscanf(file, "%s %s %s %s %s %s %lf",
                  cardInfo.name, cardInfo.accountno, cardInfo.serialNumber, cardInfo.bankCode, cardInfo.password, cardInfo.expiryDate, &cardInfo.accountBalance) == 7)
    {
        if (strcmp(serialNumber, cardInfo.serialNumber) == 0)
        {
            cardInfo.accountBalance = newCardInfo->accountBalance;
            cardUpdated = 1;
        }
        fprintf(tempFile, "%s %s %s %s %s %s %.2lf\n", cardInfo.name, cardInfo.accountno, cardInfo.serialNumber, cardInfo.bankCode, cardInfo.password, cardInfo.expiryDate, cardInfo.accountBalance);
    }

    fclose(file);
    fclose(tempFile);

    if (cardUpdated)
    {
        remove("carddatabase.txt");
        rename("temp_card_database.txt", "carddatabase.txt");
    }
    else
    {
        remove("temp_card_database.txt");
    }
}


void logTransaction(const char *serialNumber, double amount, double balance)
{
    FILE *logFile = fopen("transactionlog.txt", "a"); // Open the file in append mode
    if (logFile == NULL)
    {
        perror("Error opening transaction log file");
        return;
    }

    time_t now;
    struct tm *tm_info;
    time(&now);
    tm_info = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(logFile, "%s %s %.2lf %s %.2lf\n", serialNumber, timestamp, amount, "Dispensed", balance);
    fclose(logFile);
}

void printReceipt(const struct CardInfo *cardInfo, double amount)
{
    // Open a file for appending the receipt
    FILE *receiptFile = fopen("receipt.txt", "a");
    if (receiptFile == NULL)
    {
        perror("Error opening receipt file");
        return;
    }

    time_t now;
    struct tm *tm_info;
    time(&now);
    tm_info = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    int result = fprintf(receiptFile, "------------------------\n");
    if (result < 0)
    {
        perror("Error writing to receipt file");
        fclose(receiptFile);
        return;
    }

    fprintf(receiptFile, "------------------------\n");
    fprintf(receiptFile, "Receipt for ATM Transaction\n");
    fprintf(receiptFile, "------------------------\n");
    fprintf(receiptFile, "Card Holder: %s\n", cardInfo->name);
    fprintf(receiptFile, "Account Number: %s\n", cardInfo->accountno);
    fprintf(receiptFile, "Card Serial Number: %s\n", cardInfo->serialNumber);
    fprintf(receiptFile, "Transaction Amount: $%.2lf\n", amount);
    fprintf(receiptFile, "New Account Balance: $%.2lf\n", cardInfo->accountBalance);
    fprintf(receiptFile, "ATM Balance: $%.2lf\n", getATMBalance());
    fprintf(receiptFile, "Transaction Date: %s\n", timestamp); // Use the actual transaction date and time

    fclose(receiptFile);
    printf("Receipt printed successfully. You can find it in the 'receipt.txt' file.\n");
}

// Function to perform a successful transaction
void performTransaction(struct CardInfo *cardInfo, double amount, int isDeposit)
{
    if (amount > TRANSACTION_LIMIT)
    {
        printf("Transaction amount exceeds the transaction limit ($%.2lf). Unable to process the transaction.\n", TRANSACTION_LIMIT);
        return;
    }

    if (atmInfo.dailyTransactionCount >= atmInfo.dailyTransactionLimit)
    {
        printf("Daily transaction limit reached for this user. Unable to process the transaction.\n");
        return;
    }

    if (isDeposit)
    {
        if ((atmInfo.dailyTransactionCount + 1) * amount > atmInfo.dailyTransactionLimit)
        {
            printf("Daily transaction limit would be exceeded with this deposit. Unable to process the transaction.\n");
            return;
        }

        cardInfo->accountBalance += amount;
        printf("Deposit successful.\n");
        printf("Updated available balance: $%.2lf\n", cardInfo->accountBalance);
        updateCardFile(cardInfo->serialNumber, cardInfo);
        logDeposit(cardInfo->serialNumber, amount, cardInfo->accountBalance);
        printReceipt(cardInfo, amount);
        atmInfo.dailyTransactionCount++;
    }
    else
    {
        if (amount > cardInfo->accountBalance)
        {
            printf("Insufficient funds. Unable to process the transaction.\n");
        }
        else if (amount > atmBalance)
        {
            printf("ATM is running out of money. Unable to process the transaction.\n");
        }
        else
        {
            if (atmInfo.dailyTransactionCount >= atmInfo.dailyTransactionLimit)
            {
                printf("Daily transaction limit reached. Unable to process the transaction.\n");
            }
            else
            {
                if (amount > atmBalance)
                {
                    printf("ATM is running out of money. Unable to process the transaction.\n");
                }
                else
                {
                    if ((atmInfo.dailyTransactionCount + 1) * amount > atmInfo.dailyTransactionLimit)
                    {
                        printf("Daily transaction limit would be exceeded with this withdrawal. Unable to process the transaction.\n");
                        return;
                    }

                    printf("Transaction successful.\n");
                    cardInfo->accountBalance -= amount;
                    printf("Updated available balance: $%.2lf\n", cardInfo->accountBalance);
                    updateCardFile(cardInfo->serialNumber, cardInfo);
                    atmBalance -= amount;
                    updateATMBalance(-amount);
                    logTransaction(cardInfo->serialNumber, amount, cardInfo->accountBalance);
                    printReceipt(cardInfo, amount);
                    printf("Ejecting the card. Customer should take the card.\n");
                    printf("Dispensing $%.2lf...\n", amount);

                    atmInfo.dailyTransactionCount++;
                }
            }
        }
    }
}


// Function to retrieve and print the mini statement for a card
void displayMiniStatement(const char *serialNumber)
{
    FILE *logFile = fopen("transactionlog.txt", "r");
    if (logFile == NULL)
    {
        perror("Error opening transaction log file");
        return;
    }

    char line[100];
    struct Transaction lastFiveTransactions[5];
    int entryCount = 0;

    while (fgets(line, sizeof(line), logFile) != NULL)
    {
        char cardSerialNumber[11];
        struct Transaction transaction;
        char transactionType[20];

        if (sscanf(line, "%s %s %s %lf %s %lf",
                   cardSerialNumber, transaction.transactionDate,
                   transactionType, &transaction.amount,
                   transactionType, &transaction.balance) == 6)
        {
            if (strcmp(cardSerialNumber, serialNumber) == 0)
            {
                // Add the transaction to the history
                lastFiveTransactions[entryCount % 5] = transaction;
                entryCount++;
            }
        }
    }

    fclose(logFile);

    if (entryCount == 0)
    {
        printf("No recent transactions found for this card.\n");
    }
    else
    {
        printf("Mini Statement for Card Serial Number: %s\n", serialNumber);
        printf("---------------------------------------------------------------------\n");
        printf("Transaction Date and Time    | Type      | Amount     | Balance\n");
        printf("---------------------------------------------------------------------\n");

        // Print the last 5 entries or fewer if there are less than 5 entries
        int start = (entryCount > 5) ? (entryCount - 5) : 0;
        int end = entryCount;

        for (int i = start; i < end; i++)
        {
            printf("%s | Dispensed | $%.2lf | $%.2lf\n",
                   lastFiveTransactions[i % 5].transactionDate, lastFiveTransactions[i % 5].amount, lastFiveTransactions[i % 5].balance);
        }
    }
}

void logDeposit(const char *serialNumber, double amount, double balance)
{
    FILE *logFile = fopen("transactionlog.txt", "a"); // Open the file in append mode
    if (logFile == NULL)
    {
        perror("Error opening transaction log file");
        return;
    }

    time_t now;
    struct tm *tm_info;
    time(&now);
    tm_info = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(logFile, "%s %s %.2lf %s %.2lf\n", serialNumber, timestamp, amount, "Deposited", balance);
    fclose(logFile);
}
void resetDailyTransactionCount()
{
    char lastTransactionDate[20];
    FILE *logFile = fopen("transactionlog.txt", "r");
    if (logFile != NULL)
    {
        while (fscanf(logFile, "%s", lastTransactionDate) == 1)
        {
            // Keep reading until you reach the end of the file to get the date of the last transaction
        }
        fclose(logFile);
    }

    // Get the current date
    time_t now;
    struct tm *tm_info;
    time(&now);
    tm_info = localtime(&now);
    char currentDate[20];
    strftime(currentDate, sizeof(currentDate), "%Y-%m-%d", tm_info);

    if (strcmp(currentDate, lastTransactionDate) != 0)
    {
        // It's a new day, reset the daily transaction count
        atmInfo.dailyTransactionCount = 0;
    }
}
int main()
{
    char userType;
    char serialNumber[11];
    char bankCode[5];
    char password[20];
    struct CardInfo cardInfo;
    int attempts = 0;
    int isLoggedIn = 0; // Flag to track whether a user is logged in
    initializeATM();
    char adminIDs[10][20];
    int adminCount = 0;
    readAdminIDs(adminIDs, &adminCount);
    printf("Welcome to the ATM!\n");

    for (;;)
    {
        if (!isLoggedIn)
        {
            printf("Select User Type (A for Admin, C for Customer, X for exiting): ");
            scanf(" %c", &userType);

            atmInfo.dailyTransactionCount = 0; // Reset daily transaction count
        }
        else
        {
            // Allow the user to log out
            printf("Logged in as %s. Enter 'L' to log out, 'X' to exit, or continue your transaction: ", cardInfo.name);
            scanf(" %c", &userType);
        }

        if (userType == 'A' || userType == 'a')
        {
            // Admin mode
            char addAccountChoice;
            char adminID[20];

            printf("Enter your admin ID: ");
            scanf("%s", adminID);

            if (isAdmin(adminID, adminIDs, adminCount))
            {
                printf("Admin Mode - Do you want to add an account? (Y/N): ");
                scanf(" %c", &addAccountChoice);

                if (addAccountChoice == 'Y' || addAccountChoice == 'y')
{
    // Ask for account details and add them to the card database
    struct CardInfo newCard;

    printf("Enter Account Holder's Name: ");
    scanf("%s", newCard.name);

    printf("Enter Account Number: ");
    scanf("%s", newCard.accountno);

    printf("Enter Serial Number (11 characters): ");
    scanf("%s", newCard.serialNumber);

    printf("Enter Bank Code (4 characters): ");
    scanf("%s", newCard.bankCode);

    printf("Enter Password: ");
    scanf("%s", newCard.password);

    printf("Enter Expiry Date (MM/YYYY): ");
    scanf("%s", newCard.expiryDate);

    printf("Enter Initial Account Balance: ");
    scanf("%lf", &newCard.accountBalance);

    // Append the new account to the card database file
    FILE *file = fopen("carddatabase.txt", "a");
    if (file != NULL)
    {
        fprintf(file, "%s %s %s %s %s %s %.2lf\n", newCard.name, newCard.accountno, newCard.serialNumber, newCard.bankCode, newCard.password, newCard.expiryDate, newCard.accountBalance);
        fclose(file);
        printf("Account added successfully!\n");

        // Reset the daily transaction count for the new account
        atmInfo.dailyTransactionCount = 0;
    }
    else
    {
        perror("Error opening card database file");
    }
}

            }
        }

        else if (userType == 'C' || userType == 'c')
        {
            int customerExit = 0;

            while (!customerExit)
            {
                if (!isLoggedIn)
                {
                    if (attempts >= 3)
                    {
                        printf("Maximum login attempts reached. Please return the card.\n");
                        customerExit = 1;
                        break;
                    }

                    printf("Enter card serial number: ");
                    scanf("%s", serialNumber);
                    printf("Enter bank code: ");
                    scanf("%s", bankCode);
                    printf("Enter your password: ");
                    scanf("%s", password);

                    int cardStatus = isCardValid(serialNumber, bankCode, password, &cardInfo);

                    if (cardStatus == -1)
                    {
                        printf("Card is expired. Please meet the bank.\n");
                        return 0;
                    }
                    else if (cardStatus == 0)
                    {
                        attempts++;
                        printf("Invalid card details or expired card. Attempts left: %d\n", 3 - attempts);
                    }
                    else if (cardStatus == 1)
                    {
                        printf("Card is valid. Welcome, %s!\n", cardInfo.name);
                        isLoggedIn = 1;
                        attempts = 0;
                    }
                }

                
      if (isLoggedIn)
    {
        char transactionType;

        // Check if the user has exceeded the daily transaction limit before allowing any transactions
        if (atmInfo.dailyTransactionCount >= atmInfo.dailyTransactionLimit)
        {
            printf("Daily transaction limit reached. Unable to process any more transactions.\n");
            break;  // Exit the loop
        }

        printf("Enter 'd' for deposit, 'w' for withdrawal, 'm' for mini statement, 'L' to log out, 'X' to exit: ");
        scanf(" %c", &transactionType);

        if (transactionType == 'd' || transactionType == 'D')
        {
            double depositAmount;
            printf("Enter the amount for deposit: ");
            scanf("%lf", &depositAmount);

            if (depositAmount > 0)
            {
                performTransaction(&cardInfo, depositAmount, 1);
            }
            else
            {
                printf("Invalid deposit amount. Please enter a positive amount.\n");
            }
        }
        else if (transactionType == 'w' || transactionType == 'W')
        {
            if ((atmInfo.dailyTransactionCount + 1) * TRANSACTION_LIMIT > atmInfo.dailyTransactionLimit)
            {
                printf("Daily transaction limit would be exceeded with this withdrawal. Unable to process the transaction.\n");
            }
            else
            {
                double withdrawalAmount;
                printf("Enter the amount for withdrawal: ");
                scanf("%lf", &withdrawalAmount);

                if (withdrawalAmount <= 0)
                {
                    printf("Invalid withdrawal amount. Please enter a positive amount.\n");
                }
                else if (withdrawalAmount > cardInfo.accountBalance)
                {
                    printf("Unable to process the withdrawal. Entered amount is more than the balance.\n");
                }
                else if (withdrawalAmount > atmBalance)
                {
                    printf("Unable to process the withdrawal. ATM is running out of money.\n");
                }
                else
                {
                    performTransaction(&cardInfo, withdrawalAmount, 0);
                }
            }
        }
        else if (transactionType == 'm' || transactionType == 'M')
        {
            displayMiniStatement(cardInfo.serialNumber);
        }
        else if (transactionType == 'L' || transactionType == 'l')
        {
            printf("Logging out...\n");
            isLoggedIn = 0;
        }
        else if (transactionType == 'X' || transactionType == 'x')
        {
            printf("Exiting...\n");
            customerExit = 1;
            isLoggedIn = 0;
        }
        else
        {
            printf("Invalid transaction type. Please enter 'd', 'w', 'm', 'L', or 'X'.\n");
        }
    }
}

                   }           
        
        else if (userType == 'X' || userType == 'x')
        {
            if (isLoggedIn)
            {
                printf("Logging out...\n");
                isLoggedIn = 0;
            }
            else
            {
                printf("Exiting...\n");
                exit(0);
            }
        }
        else
        {
            printf("Invalid user type.\n");
            exit(0);
        }
    }
    return 0;
}  