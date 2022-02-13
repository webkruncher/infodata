/*
 * Copyright (c) Jack M. Thompson WebKruncher.com, exexml.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the WebKruncher nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Jack M. Thompson ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Jack M. Thompson BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <infokruncher.h>
#include <infosite.h>
#include <auth/infoxmlauth.h>
#include <visitors/visitor.h>
#include <site/infodataservice.h>
#include <exexml.h>
#include <site/PostProcessor.h>
#include <datarest.h>



#include MainEntryPoint
#include RestInterface 




namespace InfoKruncher
{
	string ServiceName( "InfoData" );

	struct InfoResource : InfoDataService::DataResource
	{
		InfoResource( const InfoKruncher::Responder& _responder, Visitors::VisitorBase& _visitor  ) 
			: DataResource( _responder,  _visitor ) {}
		operator int () { return 0; }
	};

	void InfoSite::LoadResponse( InfoKruncher::Responder& r, InfoKruncher::RestResponse& response, InfoKruncher::ThreadLocalBase& threadlocal )
	{
		//Log( VERB_ALWAYS, ServiceName, r.resource );
		//InfoDataService::SetupDB( r.options.datapath );
		const string ip( dotted( r.ipaddr ) );
		if ( ip != "127.0.0.1" )
		{
			//cerr << red << r.ipaddr << fence << r.method << fence << r.resource << normal << endl;
			const string uauth( "UnAuthorized" );
			response( 401, "text/plain", ServiceName, false, "", "", uauth );
			return;
		}
		if ( r.resource == "/exit" )
		{
			cerr << red << "Exiting" << normal << endl;
			Log( VERB_ALWAYS, "infoservice", "Raising signal" );
			response( 200, "text/plain", ServiceName, false, "", "", "Server is exiting" );
			kill( 0, SIGUSR1 );
			return;
		}
		//cerr << teal << r.ipaddr << fence << r.method << fence << r.resource << endl << r.headers << normal << endl;
		DbRecords::RecordSet<InfoDataService::Visitor> records( r.options.datapath );

		records+=r;

		InfoResource Payload( r, records );
		const int payloadstatus( Payload );
		if ( payloadstatus ) 
		{
			response( payloadstatus, Payload.contenttype, ServiceName, false, "", "", Payload.payload.str() );
			return ;
		}

		if ( ( r.method == "POST" ) || ( r.method == "PUT" ) || ( r.method == "PATCH" ) )
			if ( ( r.ContentLength < 0 ) || ( r.ContentLength > 4096 ) )
			{
				response( 414, Payload.contenttype, ServiceName, false, "", "", Payload.payload.str() );
				return ;
			}

		//cerr << r.resource << endl;
		if ( r.method == "GET" ) KruncherSpace::LoadResponse( r, ServiceName, response, threadlocal );
		else response( 200, "text/plain", ServiceName, false, "", "", "" );
	}

	bool InfoSite::ProcessForm( const string formpath, stringmap& formdata )
	{
		stringstream ssmsg;  ssmsg << "InfoSite::ProcessForm" << fence << formpath << fence << formdata;
		Log( VERB_ALWAYS, "InfoSite::ProcessForm", ssmsg.str() );
		return true;
	}

	void InfoSite::PostProcessing
		( InfoKruncher::Responder& respond, InfoKruncher::RestResponse& response, const binarystring& PostedContent, InfoKruncher::ThreadLocalBase& threadlocal ) 
			{ KruncherSpace::PostProcessing( respond, ServiceName, response, PostedContent, threadlocal ); }

	void InfoSite::Throttle( const InfoKruncher::SocketProcessOptions& svcoptions )
		{ usleep( (rand()%100)+20000 ); }

	ThreadLocalBase* InfoSite::AllocateThreadLocal( const InfoKruncher::SocketProcessOptions& options )
	{ 
		return KruncherSpace::AllocateThreadLocal( options ); 
	}


} // InfoKruncher

namespace InfoDataService
{
	void SetupDB( const string datapath )
	{
		KruncherSpace::Allocate( datapath );
	}

	void TeardownDB()
	{
		KruncherSpace::Release();
	}
} // InfoDataService

