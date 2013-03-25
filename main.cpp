//  Created by R J Cooper.
//  Copyright (c) 2010 Mountainstorm
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#include <stdio.h>

#include <mach/mach.h>
#include <mach/mach_time.h>

#include <CoreFoundation/CFNumber.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/Audio/IOAudioTypes.h>
#include <IOKit/Audio/IOAudioDefines.h>
#include <IOKit/IODataQueueShared.h>
#include <IOKit/IODataQueueClient.h>

#include "CAHostTimeBase.h"


#define kBufferSizeInFrames			( 512 )
#define kSampleFramesPerEnginBuffer	( 9216 )


char g_bufDataStore[ ( 44100 * 2 * sizeof( Float32 ) ) * 11 ] = {0};
char g_bufData[ sizeof( IOAudioBufferDataDescriptor ) - 1 + ( kBufferSizeInFrames * 2 * sizeof( Float32 ) ) ] = {0};
IOAudioBufferDataDescriptor *g_buf = ( IOAudioBufferDataDescriptor * ) g_bufData;



typedef struct __AudioEngineInterface
{
	io_service_t audioEngine;
	io_connect_t audioEngineConnection;	
	io_service_t audioStreams[ 32 ];
	
} AudioEngineInterface;



kern_return_t AudioEngineAPI__start( AudioEngineInterface *self );
kern_return_t AudioEngineAPI__stop( AudioEngineInterface *self );
kern_return_t AudioEngineAPI__getConnectionID( AudioEngineInterface *self, UInt32 *connection );
kern_return_t AudioEngineAPI__getNearestStartTime( AudioEngineInterface *self );
kern_return_t AudioEngineAPI__registerClientBuffer( AudioEngineInterface *self, UInt32 streamID, void *buffer, UInt32 bufferSize, UInt32 bufferSetID );
kern_return_t AudioEngineAPI__unregisterClientBuffer( AudioEngineInterface *self, void *buffer, UInt32 bufferSetID );



kern_return_t AudioEngineAPI__start( AudioEngineInterface *self )
{
	kern_return_t kernResult = 0;
	kernResult = IOConnectCallMethod( self->audioEngineConnection,
									  kIOAudioEngineCallStart,
									  NULL,							// array of scalar (64-bit) input values.
									  0,							// the number of scalar input values.
									  NULL,							// a pointer to the struct input parameter.
									  0,							// the size of the input structure parameter.
									  NULL,							// array of scalar (64-bit) output values.
									  NULL,							// pointer to the number of scalar output values.
									  NULL,							// pointer to the struct output parameter.
									  NULL							// pointer to the size of the output structure parameter.
									  );
	if( kernResult != KERN_SUCCESS )
	{
		printf( "kIOAudioEngineCallStart failed, kernResult: %x\n", kernResult );
		
	} // if
	return( kernResult );
	
} // AudioEngineAPI__start( func )



kern_return_t AudioEngineAPI__stop( AudioEngineInterface *self )
{
	kern_return_t kernResult = 0;
	kernResult = IOConnectCallMethod( self->audioEngineConnection,
									 kIOAudioEngineCallStop,
									 NULL,							// array of scalar (64-bit) input values.
									 0,							// the number of scalar input values.
									 NULL,							// a pointer to the struct input parameter.
									 0,							// the size of the input structure parameter.
									 NULL,							// array of scalar (64-bit) output values.
									 NULL,							// pointer to the number of scalar output values.
									 NULL,							// pointer to the struct output parameter.
									 NULL							// pointer to the size of the output structure parameter.
									 );
	if( kernResult != KERN_SUCCESS )
	{
		printf( "kIOAudioEngineCallStop failed, kernResult: %x\n", kernResult );
		
	} // if
	return( kernResult );
	
} // AudioEngineAPI__stop( func )



kern_return_t AudioEngineAPI__getConnectionID( AudioEngineInterface *self, UInt32 *connection )
{
	kern_return_t kernResult = 0;
	uint64_t params[ 1 ] = {0};
	uint32_t outParams = sizeof( params ) / sizeof( params[ 0 ] );
	
	kernResult = IOConnectCallMethod( self->audioEngineConnection,
	 								  kIOAudioEngineCallGetConnectionID,
									  NULL,							// array of scalar (64-bit) input values.
									  0,							// the number of scalar input values.
									  NULL,							// a pointer to the struct input parameter.
									  0,							// the size of the input structure parameter.
									  params,						// array of scalar (64-bit) output values.
									  &outParams,					// pointer to the number of scalar output values.
									  NULL,							// pointer to the struct output parameter.
									  NULL							// pointer to the size of the output structure parameter.
									  );
	printf( "kIOAudioEngineCallGetConnectionID: %x\n", params[ 0 ] );	
	*connection = params[ 0 ];
	if( kernResult != KERN_SUCCESS )
	{
		printf( "kIOAudioEngineCallGetConnectionID failed, kernResult: %x\n", kernResult );
		
	} // if
	return( kernResult );
	
} // AudioEngineAPI__getConnectionID( func )



kern_return_t AudioEngineAPI__getNearestStartTime( AudioEngineInterface *self )
{
	kern_return_t kernResult = 0;
	//uint64_t params[ 3 ] = {0};
	//uint32_t inParams = sizeof( params ) / sizeof( params[ 0 ] );
	
	// IOAudioStream *audioStream, IOAudioTimeStamp *ioTimeStamp, UInt32 isInput
	/* FIX: I can't see how you can call this.  IOAudioEngineUserClient doesn't resolve the audioStream as an ID, passing it straight to the IOAudioEngine
	kernResult = IOConnectCallMethod( self->audioEngineConnection,
									  kIOAudioEngineCallGetNearestStartTime,
									  params,						// array of scalar (64-bit) input values.
									  inParams,						// the number of scalar input values.
									  NULL,							// a pointer to the struct input parameter.
									  0,							// the size of the input structure parameter.
									  NULL,							// array of scalar (64-bit) output values.
									  NULL,							// pointer to the number of scalar output values.
									  NULL,							// pointer to the struct output parameter.
									  NULL							// pointer to the size of the output structure parameter.
									  );	
	*/
	if( kernResult != KERN_SUCCESS )
	{
		printf( "kIOAudioEngineCallGetNearestStartTime failed, kernResult: %x\n", kernResult );
		
	} // if
	return( kernResult );
	
} // AudioEngineAPI__getNearestStartTime( func )



kern_return_t AudioEngineAPI__registerClientBuffer( AudioEngineInterface *self, UInt32 streamID, void *buffer, UInt32 bufferSize, UInt32 bufferSetID )
{
	kern_return_t kernResult = 0;
	uint64_t params[ 4 ] = {0};
	uint32_t inParams = sizeof( params ) / sizeof( params[ 0 ] );
	
	params[ 0 ] = streamID;
	params[ 1 ] = ( uint64_t ) buffer;
	params[ 2 ] = bufferSize;
	params[ 3 ] = bufferSetID;
	
	// IOAudioStream *audioStream, IOAudioTimeStamp *ioTimeStamp, UInt32 isInput
	kernResult = IOConnectCallMethod( self->audioEngineConnection,
									  kIOAudioEngineCallRegisterClientBuffer,
									  params,						// array of scalar (64-bit) input values.
									  inParams,						// the number of scalar input values.
									  NULL,							// a pointer to the struct input parameter.
									  0,							// the size of the input structure parameter.
									  NULL,							// array of scalar (64-bit) output values.
									  NULL,							// pointer to the number of scalar output values.
									  NULL,							// pointer to the struct output parameter.
									  NULL							// pointer to the size of the output structure parameter.
									  );	
	if( kernResult != KERN_SUCCESS )
	{
		printf( "kIOAudioEngineCallRegisterClientBuffer failed, kernResult: %x\n", kernResult );
		
	} // if
	return( kernResult );
	
} // AudioEngineAPI__registerClientBuffer( func )



kern_return_t AudioEngineAPI__unregisterClientBuffer( AudioEngineInterface *self, void *buffer, UInt32 bufferSetID )
{
	kern_return_t kernResult = 0;
	uint64_t params[ 2 ] = {0};
	uint32_t inParams = sizeof( params ) / sizeof( params[ 0 ] );
	
	params[ 0 ] = ( uint64_t ) buffer;
	params[ 1 ] = bufferSetID;
	
	// IOAudioStream *audioStream, IOAudioTimeStamp *ioTimeStamp, UInt32 isInput
	kernResult = IOConnectCallMethod( self->audioEngineConnection,
									  kIOAudioEngineCallUnregisterClientBuffer,
									  params,						// array of scalar (64-bit) input values.
									  inParams,						// the number of scalar input values.
									  NULL,							// a pointer to the struct input parameter.
									  0,							// the size of the input structure parameter.
									  NULL,							// array of scalar (64-bit) output values.
									  NULL,							// pointer to the number of scalar output values.
									  NULL,							// pointer to the struct output parameter.
									  NULL							// pointer to the size of the output structure parameter.
									  );	
	if( kernResult != KERN_SUCCESS )
	{
		printf( "kIOAudioEngineCallUnregisterClientBuffer failed, kernResult: %x\n", kernResult );
		
	} // if
	return( kernResult );
	
} // AudioEngineAPI__unregisterClientBuffer( func )



void AudioEngineAPI__calculateIOThreadTimeConstraints(UInt64& outPeriod, UInt32& outQuanta, UInt32 noFramesInBuffer, Float64 sampleRate)
{
	static const UInt64	kLowLatencyThreshhold = 1500ULL * 1000ULL;
	static const UInt64	kMedLatencyThreshhold = 4444ULL * 1000ULL;
	static UInt32		kLowLatencyComputeQuantum = 0;
	static UInt32		kMedLatencyComputeQuantum = 0;
	static UInt32		kHighLatencyComputeQuantum = 0;
	
	if(kLowLatencyComputeQuantum == 0)
	{
		kLowLatencyComputeQuantum = static_cast<UInt32>(CAHostTimeBase::ConvertFromNanos(500 * 1000));
		kMedLatencyComputeQuantum = static_cast<UInt32>(CAHostTimeBase::ConvertFromNanos(300 * 1000));
		kHighLatencyComputeQuantum = static_cast<UInt32>(CAHostTimeBase::ConvertFromNanos(100 * 1000));
	}
	
	outPeriod = static_cast<UInt64>((static_cast<Float64>(noFramesInBuffer)/sampleRate) * CAHostTimeBase::GetFrequency());
	UInt64 thePeriodNanos = CAHostTimeBase::ConvertToNanos(outPeriod);
	outQuanta = kHighLatencyComputeQuantum;
	
	if(thePeriodNanos < kLowLatencyThreshhold)
	{
		outQuanta = kLowLatencyComputeQuantum;
	}
	else if(thePeriodNanos < kMedLatencyThreshhold)
	{
		outQuanta = kMedLatencyComputeQuantum;
	}
	
	if(outQuanta > thePeriodNanos)
	{
		outQuanta = static_cast<UInt32>(outPeriod);
	}
} // AudioEngineAPI__calculateIOThreadTimeConstraints( func )



// frameSize e.g. 512, sampleRate e.g. 44100
kern_return_t AudioEngineAPI__setSelfAsIOThread( UInt32 noFramesInBuffer, Float64 sampleRate )
{
	kern_return_t kernResult = 0;
	thread_time_constraint_policy_data_t thePolicy = {0};
	
	UInt64 period = 0;
	UInt32 quanta = 0;
	AudioEngineAPI__calculateIOThreadTimeConstraints( period, quanta, noFramesInBuffer, sampleRate );
	
	thePolicy.period = ( UInt32 ) period;
	thePolicy.computation = quanta;
	thePolicy.constraint = ( UInt32 ) period;
	thePolicy.preemptible = true;
	kernResult = thread_policy_set( mach_thread_self(), 
								    THREAD_TIME_CONSTRAINT_POLICY, 
								    ( thread_policy_t ) &thePolicy, 
								    THREAD_TIME_CONSTRAINT_POLICY_COUNT );
	if( kernResult != KERN_SUCCESS )
	{
		printf( "thread_policy_set failed, kernResult: %x\n", kernResult );
		
	} // if
	return( kernResult );
	
} // AudioEngineAPI__setSelfAsIOThread( func )



kern_return_t AudioEngineAPI__sleepTillIOCycle( UInt64 anchor, UInt64 period, UInt64 ioCycle )
{
	kern_return_t kernResult = KERN_FAILURE;
	UInt64 wakeup = anchor + ( period * ( ioCycle + 1 ) );
	UInt64 now = CAHostTimeBase::GetTheCurrentTime();
	
	if( now < wakeup )
	{
		// sleep till wakupTime (its in ticks)
		kernResult = mach_wait_until( wakeup );
		
	}
	else
	{
		printf( "overload, ioCycle: %u\n", ioCycle );
		
	} // if
	return( kernResult );
	
} // AudioEngineAPI__sleepTillIOCycle( func )



void callNN( void *refcon, io_service_t service, uint32_t messageType, void *messageArgument )
{
	printf( "callNN\n" );
}



void test( AudioEngineInterface *dst )
{
    kern_return_t kernResult = 0;
	io_string_t dstPath = {0};
	
	IORegistryEntryGetPath( dst->audioEngine, kIOServicePlane, dstPath );
	printf( "playing, using: %s\n", dstPath );
	
	// Instantiate a connection to the user client.
	kernResult = IOServiceOpen( dst->audioEngine, mach_task_self(), 0, &dst->audioEngineConnection );
	if( dst->audioEngineConnection != IO_OBJECT_NULL )
	{
		//mach_port_name_t port = 0;
		//IOAudioNotificationMessage notification = {0};
		int i = 0;
		//IONotificationPortRef portRef = 0;
		//io_object_t nn = 0;
		vm_address_t addr = 0;
		vm_size_t addrSize = 0;
		UInt32 bufID = 7;
		IOAudioEngineStatus *status = NULL;
		FILE *fp = NULL;
		UInt32 fsf = 0;
		char *storeCur = NULL;
		uint64_t ioCycleCount = 0;		
		uint64_t        start;
		uint64_t        end;
		uint64_t        elapsed;
		uint64_t        elapsedNano;
		static mach_timebase_info_data_t    sTimebaseInfo;
		
		
		AudioEngineAPI__registerClientBuffer( dst, 0, g_buf, sizeof( g_bufData ), bufID );
		
		kernResult = IOConnectMapMemory( dst->audioEngineConnection, 
										 kIOAudioStatusBuffer,
										 mach_task_self(), 
										 ( vm_address_t * ) &status, 
										 &addrSize, 
										 kIOMapAnywhere );
		printf( "IOConnectMapMemory, %x, %x, %u\n", kernResult, addr, addrSize );

/*
		kernResult = IOConnectMapMemory( dst->audioEngineConnection, 
										kIOAudioBytesInInputBuffer,  // use kIOAudioBytesInOutputBuffer for an output one
										mach_task_self(), 
										( vm_address_t * ) &addr, 
										&addrSize, 
										kIOMapAnywhere );
		printf( "IOConnectMapMemory, %x, %x, %u\n", kernResult, addr, addrSize );
*/		

		//portRef = IONotificationPortCreate( mach_task_self() );
		//printf( "IONotificationPortCreate, %x\n", portRef );

		//kernResult = IOServiceAddInterestNotification( portRef, dst->audioEngine, kIOGeneralInterest, callNN, NULL, &nn );
		//printf( "IOServiceAddInterestNotification, %x\n", kernResult );

		//kernResult = IOConnectSetNotificationPort( dst->audioEngineConnection, 0, IONotificationPortGetMachPort( portRef ), 0 );
		//printf( "IOConnectSetNotificationPort, kernResult: %x\n", kernResult );
		
		
		AudioEngineAPI__setSelfAsIOThread( kBufferSizeInFrames, 44100 );		
		AudioEngineAPI__start( dst );
		
		// anchor time
		UInt64 anchor = CAHostTimeBase::GetTheCurrentTime();

		// this sends back notification - returns a IOAudioNotificationMessage
//		for( i = 0; i < 2; i++ )
		{
//			mach_msg_overwrite( NULL, MACH_RCV_MSG, 0, sizeof( notification ), IONotificationPortGetMachPort( portRef ), MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL, ( mach_msg_header_t * ) &notification, sizeof( notification ) - sizeof( mach_msg_header_t ) );
//			printf("Data received: %u\n", i );
		}		

/*		
		struct {
            mach_msg_header_t	msgHdr;
            OSNotificationHeader	notifyHeader;
            mach_msg_trailer_t	trailer;
		} msg;
		kernResult = mach_msg(&msg.msgHdr, MACH_RCV_MSG, 0, sizeof(msg), IONotificationPortGetMachPort( portRef ), 0, MACH_PORT_NULL);
		printf( "mach_msg: %x\n", kernResult );
*/	
		// P1: UInt32 firstSampleFrame	= which sample frame to start overwriting
		// P2: UInt32 loopCount			= 
		// P3: bool   inputIO			= true -> record into buffer, false -> read from buffer and play
		// P4: UInt32 bufferSetID		= the buffer set to use
		// P5: UInt32 sampleIntervalHi	=
		// P6: UInt32 sampleIntervalLo	= 


/*		sleep( 1 );
		kernResult = IOConnectTrap6( dst->audioEngineConnection, kIOAudioEngineTrapPerformClientIO, 0, 0, true, bufID, 0, 0 );
		printf( "IOCOnnectTrap6, kernResult: %x, %f\n", kernResult, checkbuf() );
		
		sleep( 1 );
		kernResult = IOConnectTrap6( dst->audioEngineConnection, kIOAudioEngineTrapPerformClientIO, 0, 0, true, bufID, 0, 0 );
		printf( "IOCOnnectTrap6, kernResult: %x, %f\n", kernResult, checkbuf() );
		
		sleep( 1 );
		kernResult = IOConnectTrap6( dst->audioEngineConnection, kIOAudioEngineTrapPerformClientIO, 0, 0, true, bufID, 0, 0 );
		printf( "IOCOnnectTrap6, kernResult: %x, %f\n", kernResult, checkbuf() );
*/
		
		UInt64 idealPeriod = 0;
		UInt32 idealQuanta = 0;
		AudioEngineAPI__calculateIOThreadTimeConstraints( idealPeriod, idealQuanta, kBufferSizeInFrames, 44100 );

		fprintf( stderr, "--------\n" );
		storeCur = g_bufDataStore;
				
		fsf = 0; 
		g_buf->fActualDataByteSize = sizeof( g_bufData ) - sizeof( IOAudioBufferDataDescriptor ) + 1;
		g_buf->fActualNumSampleFrames = kBufferSizeInFrames; // read x frames ?
		g_buf->fTotalDataByteSize = sizeof( g_bufData ) - sizeof( IOAudioBufferDataDescriptor ) + 1;
		g_buf->fNominalDataByteSize = sizeof( g_bufData ) - sizeof( IOAudioBufferDataDescriptor ) + 1; // not used
		
		for( ioCycleCount = 0; ioCycleCount < 860; ioCycleCount++ )
		{
			if( AudioEngineAPI__sleepTillIOCycle( anchor, idealPeriod, ioCycleCount ) == KERN_SUCCESS )
			{
			
/*			printf( "Status:\n" );	
			printf( "             fVersion: %u\n", status->fVersion );
			printf( "    fCurrentLoopCount: %u\n", status->fCurrentLoopCount );
			printf( "        fLastLoopTime: %u64\n", status->fLastLoopTime );
			printf( "fEraseHeadSampleFrame: %u\n", status->fEraseHeadSampleFrame );
*/				
		
/*			g_buf->fActualDataByteSize = sizeof( g_bufData ) - sizeof( IOAudioBufferDataDescriptor ) - 1;
			g_buf->fActualNumSampleFrames = 256; // read x frames ?
			g_buf->fTotalDataByteSize = sizeof( g_bufData );
			g_buf->fNominalDataByteSize = 0; // not used
*/
			kernResult = IOConnectTrap6( dst->audioEngineConnection, kIOAudioEngineTrapPerformClientIO, fsf, 0, true, bufID, 0, 0 );
		
			memcpy( storeCur, g_buf->fData, g_buf->fActualDataByteSize );
			fsf += g_buf->fActualNumSampleFrames;
			storeCur += g_buf->fActualDataByteSize;   
				
			if( fsf >= kSampleFramesPerEnginBuffer )
			{
				fsf = 0;				
				
			} // if
				
/*			printf( "    Data:\n" );
			printf( "       fActualDataByteSize: %u\n", g_buf->fActualDataByteSize );
			printf( "    fActualNumSampleFrames: %u\n", g_buf->fActualNumSampleFrames );
			printf( "        fTotalDataByteSize: %u\n", g_buf->fTotalDataByteSize );
			printf( "      fNominalDataByteSize: %u\n", g_buf->fNominalDataByteSize );
*/
			} // if	
		}
		end = mach_absolute_time();
		fprintf( stderr, "-------- %llu, %llu\n", end, start );
		
		
		AudioEngineAPI__stop( dst );
		

		elapsed = end - anchor;
		if ( sTimebaseInfo.denom == 0 ) {
			(void) mach_timebase_info(&sTimebaseInfo);
		}
		printf( "total loop time: %f - %llu\n", ( float ) elapsed / ( float ) 1000000000, elapsed * sTimebaseInfo.numer / sTimebaseInfo.denom );
		printf( "time: %f\n", ( float ) CAHostTimeBase::ConvertToNanos( elapsed ) / ( float ) 1000000000 );
		
		
		// Do the maths.  We hope that the multiplication doesn't
		// overflow; the price you pay for working in fixed point.	
		
		
		fp = fopen( "rawaudui.pcm", "wb" );	
		fwrite( g_bufDataStore, sizeof( g_bufDataStore ), 1, fp );
		fclose( fp );
		

		//kernResult = IOConnectSetNotificationPort( dst->audioEngineConnection, kIOAudioEngineAllNotifications, MACH_PORT_NULL, 0 );
		//printf( "IOConnectSetNotificationPort, kernResult: %x\n", kernResult );

		
		AudioEngineAPI__unregisterClientBuffer( dst, g_buf, bufID );
		
		kernResult = IOConnectUnmapMemory( dst->audioEngineConnection, kIOAudioStatusBuffer, mach_task_self(), ( vm_address_t )&g_buf ); 
		printf( "IOConnectUnmapMemory, %x\n", kernResult );
		//IONotificationPortDestroy( portRef );
										  
		IOServiceClose( dst->audioEngineConnection );
	
	}
	else
	{
		printf( "Unable to open dst service: %x\n", kernResult );		
	}
}




int main (int argc, const char * argv[])
{
    kern_return_t kernResult = 0; 
    AudioEngineInterface audioEngine[ 32 ] = {0};
	AudioEngineInterface *cur = NULL;
    io_iterator_t iterator = 0;
	
	
    // Repeat the test on any instances of the Mac OS X 10.4 version of the driver.
    kernResult = IOServiceGetMatchingServices( kIOMasterPortDefault, IOServiceMatching( kIOAudioEngineClassName ), &iterator );
    if( kernResult != KERN_SUCCESS ) 
	{
        printf( "IOServiceGetMatchingServices failed: %x\n", kernResult );
        return -1;
		
    } // if
	
	cur = audioEngine;
    while( ( cur->audioEngine = IOIteratorNext( iterator ) ) != IO_OBJECT_NULL ) 
	{
		io_iterator_t childIterator = 0;
		
		io_string_t path = {0};
		CFStringRef id = NULL;
		CFStringRef desc = NULL;
		IORegistryEntryGetPath( cur->audioEngine, kIOServicePlane, path );
		
		id = ( CFStringRef ) IORegistryEntryCreateCFProperty( cur->audioEngine, CFSTR( kIOAudioEngineGlobalUniqueIDKey ), kCFAllocatorDefault, 0 );
		desc = ( CFStringRef ) IORegistryEntryCreateCFProperty( cur->audioEngine, CFSTR( kIOAudioEngineDescriptionKey ), kCFAllocatorDefault, 0 );
		
		printf( "Found a device( %u ): %s\n", ( cur - audioEngine ), path );
		printf( "                  ID: %s\n", id ? CFStringGetCStringPtr( id, kCFStringEncodingMacRoman ): "(null)" );
		printf( "         description: %s\n", desc ? CFStringGetCStringPtr( desc, kCFStringEncodingMacRoman ): "(null)" );
		
		kernResult = IORegistryEntryGetChildIterator( cur->audioEngine, kIOServicePlane, &childIterator );
		if( kernResult == KERN_SUCCESS )
		{
			io_service_t *curStream = cur->audioStreams;
			while( ( *curStream = IOIteratorNext( childIterator ) ) != IO_OBJECT_NULL ) 
			{
				io_name_t clsName = {0};
				IOObjectGetClass( *curStream, clsName );
				if(    ( strcmp( clsName, kIOAudioStreamClassName ) == 0 )
					|| ( strcmp( clsName, "AppleUSBAudioStream" ) == 0 ) )
				{
					printf( "    Found a stream: %s\n", clsName );
					curStream++;
					
				}
				else
				{
					printf( "    - not a stream: %s\n", clsName );
					*curStream = 0;
					
				} // if				
			} // while
			IOObjectRelease( childIterator );
			
		} // if
		cur++;
		
	} // while
    IOObjectRelease( iterator );
	

	test( &audioEngine[ 1 ] );
	return( 0 );
}

