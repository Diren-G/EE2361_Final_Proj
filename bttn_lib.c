#include "xc.h"

// --- Button setup ---
void setupButtons(void){
    TRISBbits.TRISB10 = 1; // Set as input
    TRISBbits.TRISB11 = 1;

    AD1PCFGbits.PCFG10 = 1; // RB10 digital
    AD1PCFGbits.PCFG11 = 1; // RB11 digital

    CNPU1bits.CN15PUE = 1;  // Enable pull-up on RB11 (CN15)
    CNPU2bits.CN16PUE = 1;  // Enable pull-up on RB10 (CN16)
}
