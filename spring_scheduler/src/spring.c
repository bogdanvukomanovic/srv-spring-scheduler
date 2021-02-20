/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <limits.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* My includes. */
#include "utils.h"
#include "job.h"
#include "spring.h"


void vUpdateEligibility( JobBatch *myBatch )
{
	for ( int i = 0; i < myBatch->iJobCount; i++ )
	{
		Job *job = &( myBatch->xJobArray[i] );
		if ( job->iPredecessorCount == 0 ) {
			job->E = 1;
		}
	}
}

void vDeadlineValidation( JobBatch *myBatch, int iTime )
{
	for ( int i = 0; i < myBatch->iJobCount; i++ ) {
		Job job = myBatch->xJobArray[i];
		if ( job.iChecked == 0 ) {
			if ( iTime + job.xExecutionTime > job.xDeadline ) { 		// iTime > job.xDeadline
				xprintf( "Error [JOB %d]: Can't make schedule for the given heuristic function - deadline exceeded! Exiting...\n", job.iJobID );
				xprintf( "%d + %d > %d\n", iTime, job.xExecutionTime, job.xDeadline );
				exit(0);
			}
		}
	}
}


// H = a
int iGetHeuristicFCFS( Job *job ) {
	return job->xStartTime;
}

// H = C
int iGetHeuristicSJF( Job *job ) {
	return job->xExecutionTime;
}

// H = d
int iGetHeuristicEDF( Job *job ) {
	return job->xDeadline;
}

// H = d + W * C
int iGetHeuristicEDF_SJF( Job *job, int W ) {
	return job->xDeadline + W * job->xExecutionTime;
}

// H = T_est
int iGetHeuristicESTF( Job *job, int *piResources ) {

	// piResources = [ 3, 5, 17, 2, 0 ]
	// job->piUsingResourcesIndexes = [ 1, 2, 4 ]

	int a = job->xStartTime;
	int iMaxResourceWaitTime = 0;

	for ( int i = 0; i < job->iUsingResourcesCount; i++ )
	{
		int iTempMaxResourceWaitTime = piResources[ job->piUsingResourcesIndexes[i] ];
		if ( iTempMaxResourceWaitTime > iMaxResourceWaitTime )
			iMaxResourceWaitTime = iTempMaxResourceWaitTime;
	}

	return a >= iMaxResourceWaitTime ? a : iMaxResourceWaitTime;
}

// H = d + W * T_est
int iGetHeuristicEDF_ESTF( Job *job, int *piResources, int W ) {
	return job->xDeadline + W * iGetHeuristicESTF( job, piResources );
}


int iGetHeuristic( JobBatch *myBatch, Job *job )
{

	int heuristic = 0;

	int W = myBatch->xHeuristic.W;
	int *piResources = myBatch->piResources;

	if ( myBatch->xHeuristic.FCFS == 1 )
		heuristic += iGetHeuristicFCFS( job );

	if ( myBatch->xHeuristic.SJF == 1 )
		heuristic += iGetHeuristicSJF( job );

	if ( myBatch->xHeuristic.EDF == 1 )
		heuristic += iGetHeuristicEDF( job );

	if ( myBatch->xHeuristic.EDF_SJF == 1 )
		heuristic += iGetHeuristicEDF_SJF( job, W );

	if ( myBatch->xHeuristic.ESTF == 1 )
		heuristic += iGetHeuristicESTF( job, piResources );

	if ( myBatch->xHeuristic.EDF_ESTF == 1 )
		heuristic += iGetHeuristicEDF_ESTF( job, piResources, W );


	return heuristic;
}


int *piEligibilitySpringOrder( JobBatch *myBatch )
{
	int i, j, k;
	int *piSpringOrderResult = ( int* ) malloc ( sizeof( int ) * ( myBatch->iJobCount ) );

	int iTime = 0;

	for ( i = 0; i < myBatch->iJobCount; i++ ) {

		int iMinHeuristic = INT_MAX;
		int iMinHeuristicJobIndex = -1;

		for ( j = 0; j < myBatch->iJobCount; j++ )
		{

			vUpdateEligibility( myBatch );

			Job job = myBatch->xJobArray[j];
			int iTempMinHeuristic = INT_MAX;

			if ( job.iChecked == 0 && job.E == 1) {

				iTempMinHeuristic = iGetHeuristic( myBatch, &job );

				if ( iTempMinHeuristic < iMinHeuristic ) {
					iMinHeuristic = iTempMinHeuristic;
					iMinHeuristicJobIndex = j;
				}
			}

		}

		piSpringOrderResult[i] = iMinHeuristicJobIndex;
		myBatch->xJobArray[ iMinHeuristicJobIndex ].iChecked = 1;

		// ---

		Job *xPredecessor = &( myBatch->xJobArray[ iMinHeuristicJobIndex ] );
		int iSuccessorCount = xPredecessor->iSuccessorCount;

		for ( k = 0; k < iSuccessorCount; k++ )
		{
			Job *xSuccessor = xPredecessor->pxSuccessors[k];
			xSuccessor->iPredecessorCount--;
		}

		// ---

		iTime += myBatch->xJobArray[ iMinHeuristicJobIndex ].xExecutionTime;
		xprintf( "iTime: %d\n", iTime );
		vDeadlineValidation( myBatch, iTime );
	}

	return piSpringOrderResult;
}



int *piSpringOrder( JobBatch *myBatch )
{
	int i, j;
	int *piSpringOrderResult = ( int* ) malloc ( sizeof( int ) * ( myBatch->iJobCount ) );

	//vPrintBatch( myBatch );
	//vPrintJobs(  myBatch );

	int iTime = 0;

	for ( i = 0; i < myBatch->iJobCount; i++ )
	{
		int iMinHeuristic = INT_MAX;
		int iMinHeuristicJobIndex = -1;

		for ( j = 0; j < myBatch->iJobCount; j++ )
		{
			Job job = myBatch->xJobArray[j];
			int iTempMinHeuristic = INT_MAX;

			if ( job.iChecked == 0 ) {

				iTempMinHeuristic = iGetHeuristic( myBatch, &job );

				if ( iTempMinHeuristic < iMinHeuristic ) {
					iMinHeuristic = iTempMinHeuristic;
					iMinHeuristicJobIndex = j;
				}
			}
		}

		piSpringOrderResult[i] = iMinHeuristicJobIndex;
		myBatch->xJobArray[ iMinHeuristicJobIndex ].iChecked = 1;

		iTime += myBatch->xJobArray[ iMinHeuristicJobIndex ].xExecutionTime;
		xprintf( "iTime: %d\n", iTime );
	 	vDeadlineValidation( myBatch, iTime );

	}

	return piSpringOrderResult;
}



void vWrapperTask( void *params )
{
	Job *myJob = ( Job* )params;

	myJob->xJobState = RUNNING;
	myJob->func( myJob->pvParams );
	myJob->xJobState = FINISHED;

	vTaskDelete(0);
}

void vSchedulerTask( void *params )
{

	JobBatch *myBatch = ( JobBatch * ) params;

	for ( int i = 0; i < myBatch->iJobCount; i++ )
	{
		myBatch->xJobArray[i].xJobState = NEW;
		xTaskCreate( vWrapperTask, "wrapper", 1024, &( myBatch->xJobArray[i] ), 1, &( myBatch->xJobArray[i].xTaskHandle ) );
		vTaskSuspend( myBatch->xJobArray[i].xTaskHandle );
	}

	while ( 1 )
	{

		for ( int i = 0; i < myBatch->iJobCount; i++ )
		{

			int iJobIndex = myBatch->piSpringOrderResult[i];
			Job xCurrentJob = myBatch->xJobArray[ iJobIndex ];

			if ( xCurrentJob.xJobState != FINISHED ) {

				vTaskResume( xCurrentJob.xTaskHandle );
				vTaskPrioritySet( xCurrentJob.xTaskHandle, 3 );

				vTaskDelay( xCurrentJob.xExecutionTime );
			}
		}
	}


	vTaskDelete(0);
}

int spring( JobBatch *myBatch )
{

	if (myBatch->xHeuristic.E ) {
		myBatch->piSpringOrderResult = piEligibilitySpringOrder( myBatch );
	} else {
		myBatch->piSpringOrderResult = piSpringOrder( myBatch );
	}

	xprintf( "Schedule: " );
	for ( int i = 0; i < myBatch->iJobCount; i++ )
	{
		printf( "%d ", myBatch->piSpringOrderResult[i] );
	}
	xprintf("\n\n");



	xTaskCreate(vSchedulerTask, "scheduler", 1024, myBatch, 2, NULL);
	vTaskStartScheduler();

	return 0;
}

