#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MAX_TRX 100
#define PIN_CODE 1234
#define CSV_PATH 'transactions.csv'
typedef long long cents;
typedef enum {TRX_DEPOSIT = 1, TRX_WIDTHDRAW = 2} TrxType;
typedef struct {
    TrxType type;
    cents amount;
    cents balance_after;
    time_t ts;

} Transaction;
typedef struct{
    cents balance;
    Transaction history[MAX_TRX];
    int count;

} Account;
static void trim_newline(char * s) {
    if(!s) return;
    size_t n = strlen(s);
    if(n && s[n-1] == '\n') s[n-1] = 0;

}
static const char * trx_name(TrxType t) {
    return(t == TRX_DEPOSIT)? "DEPOSIT" : "WITHDRAW";
}
static int save_transactions_csv(const Transaction * t, const char *path) {
    FILE *f = fopen(path, 'a');
}
static int read_line(char * buf, size_t cap) {
    if(fgets(buf,(int)cap, stdin) == NULL) return 0;
    trim_newline(buf);
    return 1;
}
static int read_int(const char * prompt, int * out) {
    char buf[128];
    printf("%s", prompt);
    if(!read_line(buf, sizeof(buf))) return 0;
    char * end; long v = strtol(buf, &end,10);
    if(end == buf || * end!= '\0') return 0;
    *out = (int)v;
    return 1;
}
static int read_amount(const char * prompt, cents * out){
    char buf[128];
    printf("%s", prompt);
    if(!read_line(buf, sizeof(buf))) return 0;
    long long euros = 0, c = 0;
    int seen_dot = 0, c_digits = 0;
    for(char * p = buf; *p;++p) {
        if(*p>='0' && *p <= '9'){
            if((seen_dot)) {
                if(c_digits < 2) {
                    c = c*10 + (*p- '0'); 
                    c_digits++;
                }
                else {
                    return 0;   
                }
            }
            else {
                euros = euros * 10 + (*p-'0');
            }

        }
        else if(*p=='.' || *p==',') {
            if((seen_dot)) {
                return 0;
            }
            seen_dot = 1;
        }
        else if(*p == ' ') {continue;}
        else return 0;
    }
    if(c_digits == 1) c *= 10;
    *out = euros *100 + c;
    return 1;
}
static void add_trx(Account * acc, TrxType type, cents amount) {
    if(acc->count >= MAX_TRX) return;
    Transaction *t = &acc->history[acc->count++];
    t->type = type;
    t->amount = amount;
    t->balance_after = acc->balance;
    t->ts = time(NULL);

}
static int deposit(Account * acc, cents amount) {
    if(amount <=0) return 0;
    acc-> balance += amount;
    add_trx(acc, TRX_DEPOSIT, amount);
    return 1;

}
static int widthdraw(Account*acc, cents amount) {
if(amount <= 0) return 0;
if(acc->balance < amount) return -1;
acc->balance -= amount;
add_trx(acc, TRX_WIDTHDRAW, amount);
return 1;
}
static void print_money(cents c) {
    printf("%lld.%02lld", c/100, llabs(c%100));
}
static void show_balance(const Account * acc) {
    printf("\n== BALANCE == \n");
    printf("Current balance: ");
    print_money(acc->balance);
    printf("\n");
}
static void show_history(const Account * acc) {
    printf("\n== TRANSACTIONS (latest %d) ==\n", acc->count);
    if(acc->count ==0) {
        puts("No transactions yet.");
        return;
    }
    for(int i = acc->count - 1; i>=0;--i) {
        const Transaction *t = &acc->history[i];
        struct tm * tm = localtime(&t->ts);
        char tsbuf[32];
        strftime(tsbuf, sizeof(tsbuf), "%Y-%m-%d %H:%M:%S", tm);
        printf("[%s] %s: ", tsbuf, (t->type==TRX_DEPOSIT) ? "DEPOSIT" : "WITHDRAW");
        print_money(t->amount);
        printf(" | balance: ");
        print_money(t->balance_after);
        printf("\n");
    }
}
static int login(void){
    int attempts = 3;
    while(attempts--){
        int pin;
        if(!read_int("Enter PIN(4 digits): ", &pin)) {
            puts("Invalid input.");
            continue;
        }
        if(pin == PIN_CODE) {
            puts("Access granted.\n");
            return 1;
        }
        printf("Wrong PIN. Attempts left: %d\n", attempts);
    }
    puts("Card blocked. Bye.");
    return 0;
}
static void print_menu(void){
    puts("\n===== ATM =====");
    puts("1) Show balance");
    puts("2) Deposit");
    puts("3) Widthdraw");
    puts("4) History");
    puts("0) Exit");

}

int main(void) {
    Account acc = {.balance = 0, .count = 0};
    if(!login()) return 0;
    for(;;) {
        print_menu();
        int choice;
        if(!read_int("Select: ", &choice)) {
            puts("Enter a number from menu.");
            continue;
        }
        if(choice == 0) {
            puts("Goodbye!");
            break;
        }
        switch(choice){
            case 1 :
            show_balance(&acc);
            break;
            case 2: {
                cents amt;
                if(!read_amount("Amount to deposit(e.g., 100 or 50.99): ", &amt)) {
                    puts("Bad amount format.");
                    break;
                }
                if(!deposit(&acc, amt)) puts("Amount must be > 0!");
                else {
                    printf("Deposited successfully!");
                    print_money(amt);
                    printf(".\n");
                }
            }
            break;

            case 3: {
                cents amt;
                if(!read_amount("Amount to widthdraw: ", &amt)) {puts("Bad amount format.");break;}
                int ok = widthdraw(&acc, amt);
                if(ok == -1) puts("Insufficient funds.");
                else if(ok == 0) puts("Amount must be > 0.");
                else{ printf("Widthdrawn ");print_money(amt);printf(".\n");}
            } break;
            case 4: 
            show_history(&acc);
            break;
            default:
            puts("No such option.");
        }
    }
    return 0;
}
