#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include <p32xxxx.h>
#include <plib.h>
#include <peripheral/osc.h>

#include <USB/usb.h>
#include "HardwareProfile.h"
#include "lcddriver.h"
#include <USB/usb_function_generic.h>
#include <lcddriver.h>

static void mainTask(void *pvParameters);
static void usbTask(void *pvParameters);
static void lcdTask(void *pvParameters);

int main()
{
    /// sysclock 40MHz from crystal
    OSCConfig( OSC_POSC_PLL, OSC_PLL_MULT_20, OSC_PLL_POST_2, 0 );
    PORTSetPinsDigitalOut(IOPORT_B,IOPORT_BIT_13);
    //PPSOutput(3,RPB13,REFCLKO);
    //OSCREFConfig(OSC_REFOCON_USBPLL,OSC_REFOCON_OE,4);
    
    //REFOCONbits.ROSEL = 0b0111;//0b0110;
    //REFOCONbits.OE = 1;
    //REFOCONbits.RODIV = 0x01;
    //REFOTRIMbits.ROTRIM = 0;
    //REFOCONbits.ON = 1; // active reference oscillator
    mOSCSetPBDIV(OSC_PB_DIV_4);
    INTEnableSystemMultiVectoredInt();
    mPORTAClearBits(BIT_10);
    mPORTASetPinsDigitalOut(BIT_10);
    TRISBbits.TRISB15 = 0;
    LATBbits.LATB15 = 1;
    portDISABLE_INTERRUPTS();
    xTaskCreate(usbTask,"T2",1000,NULL,3,NULL);
    xTaskCreate(lcdTask,"T3",configMINIMAL_STACK_SIZE,NULL,3,NULL);
    xTaskCreate(mainTask,"T1",configMINIMAL_STACK_SIZE,NULL,4,NULL); /// Highest priority
    vTaskStartScheduler();
}

static void mainTask(void* pvParameters)
{
    TRISBbits.TRISB15 = 0;
    //TRISAbits.TRISA10 = 0;
    while(1)
    {
        LATBbits.LATB15 ^= 1;
        PORTToggleBits(IOPORT_B,IOPORT_BIT_13);
        //LATAbits.LATA10 ^= 1;
        vTaskDelay(100);
    }
}


static void lcdTask(void* pvParameters)
{
    LCD_Init();
    TRISAbits.TRISA10 = 0;
    char* mess  = "A:10.223V 3.332A";
    char* mess2 = "T:10.223V 1.332A";
    LCD_PutStr(mess);
    LCD_GotoXY(2,1);
    while(*mess2!='\0')
         LCD_PutChar(*mess2++);
    char i;
    while(1)
    {
        LCD_GotoXY(1,1);
        LCD_PutChar(i++);
        LATAbits.LATA10 ^= 1;
        vTaskDelay(250);
    }
}

static void ProcessIO();
USB_HANDLE USBGenericOutHandle;
USB_HANDLE USBGenericInHandle;
unsigned char INPacket[USBGEN_EP_SIZE];		//User application buffer for sending IN packets to the host
unsigned char OUTPacket[USBGEN_EP_SIZE];	//User application buffer for receiving and holding OUT packets sent from the host
static void usbTask(void* pvParameters)
{
    #if defined(USE_USB_BUS_SENSE_IO)
    tris_usb_bus_sense = INPUT_PIN; // See HardwareProfile.h
    #endif
    USBGenericOutHandle = 0;  //USB handle.  Must be initialized to 0 at startup.
    USBGenericInHandle = 0;   //USB handle.  Must be initialized to 0 at startup.
    USBDeviceInit();
    while(1)
    {
        USBDeviceTasks();
        ProcessIO();
        vTaskDelay(1);
    }
}

void ProcessIO(void)
{
    //Blink the LEDs according to the USB device status, but only do so if the PC application isn't connected and controlling the LEDs.
//    if(blinkStatusValid)
//    {
//        BlinkUSBStatus();
 //   }

    //User Application USB tasks below.
    //Note: The user application should not begin attempting to read/write over the USB
    //until after the device has been fully enumerated.  After the device is fully
    //enumerated, the USBDeviceState will be set to "CONFIGURED_STATE".
    if((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1)) return;

    //As the device completes the enumeration process, the USBCBInitEP() function will
    //get called.  In this function, we initialize the user application endpoints (in this
    //example code, the user application makes use of endpoint 1 IN and endpoint 1 OUT).
    //The USBGenRead() function call in the USBCBInitEP() function initializes endpoint 1 OUT
    //and "arms" it so that it can receive a packet of data from the host.  Once the endpoint
    //has been armed, the host can then send data to it (assuming some kind of application software
    //is running on the host, and the application software tries to send data to the USB device).

    //If the host sends a packet of data to the endpoint 1 OUT buffer, the hardware of the SIE will
    //automatically receive it and store the data at the memory location pointed to when we called
    //USBGenRead().  Additionally, the endpoint handle (in this case USBGenericOutHandle) will indicate
    //that the endpoint is no longer busy.  At this point, it is safe for this firmware to begin reading
    //from the endpoint buffer, and processing the data.  In this example, we have implemented a few very
    //simple commands.  For example, if the host sends a packet of data to the endpoint 1 OUT buffer, with the
    //first byte = 0x80, this is being used as a command to indicate that the firmware should "Toggle LED(s)".
    if(!USBHandleBusy(USBGenericOutHandle))		//Check if the endpoint has received any data from the host.
    {
        switch(OUTPacket[0])					//Data arrived, check what kind of command might be in the packet of data.
        {
            case 0x00:
                //if(RED_LED == GREEN_LED)
                {
                    RED_LED ^= 1;
                    //GREEN_LED ^= 1;
                }
                //else
                {
                    //RED_LED = 1;
                    //GREEN_LED =  1;
                }
                INPacket[0] = 0x00;
                INPacket[1] = 0x0B;
                INPacket[2] = 0x90;
                INPacket[3] = 0x50;
                INPacket[4] = 0x10;
                INPacket[5] = 0x20;
                INPacket[6] = 0x10;
                INPacket[7] = 0x10;
                INPacket[8] = 0x23;
                INPacket[9] = 0x23;
                INPacket[10] = 0x7D;
                USBGenericInHandle = USBGenWrite(USBGEN_EP_NUM,(BYTE*)&INPacket,USBGEN_EP_SIZE);
                break;
            case 0x40:
                RED_LED^=1;
                INPacket[0] = 0x00;
                INPacket[1] = 0x16;
                INPacket[2] = 0x40;
                INPacket[3] = 0x01;
                INPacket[4] = 0x02;
                INPacket[5] = 0x04;
                INPacket[6] = 0x08;
                INPacket[7] = 0x10;
                INPacket[8] = 0x20;
                INPacket[9] = 0x40;
                INPacket[10] = 0x80;
                INPacket[11] = 0x80;
                INPacket[12] = 0x40;
                INPacket[13] = 0x20;
                INPacket[14] = 0x10;
                INPacket[15] = 0x08;
                INPacket[16] = 0x04;
                INPacket[17] = 0x02;
                INPacket[18] = 0xFF;
                INPacket[19] = 0x05;
                INPacket[20] = 0x50;
                INPacket[21] = 0x55;
                USBGenericInHandle = USBGenWrite(USBGEN_EP_NUM,(BYTE*)&INPacket,USBGEN_EP_SIZE);
            case 0x81:  //Get push button state command from PC application.
				//Now check to make sure no previous attempts to send data to the host are still pending.  If any attemps are still
				//pending, we do not want to write to the endpoint 1 IN buffer again, until the previous transaction is complete.
				//Otherwise the unsent data waiting in the buffer will get overwritten and will result in unexpected behavior.
                if(!USBHandleBusy(USBGenericInHandle))
	            {
		            //The endpoint was not "busy", therefore it is safe to write to the buffer and arm the endpoint.
                    INPacket[0] = 0x81;				//Echo back to the host PC the command we are fulfilling in the first byte.  In this case, the Get Pushbutton State command.
    				if(BUTT == 1)					//pushbutton not pressed, pull up resistor on circuit board is pulling the PORT pin high
    				{
    					INPacket[1] = 0x01;
    				}
    				else							//sw2 must be == 0, pushbutton is pressed and overpowering the pull up resistor
    				{
    					INPacket[1] = 0x00;
    				}
                    INPacket[2] = RED_LED;
	                //The USBGenWrite() function call "arms" the endpoint (and makes the handle indicate the endpoint is busy).
	                //Once armed, the data will be automatically sent to the host (in hardware by the SIE) the next time the
	                //host polls the endpoint.  Once the data is successfully sent, the handle (in this case USBGenericInHandle)
	                //will indicate the the endpoint is no longer busy.
			USBGenericInHandle = USBGenWrite(USBGEN_EP_NUM,(BYTE*)&INPacket,USBGEN_EP_SIZE);
                }
                break;
        }

        //Re-arm the OUT endpoint for the next packet:
	    //The USBGenRead() function call "arms" the endpoint (and makes it "busy").  If the endpoint is armed, the SIE will
	    //automatically accept data from the host, if the host tries to send a packet of data to the endpoint.  Once a data
	    //packet addressed to this endpoint is received from the host, the endpoint will no longer be busy, and the application
	    //can read the data which will be sitting in the buffer.
        USBGenericOutHandle = USBGenRead(USBGEN_EP_NUM,(BYTE*)&OUTPacket,USBGEN_EP_SIZE);
    }
}//end ProcessIO


/******************************************************************************
 * Function:        void USBCBInitEP(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called when the device becomes
 *                  initialized, which occurs after the host sends a
 * 					SET_CONFIGURATION (wValue not = 0) request.  This
 *					callback function should initialize the endpoints
 *					for the device's usage according to the current
 *					configuration.
 *
 * Note:            None
 *****************************************************************************/
void USBCBInitEP(void)
{
    //Enable the application endpoints
    USBEnableEndpoint(USBGEN_EP_NUM,USB_OUT_ENABLED|USB_IN_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
    //Arm the application OUT endpoint, so it can receive a packet from the host
    USBGenericOutHandle = USBGenRead(USBGEN_EP_NUM,(BYTE*)&OUTPacket,USBGEN_EP_SIZE);
}


/*******************************************************************
 * Function:        BOOL USER_USB_CALLBACK_EVENT_HANDLER(
 *                        int event, void *pdata, WORD size)
 *
 * PreCondition:    None
 *
 * Input:           int event - the type of event
 *                  void *pdata - pointer to the event data
 *                  WORD size - size of the event data
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called from the USB stack to
 *                  notify a user application that a USB event
 *                  occured.  This callback is in interrupt context
 *                  when the USB_INTERRUPT option is selected.
 *
 * Note:            None
 *******************************************************************/
BOOL USER_USB_CALLBACK_EVENT_HANDLER(int event, void *pdata, WORD size)
{
    switch( event )
    {
        case EVENT_TRANSFER:
            //Add application specific callback task or callback function here if desired.
            break;
        case EVENT_SOF:
            //USBCB_SOF_Handler();
            break;
        case EVENT_SUSPEND:
            //USBCBSuspend();
            break;
        case EVENT_RESUME:
            //USBCBWakeFromSuspend();
            break;
        case EVENT_CONFIGURED:
            USBCBInitEP();
            break;
        case EVENT_SET_DESCRIPTOR:
            //USBCBStdSetDscHandler();
            break;
        case EVENT_EP0_REQUEST:
            //USBCBCheckOtherReq();
            break;
        case EVENT_BUS_ERROR:
            //USBCBErrorHandler();
            break;
        case EVENT_TRANSFER_TERMINATED:
            //Add application specific callback task or callback function here if desired.
            //The EVENT_TRANSFER_TERMINATED event occurs when the host performs a CLEAR
            //FEATURE (endpoint halt) request on an application endpoint which was
            //previously armed (UOWN was = 1).  Here would be a good place to:
            //1.  Determine which endpoint the transaction that just got terminated was
            //      on, by checking the handle value in the *pdata.
            //2.  Re-arm the endpoint if desired (typically would be the case for OUT
            //      endpoints).
            break;
        default:
            break;
    }
    return TRUE;
}


void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
{
    while(1);
}

void vApplicationMallocFailedHook()
{
    while(1);
}