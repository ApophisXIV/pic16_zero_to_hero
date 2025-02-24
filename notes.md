# ¿Cómo configurar el reloj?

Hay varias formas, vamos a ver desde la más 

```c
/* ------------------------------------------------------------ */
// Forma inicial (A registro "crudo")
volatile uint8_t *OSCON_REG = (uint8_t *)(0x8f);
asm("OSCON_REG eq 0x8f");

// Forma inicial usando mascaras de bits
void clk_use_internal(intosc_freq_t freq) {
    
    // SCS -> System clock select bit
    (*OSCON_REG) &= ~(1 << 0);    // Clear SCS bit
    (*OSCON_REG) |= (1 << 0);     // Set SCS bit -> We use internal clock
    
    // IRCF -> Internal oscillator frequency select bits
    // (*OSCON_REG) &= (~(1 << 6) | ~(1 << 5) | ~(1 << 4));
    (*OSCON_REG) &= ~(0b111 << 4);    // Clear internal oscillator frequency selection bits
    (*OSCON_REG) |= (freq << 4);

    // Validate if INTOSC is stable -> HTS (High Frequency 8MHz to 125kHz)
    //                              -> LTS (Low Frequency 31kHz)
    if (freq == F_31KHZ)
    while (~((*(OSCON_REG) >> 1) & ~(1)));
    else
    while (~((*OSCON_REG) >> 2 & ~(1)));
}
/* ------------------------------------------------------------ */

```

En esta opción definimos un puntero con la dirección física del registro obtenida de la hoja de datos. Sin embargo no es lo suficientemente legible la notación y además es propensa a errores por lo que se puede mejorar


```c
/* ---------------------------------------------------------------------- */
// Forma mejorada (Con campos de bits, más legible)
typedef struct {
    unsigned int SCS : 1;
    unsigned int LTS : 1;
    unsigned int HTS : 1;
    unsigned int OSTS : 1;
    unsigned int IRCF : 3;
} OSCON_REG_t;

volatile OSCON_REG_t *OSCCON_bits = (OSCON_REG_t *)(0x8f);
asm("OSCON_REG eq 0x8f");

void clk_use_internal_1(intosc_freq_t freq) {
    OSCCON_bits->SCS = 1;
    OSCCON_bits->IRCF = freq;

    if(freq == F_31KHZ)
        while(OSCCON_bits->LTS == 0);
    else
        while(OSCCON_bits->HTS == 0);
}
/* -------------------------------------------------------------------------- */
```

Una directiva del compilador XC8 para indicar las direcciones de las variables desde el codigo es `__at(x) = __atribute__(adress(x))`.

Azucar sintatica / sugar syntax -> += , -= , ... , &=, |=, ^= (XOR)

XOR Gate 
A | B | Y
0 | 0 | 0
0 | 1 | 1
1 | 0 | 1
1 | 1 | 0



## LCD

El LCD puede ser conectado de dos maneras por un bus de 4 bits o de 8 bits para la transferencia de data

### Modo bus de 4 bits

Este usa solo 4 pines del LCD para la transferencia de data, se manda los 4 bits mas significativos (que son desde el DB4 hasta el DB7 en el caso de el bus de 8 bits) y luego los menos significativos (los cuales son desde el DB0 hasta DB3)

Lo primero utilizando la funcion 6 (function set), seria configurar el modo 4 bits es decir  el DL = 0, el N = 1 de n lines y F= 0 es fuente del caracter premitiendo utilizar 32 caracteres.