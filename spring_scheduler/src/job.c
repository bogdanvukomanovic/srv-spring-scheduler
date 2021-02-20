/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* My includes. */
#include "job.h"
#include "utils.h"

void vUserJobFunction( void *param )
{
	int *iParam = ( int* ) param;

	xprintf( "Start tick: %d\n", xTaskGetTickCount() );

	xprintf( "  JOB %d\n", iParam[0] );
	vTaskDelay( iParam[1] );

	xprintf( "End tick:   %d\n\n", xTaskGetTickCount() );
}

void vPrintJobs( JobBatch *batch )
{

	xprintf( "#####################################################\n" );

	for ( int i = 0; i < batch->iJobCount; i++ )
	{

		xprintf( "-***********************************************-\n" );


		Job job = batch->xJobArray[i];

		xprintf( "\tJOB ID         : %d\n", job.iJobID );

		xprintf( "\t - - - - - - - - - - - - - \n" );

		xprintf( "\tSTART         : %d\n", job.xStartTime );
		xprintf( "\tEXECUTION     : %d\n", job.xExecutionTime );
		xprintf( "\tDEADLINE      : %d\n", job.xDeadline );

		xprintf( "\t - - - - - - - - - - - - - \n" );

		xprintf( "\tRESOURCE COUNT: %d ->", job.iUsingResourcesCount );
		for ( int j = 0; j < job.iUsingResourcesCount; j++ ) {
			xprintf( " %d", job.piUsingResourcesIndexes[j] );
		}

		xprintf( "\n\t - - - - - - - - - - - - - \n" );

		xprintf( "\tPREDECESSORS  : %d\n", job.iPredecessorCount );
		xprintf( "\tSUCCESSORS    : %d\n", job.iSuccessorCount );

		xprintf( "\t - - - - - - - - - - - - - \n" );

		xprintf( "\tELIGIBILITY   : %d\n", job.E );
		xprintf( "\tCHECKED       : %d\n", job.iChecked );


		xprintf( "-***********************************************-\n" );
	}

	xprintf( "#####################################################\n" );
}

void vPrintBatch( JobBatch *batch )
{
	xprintf( "#####################################################\n" );


	xprintf( "\tNUMBER OF JOBS          : %d\n", batch->iJobCount );

	xprintf( "\t - - - - - - - - - - - - - \n" );

	xprintf( "\tE                       : %d\n", batch->xHeuristic.E );
	xprintf( "\tFCFS (a)                : %d\n", batch->xHeuristic.FCFS );
	xprintf( "\tSJF (C)                 : %d\n", batch->xHeuristic.SJF );
	xprintf( "\tEDF (d)                 : %d\n", batch->xHeuristic.EDF );
	xprintf( "\tESTF (T_est)            : %d\n", batch->xHeuristic.ESTF );
	xprintf( "\tEDF_SJF (d + W * C)     : %d\n", batch->xHeuristic.EDF_SJF );
	xprintf( "\tEDF_ESTF (d + W * T_est): %d\n", batch->xHeuristic.EDF_ESTF );
	xprintf( "\tW                       : %d\n", batch->xHeuristic.W );

	xprintf( "\t - - - - - - - - - - - - - \n" );

	xprintf( "\tNUMBER OF RESOURCES     : %d ->", batch->iResourcesCount );
	for (int i = 0; i < batch->iResourcesCount; i++ ) {
		xprintf( " %d", batch->piResources[i] );
	}


	xprintf( "\n#####################################################\n" );
}


void vMakeJob( Job *newJob, void ( *func )( void *pvParams ), void *pvParams, TickType_t xStartTime, TickType_t xExecutionTime, TickType_t xDeadline, int iUsingResourceCount, int iJobID )
{

	newJob->func = func;
	newJob->pvParams = pvParams;

	newJob->xStartTime = xStartTime;
	newJob->xExecutionTime = xExecutionTime;
	newJob->xDeadline = xDeadline;

	newJob->iJobID = iJobID;

	newJob->iUsingResourcesCount = iUsingResourceCount;

	newJob->xJobState = NEW;
	newJob->E = 0;
	newJob->iChecked = 0;
	newJob->iPredecessorCount = 0;
	newJob->iSuccessorCount = 0;

}
