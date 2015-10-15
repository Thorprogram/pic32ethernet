/* 
 * File:   newmain.c
 * Author: Michael
 *
 * Created on 12 July 2015, 22:42
 */


#include <stdio.h>
#include <stdlib.h>
#include <xc.h>

#include "myconfig.h"
#include "plib.h"


/* Core configuration fuse settings */
#pragma config FPLLMUL = MUL_20, FPLLIDIV = DIV_2, FPLLODIV = DIV_1, FWDTEN = OFF
#pragma config POSCMOD = HS, FNOSC = PRIPLL, FPBDIV = DIV_2
#pragma config CP = OFF, BWP = OFF, PWP = OFF

/* Ethernet MAC */
#pragma config FMIIEN = OFF             // Ethernet RMII/MII Enable (RMII Enabled)
#pragma config FETHIO = OFF             // Ethernet I/O Pin Select (Alternate I/O pins)

#pragma config UPLLEN = OFF
#pragma config FSRSSEL = PRIORITY_7


#define RED_ON          PORTDbits.RD0 = 1;
#define ORANGE_ON       PORTDbits.RD1 = 1;
#define GREEN_ON        PORTDbits.RD2 = 1;

#define RED_OFF         PORTDbits.RD0 = 0;
#define ORANGE_OFF      PORTDbits.RD1 = 0;
#define GREEN_OFF       PORTDbits.RD2 = 0;


int main(int argc, char** argv) {

   eEthRes enEth_Hardware_opresult = 0;
   unsigned short phyreg_result = 0;
   UINT32 uart_data_rate;
   long count = 0;
   int i;
   unsigned char hostAddress[6]= { 0x00, 0x04, 0xa3, 0x00, 0x00, 0x02};
   UINT32 byte1;
   unsigned short octet6, octet5, octet4, octet3, octet2, octet1;
   
       __XC_UART = 1;

       // TRISD = TRISD & ~(0x07); 
       // PORTD = PORTD & ~(0x07);
        
       TRISD = TRISD & ~(0x0907);
       TRISD = TRISD & ~(0x0907);
               
       
        UARTConfigure(UART1, UART_ENABLE_PINS_TX_RX_ONLY);
        uart_data_rate = UARTSetDataRate(UART1, 40000000, UART1_DATA_RATE);
        UARTSetLineControl(UART1, UART_DATA_SIZE_8_BITS|UART_PARITY_NONE|UART_STOP_BITS_1);
        UARTEnable(UART1, UART1|UART_ENABLE|UART_TX|UART_RX);
        U1MODE = 0x8000; /* UARTEnable is not enabling the port */
        
      //  while(1)
      //  {
            if (UARTTransmitterIsReady(UART1))
                {
                    GREEN_ON
                    RED_OFF
           //         UARTSendDataByte(UART1, sizeof(short));
                    printf("UART OK \r\n");
                }
            else {RED_ON 
                    GREEN_OFF }
            
                for (count=0; count<=40000; count++)
                    { ORANGE_ON}
                 for (count=0; count<=40000; count++)
                 { ORANGE_OFF }
            
      //  }
        GREEN_OFF
        
   /* Sequence follows Microchips recommendations in PIC32 Family reference
    * Section (35) 35.4.10 Ethernet Initialization
    */
         
   /* (1) Ethernet Initialization (disable interrupts, clears other activity
    * 
    * eEthRes EthClose(eEthCloseFlags cFlags) follows steps (1a) to (1e)
    * disables interrupts on entry, must enable later. 
    * exit. The function also resets the MAC controller with inline function 
    * _EthMacReset() */
   enEth_Hardware_opresult = EthClose(ETH_CLOSE_GRACEFUL);
   printf("1. EthClose: %d \n\r", enEth_Hardware_opresult);
   
   if (enEth_Hardware_opresult != ETH_RES_OK)
   {
       /* will never get here as EthcLose() will either hang waiting 
        * for something to finish or always returns an OK result
        */        
   }
   enEth_Hardware_opresult = 0;
   /* Clear the address of the RX and TX descriptors start addresses as
    * EthClose() does not do this
    */
   EthConfigTxStAddress(0);
   EthConfigRxStAddress(0);
   
   /* (2) MAC Initialization, resets the MAC, sets up the I/O pins as
    * inputs or outputs and configure analog shared pins
    * #pragma steps are completed above for FETHIO and FMIIEN
    */
 
   /* Clears only the bits set here, all other bits remain untouched by the 
    * EMACxCFG1CLR. 
    */
    EMACxCFG1CLR = (ETH_MAC_CONFIG1_RESET_TFUN|ETH_MAC_CONFIG1_RESET_TMCS
           |ETH_MAC_CONFIG1_RESET_RFUN|ETH_MAC_CONFIG1_RESET_RMCS);

    
   /* Configure ports and others. PIC32 Ethernet kit II uses
    * the alternative RMII setting. The Ports used by the Ethernet MAC are
    * PORTA, PORTD, PORTE, PORTG. The three leds on the kit are also on 
    * PORTD
    */
   
    /* Set the port direction bits for the LEDs (D0,D1,D2). Later when the 
     * Ethernet controller is enabled is will override and set the remaining 
     * pins that it requires.
     */
    //TRISD = 0x0007;
          
    /* A number of the pins used by the Ethernet controller are also external
     * interrupt sources. Make sure these INT are disabled.
     */
        
   /* Configure the PHY 8740a chip */
    EthMIIMConfig ( 80000000, 2500000 );
   
    enEth_Hardware_opresult = EthInit ();
    printf("2. EthInit: %d \n\r", enEth_Hardware_opresult);
    
     
       GREEN_ON
    /* Configure and open the MAC controller, MAC and PHY need to have consistent configurations*/
       for (i=0; i<32; i++)
       {
         while(EthMIIMBusy());   // wait device not busy
         EthMIIMReadStart(i, 0x00);
         
         while (EthMIIMBusy());
         phyreg_result = EthMIIMReadResult();
         
         printf("Reg: %d ", i);
         printf("Data: %d\n\r",phyreg_result);

       }

        /* Need to send a soft reset to PHY
         * Send 0x00 to phy register 0 
         *  ?? these values appear to have been 
         * set by the circuit mode[1,1,1]... nothing 
         * to really done here.. will use the results of the 
         * register queries above.
         */
        
        /* MAC Configuration 
         * MAC Open and MAC Config operate on the EMAC1CFG1 and CFG2 registers, 
         * MAC Config also operates on EMAC1SUPP registers.
         * MAC Config can presumably be used to configure other bits after a MAC
         * open
         * MAC Config SET and CLR are other variants of MAC Config for clearing
         * and setting optional bits.
         * 
         * void EthMACOpen ( eEthOpenFlags oFlags, eEthMacPauseType pauseType );
         * Configure other flags: 
         * void EthMACConfig ( eEthMACConfig1Flags c1Flags, eEthMACConfig2Flags 
         *                        c2Flags,      eEthMACConfigSuppFlags sFlags );
         * 
         * The configuration should match the PHY
         */
        
        EthMACOpen ( ETH_OPEN_AUTO|ETH_OPEN_FDUPLEX|ETH_OPEN_100|ETH_OPEN_HUGE_PKTS, 
                        ETH_MAC_PAUSE_TYPE_PAUSE|ETH_MAC_PAUSE_TYPE_EN_TX|ETH_MAC_PAUSE_TYPE_EN_RX );
         
        /* CRC Configuration
         * Need to set the desired auto-padding and CRC in config reg 2. Can't be
         * done using the MAC open, probably use MAC REG SET ....
         *
         */    
                
         /* Progam the EMAC1IPGT with back to back inter packet gap
          * void EthMACConfigB2BIpGap ( unsigned int ipGap );
          * 0x15 recommended for 100Mbps = .96us
          */       
            
        EthMACConfigB2BIpGap (0x15);
        
        /* Set the non back to back IPGR
         * void EthMACConfigNB2BIpGap ( unsigned int ipgr1, unsigned int ipgr2 );
         * Recommended ipgr1 = 0xC
         * Recommended ipgr2 = 0x12
         * 
         */
        EthMACConfigNB2BIpGap ( 0xC, 0x12 );
        
        /* Set the maximum frame length
         * This is apparently set by the EthInit() to the 
         * default value of 0x600. Going to set it here
         * to be sure.. will check debug later.
         */
        EthMACSetMaxFrame (0x600);

         /* Set the station MAC address registers
          * (these are loaded at reset from the factory 
          * pre-programmed station address.
          * 
          * Will do a get first and see what is stored here using
          * void EthMACGetAddress ( unsigned char bAddress[6] );

          */   
        
        EthMACGetAddress(hostAddress);
        
        printf("Octet 1: %x\n", hostAddress[0]);
        printf("Octet 2: %x\n", hostAddress[1]);
        printf("Octet 3: %x\n", hostAddress[2]);
        printf("Octet 4: %x\n", hostAddress[3]);
        printf("Octet 5: %x\n", hostAddress[4]);
        printf("Octet 6: %x\n", hostAddress[5]);
        
        ORANGE_ON
        
    return (EXIT_SUCCESS);
}

