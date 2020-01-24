/** @file pa_devs.c
	@ingroup examples_src
    @brief List available devices, including device information.
	@author Danny Boisvert

    @note Original author: Phil Burk http://www.softsynth.com
		This is a port of "pa_devs.c" to print as a JSON object
*/
/*
 * $Id$
 *
 * This program uses the PortAudio Portable Audio Library.
 * For more information see: http://www.portaudio.com
 * Copyright (c) 1999-2000 Ross Bencina and Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire PortAudio license; however, 
 * the PortAudio community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also 
 * requested that these non-binding requests be included along with the 
 * license above.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include "portaudio.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#ifdef WIN32
#include <windows.h>

#if PA_USE_ASIO
#include "pa_asio.h"
#endif
#endif

/*******************************************************************/
static rapidjson::Value PrintSupportedStandardSampleRates(
        const PaStreamParameters *inputParameters,
        const PaStreamParameters *outputParameters,
		rapidjson::Document::AllocatorType& jsonDocAlloc
		
)
{
    static double standardSampleRates[] = {
        8000.0, 9600.0, 11025.0, 12000.0, 16000.0, 22050.0, 24000.0, 32000.0,
        44100.0, 48000.0, 88200.0, 96000.0, 192000.0, -1 /* negative terminated  list */
    };
    int     i, printCount;
	PaError err;
	rapidjson::Value jsonArray(rapidjson::kArrayType);

    printCount = 0;
    for( i=0; standardSampleRates[i] > 0; i++ )
    {
        err = Pa_IsFormatSupported( inputParameters, outputParameters, standardSampleRates[i] );
        if( err == paFormatIsSupported )
        {
			jsonArray.PushBack(rapidjson::Value().SetDouble(standardSampleRates[i]), jsonDocAlloc);	
        }
	
    }
	return jsonArray;
}

/*******************************************************************/
int main(void);
int main(void)
{
    int     i, numDevices, defaultDisplayed;
    const   PaDeviceInfo *deviceInfo;
    PaStreamParameters inputParameters, outputParameters;
    PaError err;

	rapidjson::Document jsonDoc;
	rapidjson::Document::AllocatorType& jsonDocAlloc = jsonDoc.GetAllocator();
	rapidjson::Value jsonArray(rapidjson::kArrayType);
	jsonDoc.SetObject();
	
    err = Pa_Initialize();
    if( err != paNoError )
    {
        printf( "ERROR: Pa_Initialize returned 0x%x\n", err );
		Pa_Terminate();
		fprintf( stderr, "Error number: %d\n", err );
		fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
		exit( err );
    }
    
    numDevices = Pa_GetDeviceCount();
    if( numDevices < 0 )
    {
        printf( "ERROR: Pa_GetDeviceCount returned 0x%x\n", numDevices );
        err = numDevices;
		Pa_Terminate();
		fprintf( stderr, "Error number: %d\n", err );
		fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
		exit( err );
    }
    
	jsonDoc.AddMember("devicesCount", rapidjson::Value().SetInt(Pa_GetDeviceCount()), jsonDocAlloc);
	jsonDoc.AddMember("portaudioVersion", rapidjson::Value().SetString(std::string(Pa_GetVersionInfo()->versionText).c_str(), std::string(Pa_GetVersionInfo()->versionText).length()), jsonDocAlloc);
    for( i=0; i<numDevices; i++ )
    {
		deviceInfo = Pa_GetDeviceInfo( i );
        rapidjson::Value jsonObject(rapidjson::kObjectType);
		jsonObject.SetObject();
        jsonObject.AddMember("id", rapidjson::Value().SetInt(i), jsonDocAlloc);
		
		
    /* Mark global and API specific default devices */
        if( i == Pa_GetDefaultInputDevice() )
        {
            //printf( "[ Default Input" );
            //defaultDisplayed = 1;
			jsonObject.AddMember("type", rapidjson::Value().SetString("input"), jsonDocAlloc);
			jsonObject.AddMember("hostName", rapidjson::Value().SetString("Default"), jsonDocAlloc);
        }
        else if( i == Pa_GetHostApiInfo( deviceInfo->hostApi )->defaultInputDevice )
        {
            const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo( deviceInfo->hostApi );
			jsonObject.AddMember("type", rapidjson::Value().SetString("input"), jsonDocAlloc);
			jsonObject.AddMember("hostName", rapidjson::Value().SetString(std::string(hostInfo->name).c_str(), std::string(hostInfo->name).length(), jsonDocAlloc), jsonDocAlloc);
        }
        
        if( i == Pa_GetDefaultOutputDevice() )
        {
            jsonObject.AddMember("type", rapidjson::Value().SetString("output"), jsonDocAlloc);
			jsonObject.AddMember("hostName", rapidjson::Value().SetString("Default"), jsonDocAlloc);
        }
        else if( i == Pa_GetHostApiInfo( deviceInfo->hostApi )->defaultOutputDevice )
        {
            const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo( deviceInfo->hostApi );
			jsonObject.AddMember("type", rapidjson::Value().SetString("output"), jsonDocAlloc);
			jsonObject.AddMember("hostName", rapidjson::Value().SetString(std::string(hostInfo->name).c_str(), std::string(hostInfo->name).length(), jsonDocAlloc), jsonDocAlloc);
        }

    /* print device info fields */
#ifdef WIN32
        {   /* Use wide char on windows, so we can show UTF-8 encoded device names */
            wchar_t wideName[MAX_PATH];
            MultiByteToWideChar(CP_UTF8, 0, deviceInfo->name, -1, wideName, MAX_PATH-1);
			std::wstring ws( wideName );
			std::string strWideName( ws.begin(), ws.end() );
			jsonObject.AddMember("name", rapidjson::Value().SetString(strWideName.c_str(), strWideName.length(), jsonDocAlloc), jsonDocAlloc);
        }
#else
        jsonObject.AddMember("name", rapidjson::Value().SetString(std::string(deviceInfo->name).c_str(), std::string(deviceInfo->name).length(), jsonDocAlloc), jsonDocAlloc);
#endif
        jsonObject.AddMember("hostApi", rapidjson::Value().SetString(std::string(Pa_GetHostApiInfo( deviceInfo->hostApi )->name).c_str(), std::string(Pa_GetHostApiInfo( deviceInfo->hostApi )->name).length(), jsonDocAlloc), jsonDocAlloc);
        jsonObject.AddMember("maxInputs", rapidjson::Value().SetInt(deviceInfo->maxInputChannels), jsonDocAlloc);
		jsonObject.AddMember("maxOutputs", rapidjson::Value().SetInt(deviceInfo->maxOutputChannels), jsonDocAlloc);
		
		jsonObject.AddMember("defaultLowInputLatency", rapidjson::Value().SetDouble(deviceInfo->defaultLowInputLatency), jsonDocAlloc);
		jsonObject.AddMember("defaultLowOutputLatency", rapidjson::Value().SetDouble(deviceInfo->defaultLowOutputLatency), jsonDocAlloc);
		jsonObject.AddMember("defaultHighInputLatency", rapidjson::Value().SetDouble(deviceInfo->defaultHighInputLatency), jsonDocAlloc);
		jsonObject.AddMember("defaultHighOutputLatency", rapidjson::Value().SetDouble(deviceInfo->defaultHighOutputLatency), jsonDocAlloc);

#ifdef WIN32
#if PA_USE_ASIO
/* ASIO specific latency information */
        if( Pa_GetHostApiInfo( deviceInfo->hostApi )->type == paASIO ){
            long minLatency, maxLatency, preferredLatency, granularity;

            err = PaAsio_GetAvailableLatencyValues( i,
		            &minLatency, &maxLatency, &preferredLatency, &granularity );
			jsonObject.AddMember("isAsio", rapidjson::Value().SetBool(true), jsonDocAlloc);
			jsonObject.AddMember("asioMinBufferSize", rapidjson::Value().SetDouble(minLatency), jsonDocAlloc);
			jsonObject.AddMember("asioMaxBufferSize", rapidjson::Value().SetDouble(maxLatency), jsonDocAlloc);
			jsonObject.AddMember("asioPreferredBufferSize", rapidjson::Value().SetDouble(preferredLatency), jsonDocAlloc);

            if( granularity == -1 )
				jsonObject.AddMember("asioBufferGranularity", rapidjson::Value().SetDouble(granularity), jsonDocAlloc);
            else
				jsonObject.AddMember("asioBufferGranularity", rapidjson::Value().SetDouble(granularity), jsonDocAlloc);
		}
#else
	jsonObject.AddMember("isAsio", rapidjson::Value().SetBool(false), jsonDocAlloc);
#endif /* PA_USE_ASIO */
#endif /* WIN32 */

        jsonObject.AddMember("defaultSampleRate", rapidjson::Value().SetDouble(deviceInfo->defaultSampleRate), jsonDocAlloc);

    /* poll for standard sample rates */
        inputParameters.device = i;
        inputParameters.channelCount = deviceInfo->maxInputChannels;
        inputParameters.sampleFormat = paInt16;
        inputParameters.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
        inputParameters.hostApiSpecificStreamInfo = NULL;
        
        outputParameters.device = i;
        outputParameters.channelCount = deviceInfo->maxOutputChannels;
        outputParameters.sampleFormat = paInt16;
        outputParameters.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
        outputParameters.hostApiSpecificStreamInfo = NULL;

        if( inputParameters.channelCount > 0 )
        {
            // Supported standard sample rates for half-duplex 16 bit
			jsonObject.AddMember("supportedSampleRates", PrintSupportedStandardSampleRates( &inputParameters, NULL, jsonDocAlloc ), jsonDocAlloc);
        }

        if( outputParameters.channelCount > 0 )
        {
            // Supported standard sample rates for half-duplex 16 bit
			jsonObject.AddMember("supportedSampleRates", PrintSupportedStandardSampleRates( NULL, &outputParameters, jsonDocAlloc ), jsonDocAlloc);
        }

        if( inputParameters.channelCount > 0 && outputParameters.channelCount > 0 )
        {
            // Supported standard sample rates for half-duplex 16 bit
			jsonObject.AddMember("supportedSampleRates", PrintSupportedStandardSampleRates( &inputParameters, &outputParameters, jsonDocAlloc ), jsonDocAlloc);
        }
		
		jsonArray.PushBack(jsonObject, jsonDocAlloc);
		
    }
	
	jsonDoc.AddMember("devices", jsonArray, jsonDocAlloc);
	
	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	jsonDoc.Accept(writer);
	std::cout << buffer.GetString() << std::endl;
	
    Pa_Terminate();
    return 0;
}
