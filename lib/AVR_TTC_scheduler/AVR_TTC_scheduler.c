#include "AVR_TTC_scheduler.h"
#include <avr/io.h>
#include <avr/interrupt.h>

// The array of tasks
sTask SCH_tasks_G[SCH_MAX_TASKS];


/*------------------------------------------------------------------*-

  SCH_Dispatch_Tasks()

  This is the 'dispatcher' function.  When a task (function)
  is due to run, SCH_Dispatch_Tasks() will run it.
  This function must be called (repeatedly) from the main loop.

-*------------------------------------------------------------------*/

void SCH_Dispatch_Tasks(void)
{
   unsigned char Index;

   // Dispatches (runs) the next task (if one is ready)
   for(Index = 0; Index < SCH_MAX_TASKS; Index++)
   {
      if((SCH_tasks_G[Index].RunMe > 0) && (SCH_tasks_G[Index].pTask != 0))
      {
         (*SCH_tasks_G[Index].pTask)();  // Run the task
         SCH_tasks_G[Index].RunMe -= 1;   // Reset / reduce RunMe flag

         // Periodic tasks will automatically run again
         // - if this is a 'one shot' task, remove it from the array
         if(SCH_tasks_G[Index].Period == 0)
         {
            SCH_Delete_Task(Index);
         }
      }
   }
}

unsigned char SCH_Add_Task(void (*pFunction)(), const unsigned int DELAY, const unsigned int PERIOD)
{
   unsigned char Index = 0;

   // First find a gap in the array (if there is one)
   while((SCH_tasks_G[Index].pTask != 0) && (Index < SCH_MAX_TASKS))
   {
      Index++;
   }

   // Have we reached the end of the list?   
   if(Index == SCH_MAX_TASKS)
   {
      // Task list is full, return an error code
      return SCH_MAX_TASKS;  
   }

   // If we're here, there is a space in the task array
   SCH_tasks_G[Index].pTask = pFunction;
   SCH_tasks_G[Index].Delay =DELAY;
   SCH_tasks_G[Index].Period = PERIOD;
   SCH_tasks_G[Index].RunMe = 0;

   // return position of task (to allow later deletion)
   return Index;
}

/*------------------------------------------------------------------*-

  SCH_Delete_Task()

  Removes a task from the scheduler.  Note that this does
  *not* delete the associated function from memory: 
  it simply means that it is no longer called by the scheduler. 
 
  TASK_INDEX - The task index.  Provided by SCH_Add_Task(). 

  RETURN VALUE:  RETURN_ERROR or RETURN_NORMAL

-*------------------------------------------------------------------*/

unsigned char SCH_Delete_Task(const unsigned char TASK_INDEX)
{
   // Return_code can be used for error reporting, NOT USED HERE THOUGH!
   unsigned char Return_code = 0;

   SCH_tasks_G[TASK_INDEX].pTask = 0;
   SCH_tasks_G[TASK_INDEX].Delay = 0;
   SCH_tasks_G[TASK_INDEX].Period = 0;
   SCH_tasks_G[TASK_INDEX].RunMe = 0;

   return Return_code;
}

/*------------------------------------------------------------------*-

  SCH_Init_T1()

  Scheduler initialisation function.  Prepares scheduler
  data structures and sets up timer interrupts at required rate.
  You must call this function before using the scheduler.  

-*------------------------------------------------------------------*/

void SCH_Init_T1(void)
{
   unsigned char i;

   for(i = 0; i < SCH_MAX_TASKS; i++)
   {
      SCH_Delete_Task(i);
   }

   // Set up Timer 1
   // Values for 1ms and 10ms ticks are provided for various crystals

   // Hier moet de timer periode worden aangepast ....!
   OCR1A = (uint16_t)625;   		     // 10ms = (256/16.000.000) * 625
   TCCR1B = (1 << CS12) | (1 << WGM12);  // prescale op 64, top counter = value OCR1A (CTC mode)
   TIMSK1 = 1 << OCIE1A;   		     // Timer 1 Output Compare A Match Interrupt Enable
}

/*------------------------------------------------------------------*-

  SCH_Start()

  Starts the scheduler, by enabling interrupts.

  NOTE: Usually called after all regular tasks are added,
  to keep the tasks synchronised.

  NOTE: ONLY THE SCHEDULER INTERRUPT SHOULD BE ENABLED!!! 
 
-*------------------------------------------------------------------*/

void SCH_Start(void)
{
      sei();
}

/*------------------------------------------------------------------*-

  SCH_Update

  This is the scheduler ISR.  It is called at a rate 
  determined by the timer settings in SCH_Init_T1().

-*------------------------------------------------------------------*/

ISR(TIMER1_COMPA_vect)
{
   unsigned char Index;
   for(Index = 0; Index < SCH_MAX_TASKS; Index++)
   {
      // Check if there is a task at this location
      if(SCH_tasks_G[Index].pTask)
      {
         if(SCH_tasks_G[Index].Delay == 0)
         {
            // The task is due to run, Inc. the 'RunMe' flag
            SCH_tasks_G[Index].RunMe += 1;

            if(SCH_tasks_G[Index].Period)
            {
               // Schedule periodic tasks to run again
               SCH_tasks_G[Index].Delay = SCH_tasks_G[Index].Period;
               SCH_tasks_G[Index].Delay -= 1;
            }
         }
         else
         {
            // Not yet ready to run: just decrement the delay
            SCH_tasks_G[Index].Delay -= 1;
         }
      }
   }
}

// ------------------------------------------------------------------
void check_to_run() {
		volatile uint8_t swA = readButtons();
		if(swA & 0x80) {
			while (1) {
				uint8_t swB = readButtons();
				if(swB & 0x40) {
					break;
					}else{
					counting();
				}
			}
		}
}

int main()
{
	SCH_Init_T1();
	SCH_Add_Task(setup,0,0);
	SCH_Add_Task(check_to_run,0,1);
	SCH_Start();
	while(1) {
		SCH_Dispatch_Tasks();
	}
}