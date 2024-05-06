/*
Authors name:shridhari Hegde
            Radhika Nayak
date created:
title :ATM requirements*/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include<stdlib.h>

// Define a structure to store card information
struct CardInfo {
    char name[50];
    char accountno[12];
    char serialNumber[11];
    char bankCode[5];
    char password[20];
    double accountBalance;
    char expiryDate[8]; // Format: MM/YYYY
};
struct Transaction {
    char serialNumber[11];
    double amount;
    char transactionDate[20];
};

struct ATMInfo {
    double atmBalance;
};



// Initialize ATM balance
double atmBalance = 10000.00; // Set the initial ATM balance

// Function prototypes
void initializeATM();
double getATMBalance();
void updateATMBalance(double amount);
void updateCardFile(const char *serialNumber, const struct CardInfo *newCardInfo);
void performTransaction(struct CardInfo *cardInfo, double amount);

void initializeATM() {
    FILE *atmFile = fopen("atm_balance.txt", "r");
    if (atmFile != NULL) {
        fscanf(atmFile, "%lf", &atmBalance);
        fclose(atmFile);
    }
}

// Function to update ATM balance
void updateATMBalance(double amount) {
    atmBalance += amount;
    FILE *atmFile = fopen("atm_balance.txt", "w");
    if (atmFile != NULL) {
        fprintf(atmFile, "%.2lf", atmBalance);
        fclose(atmFile);
    }
}
// Function to get ATM balance
double getATMBalance() {
    return atmBalance;
}
// Function to check if a card is valid
int isCardValid(const char *serialNumber, const char *bankCode, const char *password, struct CardInfo *cardInfo) {
    FILE *file = fopen("carddatabase.txt", "r");
    if (file == NULL) {
        perror("Error opening card database file");
        return 0;
    }

    int cardFound = 0;

    // Read card information from the database file
    while (fscanf(file, "%s %s %s %s %s %s %lf",
                  cardInfo->name,cardInfo->accountno,cardInfo->serialNumber, cardInfo->bankCode, cardInfo->password, cardInfo->expiryDate, &cardInfo->accountBalance) == 7) {
        if (strcmp(serialNumber, cardInfo->serialNumber) == 0 &&
            strcmp(bankCode, cardInfo->bankCode) == 0 &&
            strcmp(password, cardInfo->password) == 0) {
            cardFound = 1;
            break; // Card is found
        }
    }

    fclose(file);

    if (cardFound) {
        // Check if the card is not expired
        time_t now;
        struct tm *tm_info;
        time(&now);
        tm_info = localtime(&now);

        int currentMonth = tm_info->tm_mon + 1;
        int currentYear = tm_info->tm_year + 1900;

        int cardMonth, cardYear;
        sscanf(cardInfo->expiryDate, "%2d/%4d", &cardMonth, &cardYear);

        if (cardYear > currentYear || (cardYear == currentYear && cardMonth >= currentMonth)) {
            return 1; // Card is valid
        } else {
            return -1; // Card is expired
        }
    } else {
        return 0; // Card not found
    }
}


// Function to update the card's file with the new balance
void updateCardFile(const char *serialNumber, const struct CardInfo *newCardInfo) {
    FILE *file = fopen("card_database.txt", "r");
    FILE *tempFile = fopen("temp_card_database.txt", "w");

    if (file == NULL || tempFile == NULL) {
        perror("Error opening card files");
        return;
    }

    struct CardInfo cardInfo;
    int cardUpdated = 0;

    while (fscanf(file, "%s %s %s %s %s %s %lf",
                  cardInfo.name,cardInfo.accountno, cardInfo.serialNumber, cardInfo.bankCode, cardInfo.password, cardInfo.expiryDate, &cardInfo.accountBalance) == 7) {
        if (strcmp(serialNumber, cardInfo.serialNumber) == 0) {
            fprintf(tempFile, "%s %s %s %s %s %s %.2lf\n", cardInfo.name,cardInfo.accountno, cardInfo.serialNumber, cardInfo.bankCode, cardInfo.password, cardInfo.expiryDate, newCardInfo->accountBalance);
            cardUpdated = 1;
        } else {
            fprintf(tempFile, "%s %s %s %s %s %s %.2lf\n", cardInfo.name,cardInfo.accountno, cardInfo.serialNumber, cardInfo.bankCode, cardInfo.password, cardInfo.expiryDate, cardInfo.accountBalance);
        }
    }

    fclose(file);
    fclose(tempFile);

    if (cardUpdated) {
        remove("carddatabase.txt");
        rename("temp_card_database.txt", "carddatabase.txt");
    } else {
        remove("temp_card_database.txt");
    }
}

void logTransaction(const char *serialNumber, double amount, double balance, const char *transactionType) {
    FILE *logFile = fopen("transactionlog.txt", "a"); // Open the file in append mode
    if (logFile == NULL) {
        perror("Error opening transaction log file");
        return;
    }

    time_t now;
    struct tm *tm_info;
    time(&now);
    tm_info = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(logFile, "%s %s %.2lf %s %.2lf\n", serialNumber, timestamp, amount, transactionType, balance);
    fclose(logFile);
}

void printReceipt(const struct CardInfo *cardInfo, double amount, const char *transactionType) {
    // Open a file for writing the receipt
    FILE *receiptFile = fopen("receipt.txt", "w");
    if (receiptFile == NULL) {
        perror("Error opening receipt file");
        return;
    }
    int result = fprintf(receiptFile, "------------------------\n");
    if (result < 0) {
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
    fprintf(receiptFile, "Transaction Type: %s\n", transactionType);
    fprintf(receiptFile, "Transaction Amount: $%.2lf\n", amount);
    fprintf(receiptFile, "New Account Balance: $%.2lf\n", cardInfo->accountBalance);
    fprintf(receiptFile, "ATM Balance: $%.2lf\n", getATMBalance());
    fprintf(receiptFile, "Transaction Date: %s\n", cardInfo->expiryDate); // You might want to replace this with the actual transaction date and time

    fclose(receiptFile);
    printf("Receipt printed successfully. You can find it in the 'receipt.txt' file.\n");
}

// Function to perform a successful transaction
void performTransaction(struct CardInfo *cardInfo, double amount) {
    if (amount > 0) {
        // Deposit transaction
        cardInfo->accountBalance += amount;
        updateCardFile(cardInfo->serialNumber, cardInfo);
        logTransaction(cardInfo->serialNumber, amount, cardInfo->accountBalance, "Deposit");
        printf("Deposit successful.\n");
        printf("Updated account balance: $%.2lf\n", cardInfo->accountBalance);
        printReceipt(cardInfo, amount, "Deposit"); // Print receipt for deposit
    } else if (amount > cardInfo->accountBalance) {
        printf("Insufficient funds. Unable to process the transaction.\n");
    } else if (amount > atmBalance) {
        printf("ATM is running out of money. Unable to process the transaction.\n");
    } else {
        printf("Transaction successful.\n");
        cardInfo->accountBalance -= amount;
        printf("Updated available balance: $%.2lf\n", cardInfo->accountBalance);
        updateCardFile(cardInfo->serialNumber, cardInfo);
        atmBalance -= amount;
        updateATMBalance(-amount);
        printf("Printing receipt...\n");

        // Print the receipt for withdrawal
        printReceipt(cardInfo, amount, "Withdrawal");

        printf("Ejecting the card. Customer should take the card.\n");
        printf("Dispensing $%.2lf...\n");

        // Log the transaction with the updated balance
        logTransaction(cardInfo->serialNumber, amount, cardInfo->accountBalance, "Withdrawal");
    }
}

int main() {
    char userType;
    char serialNumber[11];
    char bankCode[5];
    char password[20];
    struct CardInfo cardInfo;
    int attempts = 0;
    initializeATM();

    printf("Welcome to the ATM!\n");

    // Ask the user to select their user type (admin or customer)
    for (;;) {
        printf("Select User Type (A for Admin, C for Customer, X for exiting): ");
        scanf(" %c", &userType);

        if (userType == 'A' || userType == 'a') {
            // Admin mode (unchanged)
        } else if (userType == 'C' || userType == 'c') {
            // Customer mode
            while (attempts < 3) {
                printf("Enter card serial number: ");
                scanf("%s", serialNumber);
                printf("Enter bank code: ");
                scanf("%s", bankCode);
                printf("Enter your password: "); // Prompt for password
                scanf("%s", password); // Read the password

                int cardStatus = isCardValid(serialNumber, bankCode, password, &cardInfo);

                if (cardStatus == -1) {
                    printf("Card is expired. Please meet the bank.\n");
                    return 0;
                } else if (cardStatus == 0) {
                    attempts++;
                    if (attempts < 3) {
                        printf("Invalid card details or expired card. Please try again. Attempts left: %d\n", 3 - attempts);
                    } else {
                        printf("Invalid card details or expired card. Please return the card.\n");
                    }
                } else if (cardStatus == 1) {
                    printf("Card is valid. Welcome, %s!\n", cardInfo.name);

                    char transactionType;
                    printf("Select Transaction Type (W for Withdraw, D for Deposit): ");
                    scanf(" %c", &transactionType);

                    if (transactionType == 'W' || transactionType == 'w') {
                        double transactionAmount;
                        printf("Enter the amount for the withdrawal: ");
                        scanf("%lf", &transactionAmount);

                        performTransaction(&cardInfo, -transactionAmount); // Withdrawal is a negative amount
                    } else if (transactionType == 'D' || transactionType == 'd') {
                        double transactionAmount;
                        printf("Enter the amount for the deposit: ");
                        scanf("%lf", &transactionAmount);

                        // Update the account balance and file
                        cardInfo.accountBalance += transactionAmount;
                        updateCardFile(cardInfo.serialNumber, &cardInfo);

                        printf("Deposit successful.\n");
                        printf("Updated account balance: $%.2lf\n", cardInfo.accountBalance);
                    } else {
                        printf("Invalid transaction type.\n");
                    }

                    break; // Exit the loop after a successful transaction
                }
            }
        } else if (userType == 'X' || userType == 'x') {
            printf("Exiting...\n");
            exit(0); // Exit the program
        } else {
            printf("Invalid user type.\n");
            exit(0);
        }
    }

    return 0;
}