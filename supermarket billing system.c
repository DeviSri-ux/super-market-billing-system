#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#define MAX_ITEMS 100
#define MAX_NAME_LEN 50
#define PHONE_LEN 10

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    double qty_orig;       // original qty user entered
    double qty_std;        // converted quantity (kg or L or pcs)
    double price_per_std;  // price per kg or L or pcs
    double total;
    int unit;              // 1 kg, 2 g, 3 L, 4 ml, 5 mg, 6 pcs
} Item;

Item cart[MAX_ITEMS];
int itemCount = 0;

/* -------------------------------------------------------------
   UTILITY FUNCTIONS
--------------------------------------------------------------*/

void chomp(char *s){
    size_t L = strlen(s);
    if(L > 0 && s[L-1] == '\n')
        s[L-1] = '\0';
}

void inputLine(char *buf, int size){
    if(!fgets(buf,size,stdin)){ buf[0]='\0'; return; }
    chomp(buf);
}

int isValidLettersOnly(const char *s){
    int seen=0;
    for(int i=0;s[i];i++){
        if(s[i]==' ') continue;
        if(!isalpha((unsigned char)s[i])) return 0;
        seen=1;
    }
    return seen;
}

int isValidPhone(const char *s){
    if(strlen(s)!=PHONE_LEN) return 0;
    for(int i=0;s[i];i++)
        if(!isdigit((unsigned char)s[i])) return 0;
    return 1;
}

int inputPositiveInt(const char *prompt){
    char buf[100];
    while(1){
        printf("%s",prompt);
        inputLine(buf,sizeof(buf));
        int valid=1;
        for(int i=0;buf[i];i++)
            if(!isdigit((unsigned char)buf[i])) valid=0;
        if(valid && atoi(buf)>0) return atoi(buf);

        printf("Invalid positive integer!\n");
    }
}

double inputPositiveFloat(const char *prompt){
    char buf[100];
    while(1){
        printf("%s",prompt);
        inputLine(buf,sizeof(buf));
        int dot=0,valid=1;
        for(int i=0;buf[i];i++){
            if(buf[i]=='.') dot++;
            else if(!isdigit((unsigned char)buf[i])) valid=0;
            if(dot>1) valid=0;
        }
        if(valid && atof(buf)>0) return atof(buf);

        printf("Invalid number!\n");
    }
}

void inputItemName(char *out,int size){
    char buf[200];
    while(1){
        printf("Enter item name: ");
        inputLine(buf,sizeof(buf));
        if(isValidLettersOnly(buf)){
            strncpy(out,buf,size-1); out[size-1]='\0';
            return;
        }
        printf("Invalid! Letters only.\n");
    }
}

void inputCustomerName(char *out,int size){
    char buf[200];
    while(1){
        printf("Enter customer name: ");
        inputLine(buf,sizeof(buf));
        if(isValidLettersOnly(buf)){
            strncpy(out,buf,size-1); out[size-1]='\0';
            return;
        }
        printf("Invalid name.\n");
    }
}

void inputPhone(char *out,int size){
    char buf[50];
    while(1){
        printf("Enter phone number (10 digits): ");
        inputLine(buf,sizeof(buf));
        if(isValidPhone(buf)){
            strncpy(out,buf,size-1); out[size-1]='\0';
            return;
        }
        printf("Invalid phone.\n");
    }
}

int idIndex(int id){
    for(int i=0;i<itemCount;i++)
        if(cart[i].id==id) return i;
    return -1;
}

void normalizeName(const char *src,char *dst,size_t size){
    int j=0;
    for(int i=0;src[i] && j<size-1;i++){
        if(src[i]==' '){
            if(j>0 && dst[j-1]==' ') continue;
            dst[j++]=' ';
        } else dst[j++]=tolower(src[i]);
    }
    if(j>0 && dst[j-1]==' ') j--;
    dst[j]='\0';
}

int namesMatch(const char *a,const char *b){
    char x[100],y[100];
    normalizeName(a,x,sizeof(x));
    normalizeName(b,y,sizeof(y));
    return strcmp(x,y)==0;
}

const char* unitName(int u){
    switch(u){
        case 1: return "kg";
        case 2: return "g";
        case 3: return "L";
        case 4: return "ml";
        case 5: return "mg";
        case 6: return "pcs";
    }
    return "";
}


/* -------------------------------------------------------------
   ADD ITEM – WITH PROPER CONVERSION (g?kg, mg?kg, ml?L)
--------------------------------------------------------------*/

void addItem(){
    if(itemCount>=MAX_ITEMS){
        printf("Cart full!\n");
        return;
    }

    int id;
    char name[MAX_NAME_LEN];

    while(1){
        id = inputPositiveInt("Enter item ID: ");
        int idx = idIndex(id);

        /* New ID */
        if(idx == -1){
            inputItemName(name,sizeof(name));
            break;
        }

        /* Existing ID - must verify name */
        printf("ID exists for '%s'.\n", cart[idx].name);
        printf("Enter SAME name to update quantity.\n");

        inputItemName(name,sizeof(name));

        if(!namesMatch(name,cart[idx].name)){
            printf("Name mismatch. Try again.\n");
            continue;
        }

        /* Updating same item */
        int unit = cart[idx].unit;
        printf("Existing unit: %s\n",unitName(unit));

        double qty = inputPositiveFloat("Enter quantity to add: ");

        double price = inputPositiveFloat(
            (unit==1||unit==2||unit==5) ?
                "Enter price per kg: " :
            (unit==3||unit==4) ?
                "Enter price per liter: " :
                "Enter price per piece: "
        );

        double std=0,total=0;

        switch(unit){
            case 1: std = qty; break;
            case 2: std = qty/1000; break;        // gram ? kg
            case 5: std = qty/1000000; break;     // mg ? kg
            case 3: std = qty; break;
            case 4: std = qty/1000; break;        // ml ? L
            case 6: std = qty; break;
        }

        total = std * price;

        cart[idx].qty_orig += qty;
        cart[idx].qty_std  += std;
        cart[idx].price_per_std = price;
        cart[idx].total += total;

        printf("Updated successfully!\n");
        return;
    }

    /* New item entry */
    printf("Select unit: \n1)kg  2)g  3)L  4)ml  5)mg  6)pcs\n");

    int unit;
    while(1){
        unit = inputPositiveInt("Choice: ");
        if(unit>=1 && unit<=6) break;
        printf("Invalid choice.\n");
    }

    double qty = inputPositiveFloat("Enter quantity: ");

    double price = inputPositiveFloat(
        (unit==1||unit==2||unit==5) ?
            "Enter price per kg: " :
        (unit==3||unit==4) ?
            "Enter price per liter: " :
            "Enter price per piece: "
    );

    double std=0,total=0;

    switch(unit){
        case 1: std = qty; break;
        case 2: std = qty/1000; break;        // g?kg
        case 5: std = qty/1000000; break;     // mg?kg
        case 3: std = qty; break;
        case 4: std = qty/1000; break;        // ml?L
        case 6: std = qty; break;
    }

    total = std * price;

    Item it;
    it.id=id;
    strcpy(it.name,name);
    it.unit=unit;
    it.qty_orig=qty;
    it.qty_std=std;
    it.price_per_std=price;
    it.total=total;

    cart[itemCount++] = it;

    printf("Item added successfully.\n");
}

/* -------------------------------------------------------------
   DISPLAY CART
--------------------------------------------------------------*/

void displayCart(){
    if(itemCount==0){ printf("Cart empty.\n"); return; }

    printf("\n---------------- CART ----------------\n");
    printf("ID   Name               Qty    Unit   Price/Kg(L)   Total\n");
    printf("-----------------------------------------------------------\n");

    for(int i=0;i<itemCount;i++){
        printf("%-4d %-18s %-6.3f %-5s %-12.2f %.2f\n",
               cart[i].id, cart[i].name,
               cart[i].qty_orig, unitName(cart[i].unit),
               cart[i].price_per_std, cart[i].total);
    }
}

/* -------------------------------------------------------------
   SUPER MARKET BOX STYLE BILL
--------------------------------------------------------------*/

void generateBill(){
    if(itemCount==0){
        printf("Cart empty!\n");
        return;
    }

    char customer[100],phone[20];
    inputCustomerName(customer,sizeof(customer));
    inputPhone(phone,sizeof(phone));

    /* Load previous */
    char fname[200];
    sprintf(fname,"customer_%s.dat",phone);

    double prev=0;
    FILE *fp=fopen(fname,"r");
    if(fp){ fscanf(fp,"%lf",&prev); fclose(fp); }

    double subtotal=0;
    for(int i=0;i<itemCount;i++) subtotal+=cart[i].total;

    double combined = prev + subtotal;
    double discRate = (combined>20000)?15 : (combined>10000)?10 : (combined>5000)?5 : 0;

    double discount = subtotal*(discRate/100.0);
    double taxable = subtotal - discount;
    double gst = taxable * 0.18;
    double grand = taxable + gst;
    double newTotal = prev + grand;

    /* Save new */
    fp=fopen(fname,"w");
    if(fp){ fprintf(fp,"%.2f",newTotal); fclose(fp); }

    time_t t=time(NULL);
    char ts[100];
    strftime(ts,sizeof(ts),"%d-%m-%Y %H:%M:%S",localtime(&t));

    /* ---------------- BILL OUTPUT ----------------- */
    printf("\n+--------------------------------------------------------------+\n");
    printf("|                      SUPER MARKET BILL                       |\n");
    printf("+--------------------------------------------------------------+\n");
    printf("Date/Time : %s\n",ts);
    printf("Customer  : %s\n",customer);
    printf("Phone     : %s\n",phone);
    printf("+--------------------------------------------------------------+\n");
    printf("| ID | Item Name       | Qty     | Unit | Rate(per KG/L) |Total|\n");
    printf("+--------------------------------------------------------------+\n");

    for(int i=0;i<itemCount;i++){
        printf("| %-2d | %-15s | %-7.3f | %-4s | %-14.2f | %5.2f |\n",
               cart[i].id, cart[i].name,
               cart[i].qty_orig, unitName(cart[i].unit),
               cart[i].price_per_std, cart[i].total);
    }

    printf("+--------------------------------------------------------------+\n");
    printf("Subtotal             : %.2f\n",subtotal);
    printf("Previous Purchase    : %.2f\n",prev);
    printf("Discount (%.0f%%)        : %.2f\n",discRate,discount);
    printf("Taxable Amount       : %.2f\n",taxable);
    printf("GST (18%%)            : %.2f\n",gst);
    printf("Grand Total          : %.2f\n",grand);
    printf("UPDATED TOTAL        : %.2f\n",newTotal);
    printf("+--------------------------------------------------------------+\n");
}

/* -------------------------------------------------------------
   MAIN MENU
--------------------------------------------------------------*/

int main(){
    while(1){
        printf("\n=============== MENU ===============\n");
        printf("1. Add Item\n");
        printf("2. Display Cart\n");
        printf("3. Generate Bill\n");
        printf("4. Clear Cart\n");
        printf("5. Exit\n");

        int ch = inputPositiveInt("Enter choice: ");

        switch(ch){
            case 1: addItem(); break;
            case 2: displayCart(); break;
            case 3: generateBill(); break;
            case 4: itemCount=0; printf("Cart cleared.\n"); break;
            case 5: return 0;
            default: printf("Invalid.\n");
        }
    }
}
