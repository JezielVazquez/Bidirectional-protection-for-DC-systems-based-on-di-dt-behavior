//---------------------------------------------------------------------------//
//--------------------------- Archivos de cabecera --------------------------//
//---------------------------------------------------------------------------//
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <file.h>
#include "F28x_Project.h"
#include "sci_io.h"
//---------------------------------------------------------------------------//

// Conexiones
// Entrada No inversora del comparador es AA2 --> GPIO 29
// Entrada Inversora del comparador es Interna del DAC
// Salida del COMPH es GPIO 60
// Conectar la salida del comparador a la entrada de la interrupción
// Conectar GPIO 60 a GPIO 111

//---------------------------------------------------------------------------//
//----------------------- Funciones de interrupciones -----------------------//
//---------------------------------------------------------------------------//
interrupt void sciaTxFifoIsr(void);
interrupt void sciaRxFifoIsr(void);
interrupt void cpu_timer0_isr(void);
interrupt void xint3_isr(void);
void scia_init(void);
void scia_fifo_init(void);
void scia_xmit(int a);
void scia_msg(char *msg);
void InitCMPSS(void);
//---------------------------------------------------------------------------//

Uint16 SendChar;
Uint16 ReceivedChar;
//Uint16 Cuenta = 200;                          // Cuenta del temporizador del micro-corto
Uint16 Cuenta = 200;                          // Cuenta del temporizador del micro-corto
Uint16 Cuenta2 = 1000;                          // Cuenta del temporizador del micro-corto
Uint16 Banderaa2 = 0;                          // Cuenta del temporizador del micro-corto
Uint16 Banderaa3 = 0;                          // Cuenta del temporizador del micro-corto
Uint16 Banderaa4 = 0;                          // Cuenta del temporizador del micro-corto
Uint16 Banderab2 = 0;                          // Cuenta del temporizador del micro-corto
Uint16 Banderab3 = 0;                          // Cuenta del temporizador del micro-corto
Uint16 Banderab4 = 0;                          // Cuenta del temporizador del micro-corto
Uint16 Banderab5 = 0;                          // Cuenta del temporizador del micro-corto
Uint16 Banderac2 = 0;                          // Cuenta del temporizador del micro-corto
Uint16 Banderac3 = 0;                          // Cuenta del temporizador del micro-corto
Uint16 Banderac4 = 0;                          // Cuenta del temporizador del micro-corto
//---------------------------------------------------------------------------//
//  5   da 190  ns
//  10  da 230  ns
//  15  da 270  ns
//  20  da 270  ns
//  25  da 320  ns
//  30  da 320  ns
//  35  da 360  ns
//  45  da 400  ns
//  55  da 480  ns
//  65  da 520  ns
//  75  da 560  ns
//  85  da 620  ns
//  95  da 650  ns
//  105 da 710  ns
//  200 da 1180 ns
//  400 da 1180 ns
//  600 da 1180 ns
//  800 da 1180 ns
//---------------------------------------------------------------------------//
void main(void){
InitSysCtrl();                              // Iniciar el sistema de control PLL, WatchDog y habilitar el reloj.
//---------------------------------------------------------------------------//
//--------------- En esta sección declarar entradas y salidas ---------------//
//---------------------------------------------------------------------------//
InitGpio();
EALLOW;                                     //Permite escribir en los registros protegidos
//--------------------------------- Carga 2 ---------------------------------//
GpioCtrlRegs.GPBDIR.bit.GPIO32 = 1;         // 1 salida   0 entrada
//--------------------------------- Carga 1 ---------------------------------//
GpioCtrlRegs.GPADIR.bit.GPIO19 = 1;         // 1 salida   0 entrada
//--------------------------------- Corto C ---------------------------------//
GpioCtrlRegs.GPADIR.bit.GPIO18 = 1;         // 1 salida   0 entrada
//--------------------------------- Breaker ---------------------------------//
GpioCtrlRegs.GPCDIR.bit.GPIO67 = 1;         // 1 salida   0 entrada
//--------------------------------- Con Bus ---------------------------------//
GpioCtrlRegs.GPADIR.bit.GPIO22 = 1;         // 1 salida   0 entrada
//--------------------------------- Des Cin ---------------------------------//
GpioCtrlRegs.GPDDIR.bit.GPIO105 = 1;         // 1 salida   0 entrada
//--------------------------------- Carga C ---------------------------------//
GpioCtrlRegs.GPDDIR.bit.GPIO104 = 1;         // 1 salida   0 entrada


//GpioCtrlRegs.GPDMUX1.bit.GPIO111 = 0;       //GPIO
GpioCtrlRegs.GPDDIR.bit.GPIO111 = 0;        // Entrada
GpioCtrlRegs.GPDPUD.bit.GPIO111 = 1;        // 0 Habilita 1 deshabilita pull-up
GpioCtrlRegs.GPDQSEL1.bit.GPIO111 = 3;      // Asincrono
GpioCtrlRegs.GPDCTRL.bit.QUALPRD1 = 0;      // Selecciona la prescala


EDIS;                                       //Ya no permite escribir en los registros protegidos
//---------------------------------------------------------------------------//


//--------------------------------- Carga 2 ---------------------------------//
GpioDataRegs.GPBDAT.bit.GPIO32 = 0;         // Manda a bajo la salida
//--------------------------------- Carga 1 ---------------------------------//
GpioDataRegs.GPADAT.bit.GPIO19 = 0;         // Manda a bajo la salida
//--------------------------------- Corto C ---------------------------------//
GpioDataRegs.GPADAT.bit.GPIO18 = 0;         // Manda a bajo la salida
//--------------------------------- Breaker ---------------------------------//
GpioDataRegs.GPCDAT.bit.GPIO67 = 0;         // Manda a bajo la salida
//--------------------------------- Con Bus ---------------------------------//
GpioDataRegs.GPADAT.bit.GPIO22 = 0;         // Manda a bajo la salida
//--------------------------------- Des Cin ---------------------------------//
GpioDataRegs.GPDDAT.bit.GPIO105 = 0;         // Manda a bajo la salida
//--------------------------------- Carga C ---------------------------------//
GpioDataRegs.GPDDAT.bit.GPIO104 = 0;         // Manda a bajo la salida


//---------------------------- Configuración de Rx --------------------------//
GPIO_SetupPinMux(43, GPIO_MUX_CPU1, 15);
GPIO_SetupPinOptions(43, GPIO_INPUT, GPIO_PUSHPULL);
//---------------------------- Configuración de Tx --------------------------//
GPIO_SetupPinMux(42, GPIO_MUX_CPU1, 15);
GPIO_SetupPinOptions(42, GPIO_OUTPUT, GPIO_ASYNC);
//----------------- Configuración de la salida del comparador ---------------//
GPIO_SetupPinMux(60 , GPIO_MUX_CPU1, 5);
//---------------------------------------------------------------------------//
//---------------- Configuración del pin de interrupción XINT3 --------------//
GPIO_SetupXINT3Gpio(111);
XintRegs.XINT3CR.bit.POLARITY = 1;          // 0 y 2flanco de bajada   1 de subida   3 ambos
XintRegs.XINT3CR.bit.ENABLE = 1;            // 1 habilita la interrupción 0 la deshabilita XINT3



//---------------------------------------------------------------------------//
//------------- Esta sección inicializa el control de registros -------------//
//---------------------------------------------------------------------------//
InitPieCtrl();                          // Inicia el control de los registros PIE
IER = 0x0000;                           // Deshabilita las interrupciones
IFR = 0x0000;                           // Limpia las banderas
InitPieVectTable();                     // Inicializa la tabla de vectores para las ISR
EALLOW;                                 // Permite escribir en los registros protegidos
//---------------------------------------------------------------------------//
//------------ Esta sección habilita los vectores de interrupción -----------//
//---------------------------------------------------------------------------//
PieCtrlRegs.PIECTRL.bit.ENPIE = 1;      // Habilita el bloque PIE
PieVectTable.SCIA_RX_INT = &sciaRxFifoIsr;
PieVectTable.SCIA_TX_INT = &sciaTxFifoIsr;
PieVectTable.TIMER0_INT = &cpu_timer0_isr;
PieVectTable.XINT3_INT = &xint3_isr;    // Define el vector de interrupción ISR
PieCtrlRegs.PIEIER9.bit.INTx1 = 1;      // Conecta la col 1 donde esta SCIA_RX
PieCtrlRegs.PIEIER9.bit.INTx2 = 1;      // Conecta la col 2 donde esta SCIA_TX
PieCtrlRegs.PIEIER12.bit.INTx1 = 1;     // Conecta la col 1 donde esta XINT3
PieCtrlRegs.PIEIER1.bit.INTx7 = 1;      // Conecta la col 7 donde esta Timer0
CpuTimer0Regs.TCR.bit.TIE = 1;          // Habilita el Timer0
EDIS;    // This is needed to disable write to EALLOW protected registers
EnableInterrupts();
ERTM;          // Enable Global realtime interrupt DBGM
scia_fifo_init();       // Initialize the SCI FIFO
scia_init();  // Initialize SCI for digital loop back

IER = 0x100;                            // Enable CPU INT
//IER |= M_INT9;                        // Enable CPU INT9      //TX Y RX
IER |= M_INT1;                          // Habilita la interrupción del grupo 1 donde esta Timer0
IER |= M_INT12;                         // Habilita la interrupción del grupo 12 donde esta XINT3
EINT;                                   // Habilita las interrupciones globales INTM

//---------------------------------------------------------------------------//
CpuTimer0Regs.PRD.all  = 100000000;     // Cuenta inicial

InitCMPSS();

while (1){
    if (Cmpss1Regs.COMPSTS.bit.COMPHSTS){   // Limpia el enganche si ya no esta el corto
        EALLOW;
        DELAY_US(1000 * 5000);
        Cmpss1Regs.COMPSTSCLR.bit.HLATCHCLR = 1;        // Genera un pulso y limpia el enganche
        EDIS;
    }

    if (ReceivedChar != SendChar){
        SendChar = ReceivedChar;

//---------------------------------     SW 4    ---------------------------------//
        if(SendChar == 66){                     //  B
            GpioDataRegs.GPADAT.bit.GPIO19 = 0;     // Manda a bajo la salida
        }
        if(SendChar == 67){                     //  C
            GpioDataRegs.GPADAT.bit.GPIO19 = 1;     // Manda a alto la salida
        }
//---------------------------------     SW 3    ---------------------------------//
        if(SendChar == 68){                     //  D
            GpioDataRegs.GPADAT.bit.GPIO18 = 0;     // Manda a bajo la salida
        }
        if(SendChar == 69){                     //  E
            GpioDataRegs.GPADAT.bit.GPIO18 = 1;     // Manda a alto la salida
            CpuTimer0Regs.TCR.bit.TSS = 0;          // Reinicia el timer
            CpuTimer0Regs.TIM.all  = Cuenta;        // Reinicia la cuenta
            PieCtrlRegs.PIEACK.bit.ACK1 = 1;                // Habilita la interrupción del timer
           // GpioDataRegs.GPADAT.bit.GPIO19 = 1;     // Turn off LD2
            }
//---------------------------------     SW 2    ---------------------------------//
        if(SendChar == 70){                     //  F
            GpioDataRegs.GPADAT.bit.GPIO22 = 0;     // Manda a bajo la salida
        }
        if(SendChar == 71){                     //  G
            GpioDataRegs.GPADAT.bit.GPIO22 = 1;     // Manda a alto la salida
        }
//---------------------------------     SW 1    ---------------------------------//
        if(SendChar == 72){                     //  H
            GpioDataRegs.GPCDAT.bit.GPIO67 = 0;     // Manda a bajo la salida
        }
        if(SendChar == 73){                     //  I
            GpioDataRegs.GPCDAT.bit.GPIO67 = 1;     // Manda a alto la salida
        }
//---------------------------------     SW 5    ---------------------------------//
        if(SendChar == 74){                     //  J
            GpioDataRegs.GPBDAT.bit.GPIO32 = 0;     // Manda a bajo la salida
        }
        if(SendChar == 75){                     //  K
            GpioDataRegs.GPBDAT.bit.GPIO32 = 1;     // Manda a alto la salida
        }
//---------------------------------     SW 6    ---------------------------------//
        if(SendChar == 76){                     //  L
            GpioDataRegs.GPDDAT.bit.GPIO105 = 0;     // Manda a bajo la salida
        }
        if(SendChar == 77){                     //  M
            GpioDataRegs.GPDDAT.bit.GPIO105 = 1;     // Manda a alto la salida
        }
//---------------------------------     SW 7    ---------------------------------//
        if(SendChar == 78){                     //  N
            GpioDataRegs.GPDDAT.bit.GPIO104 = 0;     // Manda a bajo la salida
        }
        if(SendChar == 79){                     //  O
            GpioDataRegs.GPDDAT.bit.GPIO104 = 1;     // Manda a alto la salida
        }
//---------------------------------     SW PRUEBA A    ---------------------------------//
        if(SendChar == 89){                     //  Y
            GpioDataRegs.GPDDAT.bit.GPIO104 = 0;     // Manda a bajo la salida
        }
        if(SendChar == 90){                     //  Z
            GpioDataRegs.GPDDAT.bit.GPIO104 = 1;     // Manda a alto la salida
            GpioDataRegs.GPDDAT.bit.GPIO104 = 1;     // Manda a alto la salida
            GpioDataRegs.GPDDAT.bit.GPIO104 = 1;     // Manda a alto la salida
            GpioDataRegs.GPDDAT.bit.GPIO104 = 1;     // Manda a alto la salida
            CpuTimer0Regs.TCR.bit.TSS = 0;          // Reinicia el timer
            CpuTimer0Regs.TIM.all  = Cuenta2;        // Reinicia la cuenta
            CpuTimer0Regs.TIM.all  = Cuenta2;        // Reinicia la cuenta
            CpuTimer0Regs.TIM.all  = Cuenta2;        // Reinicia la cuenta
            CpuTimer0Regs.TIM.all  = Cuenta2;        // Reinicia la cuenta
            PieCtrlRegs.PIEACK.bit.ACK1 = 1;                // Habilita la interrupción del timer
            Banderaa2 = 1;
        }
//---------------------------------     SW PRUEBA B    ---------------------------------//
        if(SendChar == 80){                     //  P
            GpioDataRegs.GPDDAT.bit.GPIO104 = 0;     // Manda a bajo la salida
        }
        if(SendChar == 81){                     //  Q
            GpioDataRegs.GPADAT.bit.GPIO18 = 1;     // Manda a alto la salida
            CpuTimer0Regs.TCR.bit.TSS = 0;          // Reinicia el timer
            CpuTimer0Regs.TIM.all  = Cuenta2;        // Reinicia la cuenta
            CpuTimer0Regs.TIM.all  = Cuenta2;        // Reinicia la cuenta
            CpuTimer0Regs.TIM.all  = Cuenta2;        // Reinicia la cuenta
            CpuTimer0Regs.TIM.all  = Cuenta2;        // Reinicia la cuenta
            PieCtrlRegs.PIEACK.bit.ACK1 = 1;                // Habilita la interrupción del timer
            Banderab2 = 1;
        }
//---------------------------------     SW PRUEBA C    ---------------------------------//
        if(SendChar == 82){                     //  R
            GpioDataRegs.GPDDAT.bit.GPIO104 = 0;     // Manda a bajo la salida
        }
        if(SendChar == 83){                     //  S
            GpioDataRegs.GPDDAT.bit.GPIO104 = 1;     // Manda a bajo la salida
            CpuTimer0Regs.TCR.bit.TSS = 0;          // Reinicia el timer
            CpuTimer0Regs.TIM.all  = Cuenta2;        // Reinicia la cuenta
            CpuTimer0Regs.TIM.all  = Cuenta2;        // Reinicia la cuenta
            CpuTimer0Regs.TIM.all  = Cuenta2;        // Reinicia la cuenta
            CpuTimer0Regs.TIM.all  = Cuenta2;        // Reinicia la cuenta
            PieCtrlRegs.PIEACK.bit.ACK1 = 1;                // Habilita la interrupción del timer
            Banderac2 = 1;
        }
    }
}

}

void scia_init(){
    SciaRegs.SCICCR.all = 0x0007;               // 8 Bits
    SciaRegs.SCICTL1.all = 0x0003;              // Tx y Rx  habilitados
    SciaRegs.SCICTL2.all = 0x0003;              // Habilita la interrupción Tx y Rx
    SciaRegs.SCICTL2.bit.TXINTENA = 1;
    SciaRegs.SCICTL2.bit.RXBKINTENA = 1;
/////////////////  Definir a 115200 bauds   /////////////
    SciaRegs.SCIHBAUD.all    =0x0000;
    SciaRegs.SCILBAUD.all    =53;
/////////////////////////////////////////////////////////
    SciaRegs.SCICCR.bit.LOOPBKENA = 0; // Conecta internamente Tx con RX   0--> Deshabilitado
/////////////////////////////////////////////////////////////////

    SciaRegs.SCIFFTX.all = 0xC021;  // FIFO Tx y Rx, Habilita FIFO para Tx y almacena 1 bits
    SciaRegs.SCIFFRX.all = 0x0021;  // Habilita Fifo para recepción y almacena 1 bits
    SciaRegs.SCIFFCT.all = 0x00;
//////////////////////////////////////////////////////////////////

    SciaRegs.SCICTL1.all = 0x0023;     // Relinquish SCI from Reset  sin paridad, 1 bit stop

////////////////////////////////////////////////////////////////
    SciaRegs.SCIFFTX.bit.TXFIFORESET = 1;
    SciaRegs.SCIFFRX.bit.RXFIFORESET = 1;
///////////////////////////////////////////////////////////////
}

void scia_xmit(int a){
    while (SciaRegs.SCIFFTX.bit.TXFFST != 0) {}
    SciaRegs.SCITXBUF.all=a;
}

void scia_msg(char * msg){
    int i;
    i = 0;
    while(msg[i] != '\0')    {
        scia_xmit(msg[i]);
        i++;
    }
}

void scia_fifo_init(){
    SciaRegs.SCIFFTX.all = 0xE040;
    SciaRegs.SCIFFRX.all = 0x2044;
    SciaRegs.SCIFFCT.all = 0x0;
}

interrupt void sciaTxFifoIsr(void){
    scia_xmit(SendChar);

////    /*Se intercambiaron la limpia de interrupciones de Rx y TX*/  //////
    SciaRegs.SCIFFRX.bit.RXFFOVRCLR=1;   // Clear Overflow flag
    SciaRegs.SCIFFRX.bit.RXFFINTCLR=1;   // Clear Interrupt flag
    PieCtrlRegs.PIEACK.all|=0x100;       // Issue PIE ACK
}

interrupt void sciaRxFifoIsr(void){
    ReceivedChar = SciaRegs.SCIRXBUF.all;

////    /*Se intercambiaron la limpia de interrupciones de Rx y TX*/  //////
    SciaRegs.SCIFFTX.bit.TXFFINTCLR=1;   // Clear SCI Interrupt flag
    PieCtrlRegs.PIEACK.all|=0x100;       // Issue PIE ack
}


interrupt void cpu_timer0_isr(void)
{
   if (Banderaa2){
       if (Banderaa3){
           if (Banderaa4){
               GpioDataRegs.GPBDAT.bit.GPIO32 = 0;         // Manda a bajo la salida
               CpuTimer0Regs.TCR.bit.TSS = 1;                   // Detiene el timer
               CpuTimer0Regs.TIM.all  = Cuenta2;        // Reinicia la cuenta
               //PieCtrlRegs.PIEACK.bit.ACK1 = 1;                // Habilita la interrupción del timer
               Banderaa4 = 0;
               Banderaa3 = 0;
               Banderaa2 = 0;
           }else{
               GpioDataRegs.GPBDAT.bit.GPIO32 = 1;         // Manda a alto la salida
               CpuTimer0Regs.TCR.bit.TSS = 0;          // Reinicia el timer
               CpuTimer0Regs.TIM.all  = Cuenta2;        // Reinicia la cuenta
               PieCtrlRegs.PIEACK.bit.ACK1 = 1;                // Habilita la interrupción del timer
               Banderaa4 = 1;
           }
       }else{
           //PieCtrlRegs.PIEACK.bit.ACK12 = 1;                // Habilita la interrupción del boton
           GpioDataRegs.GPDDAT.bit.GPIO104 = 0;             // Manda a bajo la salida
           CpuTimer0Regs.TCR.bit.TSS = 0;          // Reinicia el timer
           CpuTimer0Regs.TIM.all  = Cuenta2;                 // Reinicia la cuenta
           PieCtrlRegs.PIEACK.bit.ACK1 = 1;                // Habilita la interrupción del timer
           Banderaa3 = 1;
       }
   }
   else{
       if (Banderab2){
          if (Banderab3){
              if (Banderab4){
                  if (Banderab5){
                      GpioDataRegs.GPBDAT.bit.GPIO32 = 0;         // Manda a bajo la salida
                      CpuTimer0Regs.TCR.bit.TSS = 1;                   // Detiene el timer
                      CpuTimer0Regs.TIM.all  = Cuenta2;        // Reinicia la cuenta
                      //PieCtrlRegs.PIEACK.bit.ACK1 = 1;                // Habilita la interrupción del timer
                      Banderab5 = 0;
                      Banderab4 = 0;
                      Banderab3 = 0;
                      Banderab2 = 0;
                  }else{
                      GpioDataRegs.GPBDAT.bit.GPIO32 = 1;         // Manda a alto la salida
                      CpuTimer0Regs.TCR.bit.TSS = 0;          // Reinicia el timer
                      CpuTimer0Regs.TIM.all  = Cuenta2;        // Reinicia la cuenta
                      PieCtrlRegs.PIEACK.bit.ACK1 = 1;                // Habilita la interrupción del timer
                      Banderab5 = 1;
                  }
              }else{
                  GpioDataRegs.GPDDAT.bit.GPIO104 = 0;     // Manda a bajo la salida
                  GpioDataRegs.GPADAT.bit.GPIO18 = 0;     // Manda a bajo la salida
                  CpuTimer0Regs.TCR.bit.TSS = 0;          // Reinicia el timer
                  CpuTimer0Regs.TIM.all  = Cuenta2;        // Reinicia la cuenta
                  PieCtrlRegs.PIEACK.bit.ACK1 = 1;                // Habilita la interrupción del timer
                  Banderab4 = 1;
              }
          }else{
              //PieCtrlRegs.PIEACK.bit.ACK12 = 1;                // Habilita la interrupción del boton
              GpioDataRegs.GPDDAT.bit.GPIO104 = 1;     // Manda a alto la salida
              CpuTimer0Regs.TCR.bit.TSS = 0;          // Reinicia el timer
              CpuTimer0Regs.TIM.all  = Cuenta2;                 // Reinicia la cuenta
              PieCtrlRegs.PIEACK.bit.ACK1 = 1;                // Habilita la interrupción del timer
              Banderab3 = 1;
          }
      }else{
          if (Banderac2){
             if (Banderac3){
                 if (Banderac4){
                     GpioDataRegs.GPBDAT.bit.GPIO32 = 0;         // Manda a bajo la salida
                     CpuTimer0Regs.TCR.bit.TSS = 1;                   // Detiene el timer
                     CpuTimer0Regs.TIM.all  = Cuenta2;        // Reinicia la cuenta
                     //PieCtrlRegs.PIEACK.bit.ACK1 = 1;                // Habilita la interrupción del timer
                     Banderac4 = 0;
                     Banderac3 = 0;
                     Banderac2 = 0;
                 }else{
                     GpioDataRegs.GPBDAT.bit.GPIO32 = 1;         // Manda a alto la salida
                     CpuTimer0Regs.TCR.bit.TSS = 0;          // Reinicia el timer
                     CpuTimer0Regs.TIM.all  = Cuenta2;        // Reinicia la cuenta
                     PieCtrlRegs.PIEACK.bit.ACK1 = 1;                // Habilita la interrupción del timer
                     Banderac4 = 1;
                 }
             }else{
                 //PieCtrlRegs.PIEACK.bit.ACK12 = 1;                // Habilita la interrupción del boton
                 GpioDataRegs.GPDDAT.bit.GPIO104 = 0;             // Manda a bajo la salida
                 CpuTimer0Regs.TCR.bit.TSS = 0;          // Reinicia el timer
                 CpuTimer0Regs.TIM.all  = Cuenta2;                 // Reinicia la cuenta
                 PieCtrlRegs.PIEACK.bit.ACK1 = 1;                // Habilita la interrupción del timer
                 Banderac3 = 1;
             }
         }else{
           PieCtrlRegs.PIEACK.bit.ACK12 = 1;                // Habilita la interrupción del boton
           GpioDataRegs.GPADAT.bit.GPIO18 = 0;              // Apaga el Corto
           CpuTimer0Regs.TCR.bit.TSS = 1;                   // Detiene el timer
           CpuTimer0Regs.TIM.all  = Cuenta;                 // Reinicia la cuenta
         }

      }
   }
}

void InitCMPSS(void){
    EALLOW;
// *******    Habilita el comparador y el DAC    *******
    Cmpss1Regs.COMPCTL.bit.COMPDACE = 1;
// *******    Selección de la entrada inversora   0 --> DAC   1 --> Pin externo   *******
    Cmpss1Regs.COMPCTL.bit.COMPHSOURCE = 0;


// *******    Habilita la ruta asíncrona del comparador   0 --> No    1 --> si   *******
   Cmpss1Regs.COMPCTL.bit.ASYNCHEN = 1;


// *******    Selección del voltaje de referencia del DAC   0 --> VDDA   1 --> VDAC   *******
    Cmpss1Regs.COMPDACCTL.bit.SELREF = 0;
// *******    Selección de actualización de DACxVALA    0 --> SYSCLK   1 --> EPWMSYNCPER   *******
    Cmpss1Regs.COMPDACCTL.bit.SWLOADSEL = 0;
// *******    Selección de la fuente del DAC     0 --> DACHVALS   1 --> GENERADOR DE RAMPA   *******
    Cmpss1Regs.COMPDACCTL.bit.DACSOURCE = 0;
// *******    Valor a cargar en DACHVALA  12 bits   *******
    Cmpss1Regs.DACHVALS.bit.DACVAL = 2048;
// *******    Selección de la salida de CTRIPH   0 --> Asincrona   1 --> Sincrona    *******
// *******    Selección de la salida de CTRIPH   2 --> Filtro Dig  3 --> Enganchada  *******
    Cmpss1Regs.COMPCTL.bit.CTRIPHSEL = 0;
// *******    Selección de la salida de CTRIOUTPH   0 --> Asincrona   1 --> Sincrona    *******
// *******    Selección de la salida de CTRIPOUTH   2 --> Filtro Dig  3 --> Enganchada  *******
    Cmpss1Regs.COMPCTL.bit.CTRIPOUTHSEL = 3;
//  OUTPUT X-BAR 3
    /* Tabla 9.2
     * Mux0
     *          0   CMPSS1.CTRIPH
     *          1   CMPSS1.CTRIPH:OR:CTRIPL
     *          2   ADCAEVT1
     *          3   ECAP1 OUT*/
    //OutputXbarRegs.OUTPUT1MUX0TO15CFG.bit.MUX0 = 0;
    OutputXbarRegs.OUTPUT3MUX0TO15CFG.bit.MUX0 = 0;
// Habilitar el MUX para la salida 1 X-BAR
    //OutputXbarRegs.OUTPUT1MUXENABLE.bit.MUX0 = 1;
    OutputXbarRegs.OUTPUT3MUXENABLE.bit.MUX0 = 1;
    EDIS;
}

interrupt void xint3_isr(void){
    XintRegs.XINT3CR.bit.ENABLE = 0;            // 1 habilita la interrupción 0 la deshabilita XINT3

    GpioDataRegs.GPCDAT.bit.GPIO67 = 0;         // Apaga el Breaker

    XintRegs.XINT3CR.bit.ENABLE = 1;            // 1 habilita la interrupción 0 la deshabilita XINT1
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;
}



