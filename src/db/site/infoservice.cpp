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
#include <records/inforecords.h>


namespace InfoKruncher
{
	const string ServiceName( "InfoData" );
	struct InfoResource : InfoDataService::DataResource
	{
		InfoResource( const InfoKruncher::Responder& _responder, Visitors::VisitorBase& _visitor  ) 
			: DataResource( _responder,  _visitor ) {}
		operator int () { return 0; }
	};

	void InfoSite::LoadResponse( InfoKruncher::Responder& r, InfoKruncher::RestResponse& Responder )
	{
		const string ip( dotted( r.ipaddr ) );
		if ( ip != "127.0.0.1" )
		{
			cerr << red << r.ipaddr << fence << r.method << fence << r.resource << normal << endl;
			const string uauth( "UnAuthorized" );
			Responder( 401, "text/plain", ServiceName, false, "", "", uauth );
			return;
		}
		//cerr << teal << r.ipaddr << fence << r.method << fence << r.resource << normal << endl;
		DbRecords::RecordSet<InfoDataService::Visitor> records( r.options.datapath );
		records=r.options.datapath;
		records+=r;

		InfoResource Payload( r, records );
		const int payloadstatus( Payload );
		if ( payloadstatus ) 
		{
			Responder( payloadstatus, Payload.contenttype, ServiceName, false, "", "", Payload.payload.str() );
			return ;
		}

		if ( ( r.method == "POST" ) || ( r.method == "PUT" ) || ( r.method == "PATCH" ) )
			if ( ( r.ContentLength < 0 ) || ( r.ContentLength > 4096 ) )
			{
				Responder( 414, Payload.contenttype, ServiceName, false, "", "", Payload.payload.str() );
				return ;
			}


		if ( r.method == "GET" ) 
		{
			cerr << r.method << " " << r.resource << endl;
			//cerr << "Search for:" << r.resource << endl;
			stringvector qparts;
			qparts.split( r.resource, "?" );
			if ( qparts.empty() )
			{
				const string estr( "No resource specified" );
				Responder( 404, "text/plain", ServiceName, false, "", "", estr );
				return;
			}
			const string what( qparts[ 0 ] );
			const size_t qsize( qparts.size() );

			string query;
			if ( qsize > 1 ) query=qparts[ 1 ];
			stringstream ss;

			cerr << teal << "Get->DB:" << what << normal << endl;

			if ( what == "/tickerlist" )
			{
				Responder.SetChunked( 4096 );
				DbRecords::KeyLister<StockMarket::TickerBase>( ss, query.c_str(), r.options.datapath );
				Responder( 200, "text/plain", ServiceName, false, "", "", ss.str() );
				return;
			}

			if ( what == "/tickers" )
			{
				DbRecords::RecordPrinter<StockMarket::TickerBase>( ss, query.c_str(), r.options.datapath );
				Responder( 200, "text/plain", ServiceName, false, "", "", ss.str() );
				return;
			}
			{
				Responder( 404, "text/plain", ServiceName, false, "", "", what );
				return;
			}
		} 

		Responder( 200, "text/plain", ServiceName, false, "", "", "" );
	}

	bool InfoSite::ProcessForm( const string formpath, stringmap& formdata )
	{
		stringstream ssmsg;  ssmsg << "InfoSite::ProcessForm" << fence << formpath << fence << formdata;
		Log( VERB_ALWAYS, "InfoSite::ProcessForm", ssmsg.str() );
		return true;
	}

	void InfoSite::PostProcessing( InfoKruncher::Responder& respond, InfoKruncher::RestResponse& DefaultResponse, const binarystring& PostedContent ) 
	{
		string payload;
		payload.assign( (char*) PostedContent.data(), PostedContent.size() );

		stringstream get;
		get << fence << respond.method << fence << respond.resource << payload;
		cerr << teal << "Put->DB:" << get.str() << normal << endl;
		const size_t Len( get.str().size() );

		InfoMarketData::MarketData markets( respond.options );
		const string& s( get.str() );
		pair< unsigned char*,size_t > result( markets( s ) );

		DefaultResponse.SetStatus( markets );

		if ( result.second )
			DefaultResponse.Set( result.first, result.second );

	}

	void InfoSite::Throttle( const InfoKruncher::SocketProcessOptions& svcoptions )
		{ usleep( (rand()%100)+20000 ); }
} // InfoKruncher

namespace InfoDataService
{

	void ShowRecords( const KruncherTools::Args& args, const InfoKruncher::SocketProcessOptions& options )
	{
		BdbSpace::DbMetaData meta( args );
		if ( ! meta ) return ;
		cerr << "#" << fence << meta.TableName() << fence << meta.Key() << fence << endl;
		DbRecords::RecordPrinter<Visitors::VisitorData>( cout, meta.Key(), options.datapath );
	}

} // InfoDataService

