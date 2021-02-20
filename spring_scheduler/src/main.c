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
#include "utils.h"
#include "spring.h"
#include "job.h"


/**
 * When mode is 0 we are handling heuristic functions from console input.
 * When mode is 1 we are handling heuristic functions from configuration file.
 * File 'in' is only used when mode = 1, to read the value of W if we are using H = d + W * C or H = d + W * T_est.
 **/
void vHandleFunctions( char *functions, JobBatch *myBatch, int mode, FILE *in )
{

	myBatch->xHeuristic.E 			= 0; // Eligibility
	myBatch->xHeuristic.FCFS 		= 0; // H = a ( First Come First Served )
	myBatch->xHeuristic.SJF 		= 0; // H = C ( Shortest Job First )
	myBatch->xHeuristic.EDF 		= 0; // H = d ( Earliest Deadline First )
	myBatch->xHeuristic.ESTF 		= 0; // H = T_est ( Earliest Start Time First )
	myBatch->xHeuristic.EDF_SJF		= 0; // H = d + W * C
	myBatch->xHeuristic.EDF_ESTF 	= 0; // H = d + W * T_est

	myBatch->xHeuristic.W 			= 1;

	char *function;

	function = strtok(functions, "|");

	while (function != NULL)
	{
		if ( !strcmp( function, "E" ) )
			myBatch->xHeuristic.E = 1;
		else if ( !strcmp( function, "a" ) )
			myBatch->xHeuristic.FCFS = 1;
		else if ( !strcmp( function, "C" ) )
			myBatch->xHeuristic.SJF = 1;
		else if ( !strcmp( function, "d" ) )
			myBatch->xHeuristic.EDF = 1;
		else if ( !strcmp( function, "T_est" ) )
			myBatch->xHeuristic.ESTF = 1;
		else if ( !strcmp( function, "d+W*C" ) )
			myBatch->xHeuristic.EDF_SJF = 1;
		else if ( !strcmp( function, "d+W*T_est" ) )
			myBatch->xHeuristic.EDF_ESTF = 1;
		else {
			xprintf( "Error: Not supported heuristic function '%s'! Exiting...\n", function );
			exit(0);
		}

		function = strtok(NULL, "|");
	}


	int W;
	if ( mode == 0 && ( myBatch->xHeuristic.EDF_SJF != 0 || myBatch->xHeuristic.EDF_ESTF != 0 ) ) {

		xprintf( "Insert value of W: " );
		scanf( "%d", &W );
		myBatch->xHeuristic.W = W;

	} else if (  mode == 1 && ( myBatch->xHeuristic.EDF_SJF != 0 || myBatch->xHeuristic.EDF_ESTF != 0 ) ) {

		fscanf( in, "%d", &W);
		myBatch->xHeuristic.W = W;

	}

}



JobBatch* xConsoleInput()
{
	JobBatch *myBatch = ( JobBatch* ) malloc ( sizeof ( JobBatch ) );

	int iResourcesCountInput, iJobCountInput, iStartTimeInput, iExecutionTimeInput, iDeadlineInput;
	int iUsingResourceCount;

	// [1] Info about resources ( System scope ):
	xprintf( " -> How many resources are Jobs going to share? " );
	scanf( "%d", &iResourcesCountInput );
	myBatch->iResourcesCount = iResourcesCountInput;
	myBatch->piResources = ( int* ) malloc ( sizeof ( int ) * ( iResourcesCountInput ) );

	int iEarliestAvailableTime;
	for ( int i = 0; i < iResourcesCountInput; i++ )
	{
		xprintf( "\t[RESOURCE %d] Earliest available time: ", i );
		scanf( "%d", &iEarliestAvailableTime );
		myBatch->piResources[i] = iEarliestAvailableTime;
	}


	// [2] Info about jobs:
	xprintf( " -> How many Jobs is going to exist in the system? " );
	scanf( "%d", &iJobCountInput );
	myBatch->iJobCount = iJobCountInput;
	myBatch->xJobArray = ( Job* ) malloc ( sizeof( Job ) * ( iJobCountInput ) );

	if ( iJobCountInput == 0 ) {
		xprintf( "Error: Minimum number of Jobs is 1! Exiting...\n" );
		exit(0);
	}
	xprintf( " -> Please input information about each Job...\n" );
	for ( int i = 0; i < iJobCountInput; i++ )
	{
		xprintf( "\t[Job %d] Start time: ", i );
		scanf( "%d", &iStartTimeInput );

		xprintf( "\t[Job %d] Execution time: ", i );
		scanf( "%d", &iExecutionTimeInput );

		xprintf( "\t[Job %d] Deadline: ", i );
		scanf( "%d", &iDeadlineInput );

		int *iJobFunctionParameter;
		iJobFunctionParameter = ( int* ) malloc ( sizeof ( int ) * ( 2 ) );
		iJobFunctionParameter[0] = i;
		iJobFunctionParameter[1] = iExecutionTimeInput;

		xprintf( "\t-> How many resources [Job %d] is going to use? ", i );
		scanf( "%d", &iUsingResourceCount );
		myBatch->xJobArray[i].piUsingResourcesIndexes = ( int* ) malloc ( sizeof ( int ) * ( iUsingResourceCount ) );

		if (iUsingResourceCount != 0)
			xprintf( "\t-> Please input indexes of resources [Job %d] is using:\n", i );

		int iUsingResourceIndex;
		for ( int j = 0; j < iUsingResourceCount; j++ )
		{
			xprintf("\t\t[Resource %d] index of [Job %d] is: ", i, j )
			scanf( "%d", &iUsingResourceIndex );
			myBatch->xJobArray[i].piUsingResourcesIndexes[j] = iUsingResourceIndex;
		}


		vMakeJob( &( myBatch->xJobArray[i] ), vUserJobFunction, iJobFunctionParameter, iStartTimeInput, iExecutionTimeInput, iDeadlineInput, iUsingResourceCount, i );

		xprintf("\t-----------------------------------------------------------\n");
	}


	// [3] Heuristic functions:
	char functions[64];
	xprintf("Insert heuristic functions you want to use: "); 	fflush(stdin);
	gets(functions);
	vHandleFunctions( functions, myBatch, 0, NULL );


	// [4] Precedence information:
	xprintf(" -> Please connect Jobs... \n");
	while ( 1 )
	{
		int option;
		xprintf( "\t[1] Connect Jobs\n\t[2] Exit\n" );
		scanf( "%d", &option );

		int iPredecessor, iSuccessor;

		switch ( option )
		{
			case 1:

				xprintf( "\tInsert Job predecessor: " );
				scanf( "%d", &iPredecessor );

				xprintf( "\tInsert Job successor: ");
				scanf( "%d", &iSuccessor );

				Job *xPredecessor = &( myBatch->xJobArray[ iPredecessor ] );
				Job *xSuccessor = &( myBatch->xJobArray[ iSuccessor ] );

				xPredecessor->pxSuccessors[ xPredecessor->iSuccessorCount++ ] = xSuccessor;
				xSuccessor->pxPredecessors[ xSuccessor->iPredecessorCount++ ] = xPredecessor;

				break;

			case 2:
				goto exit;

			default:
				xprintf( "Error: Invalid option." )
				break;
		}

		xprintf("\t-----------------------------------------------------------\n");

	}

	exit:
	xprintf("Exited...\n");
	return myBatch;
}



JobBatch* xReadConfiguration()
{
	JobBatch *myBatch = ( JobBatch* ) malloc ( sizeof ( JobBatch ) );

	FILE *in;

	in = fopen( "job_config.txt", "r" );
	if ( in == NULL ) {
		xprintf( "Error: File you are trying to open does not exist!\n" );
		exit(0);
	}

	char chFileLineType[16];

	int iResourcesCountConfig, iJobCountConfig;
	int iCurrJob = 0;

	while( fscanf( in, "%s", chFileLineType ) == 1 ) {

		if ( !strcmp( chFileLineType, "RESOURCES" ) ) {


			fscanf( in, "%d", &iResourcesCountConfig );

			myBatch->iResourcesCount = iResourcesCountConfig;
			myBatch->piResources = ( int* ) malloc ( sizeof ( int ) * ( iResourcesCountConfig ) );

			int iResourceEarliestTime;
			for ( int i = 0; i < iResourcesCountConfig; i++ )
			{
				fscanf( in, "%d", &iResourceEarliestTime );
				myBatch->piResources[i] = iResourceEarliestTime;
			}


		} else if ( !strcmp( chFileLineType, "JOB_NUMBER" ) ) {


			fscanf( in, "%d", &iJobCountConfig );

			myBatch->iJobCount = iJobCountConfig;
			myBatch->xJobArray = ( Job* ) malloc ( sizeof( Job ) * ( iJobCountConfig ) );


		} else if ( !strcmp( chFileLineType, "JOB" ) ) {


			int *iJobFunctionParameter;
			int iStartTimeConfig, iExecutionTimeConfig, iDeadlineConfig;

			iJobFunctionParameter = ( int* ) malloc ( sizeof ( int ) * ( 2 ) );

			fscanf( in, "%d", &iJobFunctionParameter[0] );

			fscanf( in, "%d", &iStartTimeConfig );
			fscanf( in, "%d", &iExecutionTimeConfig );
			fscanf( in, "%d", &iDeadlineConfig );

			iJobFunctionParameter[1] = iExecutionTimeConfig;

			int iUsingResourceCount, iUsingResourceIndex;
			fscanf( in, "%d", &iUsingResourceCount );

			myBatch->xJobArray[ iCurrJob ].piUsingResourcesIndexes = ( int* ) malloc ( sizeof ( int ) * ( iUsingResourceCount ) );

			for ( int i = 0; i < iUsingResourceCount; i++ ) {
				fscanf( in, "%d", &iUsingResourceIndex );
				myBatch->xJobArray[ iCurrJob ].piUsingResourcesIndexes[i] = iUsingResourceIndex;
			}


			vMakeJob( &( myBatch->xJobArray[ iCurrJob ] ), vUserJobFunction, iJobFunctionParameter, iStartTimeConfig, iExecutionTimeConfig, iDeadlineConfig, iUsingResourceCount, iCurrJob );

			iCurrJob++;


		} else if ( !strcmp( chFileLineType, "HEURISTIC" ) ) {


			char functions[64];
			fscanf( in, "%s", functions );

			vHandleFunctions( functions, myBatch, 1, in );


		} else if ( !strcmp( chFileLineType, "CONNECT" ) ) {


			int iPredecessor, iSuccessor;

			fscanf( in, "%d", &iPredecessor );
			fscanf( in, "%d", &iSuccessor );

			Job *xPredecessor = &( myBatch->xJobArray[ iPredecessor ] );
			Job *xSuccessor = &( myBatch->xJobArray[ iSuccessor ] );

			xPredecessor->pxSuccessors[ xPredecessor->iSuccessorCount++ ] = xSuccessor;
			xSuccessor->pxPredecessors[ xSuccessor->iPredecessorCount++ ] = xPredecessor;


		}
	}


	return myBatch;
}


int main( void )
{

	int mode;
	xprintf( "How would you like to configure the system?\n\t[1] Manual console input\n\t[2] Read configuration from file\n" );
	scanf( "%d", &mode );

	JobBatch *myBatch;

	switch ( mode )
	{
		case 1:
			xprintf( "Welcome to the Manual console input mode.\n\n" );
			myBatch = xConsoleInput();
			break;

		case 2:
			xprintf( "Welcome to the Read configuration from file mode.\n\n" );
			myBatch = xReadConfiguration();
			break;

		default:
			xprintf( "Error: Invalid mode!\n" );
			return 0;
	}

	//vPrintBatch( myBatch );
	//vPrintJobs( myBatch );

	spring( myBatch );

	xprintf("FINISHED ALL\n");	// Never reaches this line... We start "Spring scheduler" in spring().

	return 0;
}

