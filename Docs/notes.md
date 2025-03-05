# ¿Cómo configurar el reloj?

Hay varias formas, vamos a ver desde la más "cruda" hasta la "bare metal". SPOILER: Son lo mismo pero la bare metal es más elegante.

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


## ¿Cómo ver si un byte tiene un unico bit en particular en 1?
La opción "simple" y a priori clara es la siguiente función:
```c
bool is_single_bit_set(gpio_pin_t pin) {
    return ~((pin ^ GPIO_0) && (pin ^ GPIO_1) && (pin ^ GPIO_2) && (pin ^ GPIO_3) && (pin ^ GPIO_4) && (pin ^ GPIO_5) && (pin ^ GPIO_6) && (pin ^ GPIO_7));
}
```

Sin embargo hay una forma más eficiente de hacerlo que es la siguiente:
```c
bool is_single_bit_set(gpio_pin_t pin) {
    return pin && !(pin & (pin - 1))
}
```

## Las familias viejas de PICs no tienen registro LATx
En estos casos al trabajar sobre el registro PORTx para escribir en los pines configurados como salida podemos presenciar ciertos 
problemas producto de la forma en que se realizan operaciones de escritura.
Estas son del tipo RMW (Read Modify Write) por lo que cualquier operacion de escritura primero implica la lectura del registro, 
un almacenamiento temporal de la información y luego la escritura sobre el mismo registro.
![RMW_PORTx](<Img/rmw_portx.png>)

En la imagen se ve que para el siguiente código la salida B0 debería ser 1 pero vemos un pulso "espurio" producto de un comportamiento inespecifico de la forma en que se realiza la escritura del registro de forma directa, es decir desreferenciar el puntero del registro y luego escribir sobre el mismo mediante bitwise.

```c
int main() {

    clk_use_external();

    gpio_config(GPIO_PORTB, GPIO_0, GPIO_OUTPUT_HIGH);
    gpio_config(GPIO_PORTB, GPIO_1, GPIO_OUTPUT_LOW);
    gpio_config(GPIO_PORTC, GPIO_5, GPIO_OUTPUT_HIGH);
    gpio_config(GPIO_PORTC, GPIO_4, GPIO_OUTPUT_LOW);

    gpio_write_pin(GPIO_PORTB, GPIO_1, GPIO_HIGH);
    gpio_write_pin(GPIO_PORTB, GPIO_0, GPIO_LOW);

    while (1) {
        gpio_write_pin(GPIO_PORTB, GPIO_0, GPIO_LOW);
        gpio_write_pin(GPIO_PORTB, GPIO_1, GPIO_HIGH);
        gpio_write_pin(GPIO_PORTC, GPIO_5, GPIO_LOW);
        gpio_write_pin(GPIO_PORTC, GPIO_4, GPIO_HIGH);
        __delay_ms(500);
        gpio_write_pin(GPIO_PORTB, GPIO_0, GPIO_HIGH);
        gpio_write_pin(GPIO_PORTB, GPIO_1, GPIO_LOW);
        gpio_write_pin(GPIO_PORTC, GPIO_5, GPIO_HIGH);
        gpio_write_pin(GPIO_PORTC, GPIO_4, GPIO_LOW);
        __delay_ms(500);
    }

    return 0;
}
```
La solución a esto (no necesaria en familias que implementan LATx) es preservar un estado "virtual" del registro PORTx, y luego usarlo para escribir en el registro PORTx.

```c

// Definimos la direccion de los registros PORTx.
#define PORTA_REG ((volatile uint8_t *) 0x04) 
#define PORTB_REG ((volatile uint8_t *) 0x05) 
#define PORTC_REG ((volatile uint8_t *) 0x07) 
// De igual forma con los registros TRISx
#define TRISA_REG ((volatile uint8_t *) 0x85) 
#define TRISB_REG ((volatile uint8_t *) 0x86)
#define TRISC_REG ((volatile uint8_t *) 0x87)

// Creamos un arreglo de punteros a registros accesibles mediante un enum de puertos
volatile uint8_t *port_lut [] = {[GPIO_PORTA]=PORTA_REG, [GPIO_PORTB]PORTB_REG, [GPIO_PORTC]=PORTC_REG}; 
volatile uint8_t *tris_lut [] = {[GPIO_PORTA]=TRISA_REG, [GPIO_PORTB]TRISB_REG, [GPIO_PORTC]=TRISC_REG};

// Establecemos un estado virtual de los registros PORTx.
static uint8_t portx_state [] = {[GPIO_PORTA]=0, [GPIO_PORTB]=0, [GPIO_PORTC]=0}; 

void gpio_config(uint8_t port, uint8_t pin, uint8_t mode) {
     = (*tris_lut[port] & ~(1<<pin)) | (mode<<pin);
}


// La funcion debe respetar la operatoria RMW (Read-Modify-Write)
void gpio_write_pin(uint8_t port, uint8_t pin, uint8_t value) {
    
    // Primero trabajamos el valor deseado sobre un el estado virtual del registro PORTx. (Operacion Read-Modify)
    portx_state[port] = (portx_state[port] & ~(1 << pin)) | (value << pin);

    // Luego escribimos el valor en el registro PORTx. (Operacion Write)
    *port_lut[port] = portx_state[port];
}

```

## LCD

El LCD puede ser conectado de dos maneras por un bus de 4 bits o de 8 bits para la transferencia de data

### Modo bus de 4 bits

Este usa solo 4 pines del LCD para la transferencia de data, se manda los 4 bits mas significativos (que son desde el DB4 hasta el DB7 en el caso de el bus de 8 bits) y luego los menos significativos (los cuales son desde el DB0 hasta DB3)

Lo primero utilizando la funcion 6 (function set), seria configurar el modo 4 bits es decir  el DL = 0, el N = 1 de n lines y F= 0 es fuente del caracter premitiendo utilizar 32 caracteres.

## FSM (Finite State Machine)
```c
typedef enum {
    BUSCAR_INGREDIENTES,
    MEZCLAR_INGREDIENTES,
    METER_AL_HORNO,
    ENFRIAMIENTO,
    IDLE    // En espera
} cocinar_bizcochuelo_state_t;

cocinar_bizcochuelo_state_t estado = BUSCAR_INGREDIENTES;

static bool esta_en_el_horno = false;

void obtener_estado_fsm() {
    return estado;
}

void fsm_run(cocinar_bizcochuelo_state_t estado_actual) {
    switch (estado_actual) {
    case BUSCAR_INGREDIENTES:
        comprar_ingredientes();
        almacenar_ingredientes();
        estado = MEZCLAR_INGREDIENTES;
        break;

    case MEZCLAR_INGREDIENTES:
        separar_claras();
        batir_claras(3min);
        agregar_azucar(200);
        agregar_vainilla(100);
        estado = METER_AL_HORNO;
        break;

    case METER_AL_HORNO:
        meter_horno(temp, tiempo, sonar_alarma);    // Esta es una funcion bloqueante
        esta_en_el_horno = true;
        estado           = ENFRIAMIENTO;
        break;

    case ENFRIAMENTO:
        desmoldar();
        enfriar(VENTANA | VENTILADOR | HELADERA);
        estado = IDLE;
        break;

    default:
        estado = BUSCAR_INGREDIENTES;
        break;
    }
};

while (1) {

    // if (obtener_estado() == IDLE)
    //     establecer_estado(BUSCAR_INGREDIENTES);
    fsm_run(fsm_obtener_estado());
}
```