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


#include <infotools.h>
#include <infokruncher.h>
#include <infosite.h>
#include <exexml.h>
#include <infofigur.h>
#include <infotools.h>
#include <infodataservice.h>



namespace InfoKruncher
{
	struct DbSite : InfoSite
	{
		DbSite() {}
		virtual ~DbSite() { }
		bool ProcessForm( const string formpath, stringmap& formdata )
		{
			stringstream ssmsg;  ssmsg << "DbSite" << fence << formpath << fence << formdata;
			Log( ssmsg.str() );
			return true;
		}
		void PostProcessing( InfoKruncher::Responder& responder, InfoKruncher::RestResponse& DefaultResponse, const binarystring& PostedContent, InfoKruncher::ThreadLocalBase& threadlocal ) 
		{
			InfoSite::PostProcessing( responder, DefaultResponse, PostedContent, threadlocal );
		}
		private:
		ThreadLocalBase* AllocateThreadLocal( const InfoKruncher::SocketProcessOptions& options )
		{
			return InfoSite::AllocateThreadLocal( options );
		}
	};

	template<> 
		void InfoKruncher::Service< DbSite >::ForkAndServe( PROPERTIES_BASE& node, const SocketProcessOptions& svcoptions )
	{
		InfoDataService::SetupDB( svcoptions.datapath );
		RunService( node, svcoptions );
	}

	template<> void InfoKruncher::Service< DbSite >::Terminate() 
	{ 
		subprocesses.Terminate(); 
		InfoDataService::TeardownDB();
	}
} // InfoKruncher




int main( int argc, char** argv )
{
	stringstream ssexcept;
	try
	{
		InfoKruncher::ServiceName=argv[ 0 ];
		//VERBOSITY=VERB_SIGNALS|VERB_SSOCKETS;
		//VERBOSITY=VERB_CRUD;
		//VERBOSITY=VERB_SERVICE;
		//VERBOSITY=VERB_CONSOLE;
		InfoKruncher::Options< InfoKruncher::ServiceList > options( argc, argv );
		if ( ! options ) throw string( "Invalid options" );
		PROPERTIES_BASE& Cfg( options );


		
		//cerr << yellow << "krestdb is starting up" << normal << endl;
		{

			const string verbose( options.svalue( "-v", "--verbose", ""  ) );
			if ( !verbose.empty() )
			{
				stringstream sso;
				sso << "VERBOSITY:" << verbose;
				if ( verbose.find( "DBCURSOR1" ) != string::npos ) VERBOSITY|=(VERB_CURSOR_1);
				if ( verbose.find( "DBCURSOR2" ) != string::npos ) VERBOSITY|=(VERB_CURSOR_2|VERB_CURSOR_1);
				if ( verbose.find( "DBCURSOR3" ) != string::npos ) VERBOSITY|=(VERB_CURSOR_3|VERB_CURSOR_1|VERB_CURSOR_2);
				if ( verbose.find( "REST1" ) != string::npos ) VERBOSITY|=(VERB_REST_1);
				if ( verbose.find( "REST2" ) != string::npos ) VERBOSITY|=(VERB_REST_2|VERB_REST_1);
				if ( verbose.find( "REST3" ) != string::npos ) VERBOSITY|=(VERB_REST_3|VERB_REST_2|VERB_REST_1);
				if ( verbose.find( "SERVICE" ) != string::npos ) VERBOSITY|=(VERB_SERVICE);
			}
		}

		cerr << "Starting " << argv[ 0 ] << " service, verbosity " << hex << VERBOSITY << endl;



		if ( options.find( "-d" ) == options.end() ) Initialize();
		else SetSignals();

		const InfoKruncher::ServiceList& workerlist( options.workerlist );

		if ( options.find( "--check-config" ) != options.end() )
		{
			cerr << "Configuration:" << endl << workerlist << endl;
			return 0;
		}

		const size_t nSites( options.workerlist.size() );
		KruncherTools::Daemonizer daemon( options.daemonize, "DbSite" );


		InfoKruncher::Service<InfoKruncher::DbSite> sites[ nSites ];

		for ( size_t c=0;  c < nSites; c++ )
		{
			InfoKruncher::Service<InfoKruncher::DbSite>& site( sites[ c ] );
			const InfoKruncher::SocketProcessOptions& svcoptions( *workerlist[ c ] );
			site.ForkAndServe( Cfg, svcoptions );
		}

		unsigned long tick( 1 );
		while ( !TERMINATE ) 
		{ 
			usleep( 10000 ); 
			tick++;
		}
		//Log( VERB_ALWAYS, "main", "infodbservice is terminating child processes" );

		for ( size_t t=0; t < nSites; t++ ) sites[ t ].Terminate();
	}
	catch( const exception& e ) { ssexcept<<e.what(); }
	catch( const string& s ) { ssexcept<<s;}
	catch( const char* s ) { ssexcept<<s;}
	catch( ... ) { ssexcept<<"unknown";}
	if ( ! ssexcept.str().empty() ) 
	{
		cerr << red << ssexcept.str() << normal << endl;
		Log( VERB_ALWAYS, "dbmain", ssexcept.str() );
	}

	//Log( VERB_ALWAYS, InfoKruncher::ServiceName, "shutting down" );

	return 0;
}




