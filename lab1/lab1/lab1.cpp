#include <predef.h>
#include <stdio.h>
#include <ctype.h>
#include <startnet.h>
#include <autoupdate.h>
#include <smarttrap.h>
#include <taskmon.h>
#include <NetworkDebug.h>
#include <pinconstant.h>
#include <pins.h>
#include <basictypes.h>
#include "LCD.h"
#include "bitmaps.h"
#include "error_wrapper.h"
#include "point.h"


extern "C" {
void UserMain(void * pd);
void StartTask1(void);
void StartTask2(void);
void StartTask3(void);
void Task1Main(void * pd);
void Task2Main(void * pd);
void Task3Main(void * pd);
}




/* Task stacks for all the user tasks */
/* If you add a new task you'll need to add a new stack for that task */
DWORD Task1Stk[USER_TASK_STK_SIZE] __attribute__( ( aligned( 4 ) ) );
DWORD Task2Stk[USER_TASK_STK_SIZE] __attribute__( ( aligned( 4 ) ) );
DWORD Task3Stk[USER_TASK_STK_SIZE] __attribute__( ( aligned( 4 ) ) );


const char * AppName="Put your name here hello";


/* User task priorities always based on MAIN_PRIO */
/* The priorities between MAIN_PRIO and the IDLE_PRIO are available */
#define TASK1_PRIO 	MAIN_PRIO + 1
#define TASK2_PRIO 	MAIN_PRIO + 2
#define TASK3_PRIO	MAIN_PRIO + 3

#define WAIT_FOREVER 0

const BYTE sprite[] = {0x80, 0x88, 0xEB, 0x3f, 0xEB, 0x88, 0x80};

LCD myLCD;
OS_SEM Task1Semaphore;
OS_SEM Task2Semaphore;
OS_SEM Task3Semaphore;

void draw_buddy(int pos){
	myLCD.DrawChar(sprite, char_index[pos]);
	OSTimeDly(TICKS_PER_SECOND);
}

void UserMain(void * pd) {
	BYTE err = OS_NO_ERR;
    InitializeStack();
    OSChangePrio(MAIN_PRIO);
    EnableAutoUpdate();
    StartHTTP();
    EnableTaskMonitor();

    #ifndef _DEBUG
    EnableSmartTraps();
    #endif

    #ifdef _DEBUG
    InitializeNetworkGDB_and_Wait();
    #endif

    iprintf("Application started: %s\n",AppName );

    myLCD.Init();
    myLCD.Clear();
    myLCD.Home();

    OSSemInit(& Task1Semaphore, 1);
    OSSemInit(& Task2Semaphore, 0);
    OSSemInit(& Task3Semaphore, 0);


    StartTask1();
    StartTask2();
    StartTask3();

    while (1) {
        OSTimeDly(TICKS_PER_SECOND);
    }
}


/* Name: StartTask1
 * Description: Creates the task main loop.
 * Inputs: none
 * Outputs: none
 */
void StartTask1(void) {
	BYTE err = OS_NO_ERR;

	err = display_error( "StartTask1 fail:",
					OSTaskCreatewName(	Task1Main,
					(void *)NULL,
				 	(void *) &Task1Stk[USER_TASK_STK_SIZE],
				 	(void *) &Task1Stk[0],
				 	TASK1_PRIO, "Task 1" ));


}

/* Name: StartTask2
 * Description: Creates the task main loop.
 * Inputs: none
 * Outputs: none
 */
void StartTask2(void) {
	BYTE err = OS_NO_ERR;

	err = display_error( "StartTask2 fail:",
					OSTaskCreatewName(	Task2Main,
					(void *)NULL,
				 	(void *) &Task2Stk[USER_TASK_STK_SIZE],
				 	(void *) &Task2Stk[0],
				 	TASK2_PRIO, "Task 2" ));
}

/* Name: StartTask3
 * Description: Creates the task main loop.
 * Inputs: none
 * Outputs: none
 */
void StartTask3(void) {
	BYTE err = OS_NO_ERR;

	err = display_error( "StartTask3 fail:",
					OSTaskCreatewName(	Task3Main,
					(void *)NULL,
				 	(void *) &Task3Stk[USER_TASK_STK_SIZE],
				 	(void *) &Task3Stk[0],
				 	TASK3_PRIO, "Task 3" ));
}

/* Name: Task1Main
 * Description: Displays the bitimage on the first two lines
 * Inputs:  void * pd -- pointer to generic data . Currently unused.
 * Outputs: none
 */
void	Task1Main( void * pd) {


	BYTE err = OS_NO_ERR;

	/* place semaphore usage code inside the loop */
	while (1) {
		OSSemPend(&Task1Semaphore, WAIT_FOREVER);
	    int i;
	    for(i=LINE1_ORIGIN; i<= LINE1_END; i++){
	    	draw_buddy(i);
	    }
	    draw_buddy(LINE2_END);

	    OSSemPost(&Task2Semaphore);
	    OSSemPend(&Task1Semaphore, WAIT_FOREVER);
	    draw_buddy(LINE2_ORIGIN);
	    myLCD.Clear();
	    OSTimeDly(TICKS_PER_SECOND);
	    OSSemPost(&Task1Semaphore);
	}
}

/* Name: Task2Main
 * Description:
 * Inputs:  void * pd -- pointer to generic data . Currently unused.
 * Outputs: none
 */
void Task2Main( void * pd) {


	BYTE err = OS_NO_ERR;

	/* place semaphore usage code inside the loop */
	while (1) {
		OSSemPend(&Task2Semaphore, WAIT_FOREVER);
		draw_buddy(LINE3_END);
		draw_buddy(LINE4_END);
		OSSemPost(&Task3Semaphore);

		OSSemPend(&Task2Semaphore, WAIT_FOREVER);
		draw_buddy(LINE4_ORIGIN);
		draw_buddy(LINE3_ORIGIN);
		OSSemPost(&Task1Semaphore);
	}
}

/* Name: Task3Main
 * Description:
 * Inputs:  void * pd -- pointer to generic data . Currently unused.
 * Outputs: none
 */
void	Task3Main( void * pd) {


	BYTE err = OS_NO_ERR;

	/* place semaphore usage code inside the loop */
	while (1) {
		OSSemPend(&Task3Semaphore, WAIT_FOREVER);
		draw_buddy(LINE5_END);
		int i;
		for(i=LINE6_END; i>= LINE6_ORIGIN; i--){
			draw_buddy(i);
		}
		draw_buddy(LINE5_ORIGIN);
		OSSemPost(&Task2Semaphore);
	}
}


