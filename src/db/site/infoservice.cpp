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


namespace InfoKruncher
{
	const string ServiceName( "InformationKruncher" );

	void InfoSite::LoadResponse( InfoKruncher::Responder& r, InfoKruncher::RestResponse& Responder )
	{
		Log( VERB_ALWAYS, "InfoSite->Entry", r.options.datapath );
		DbRecords::RecordSet<InfoDataService::Visitor> records( r.options.datapath );
		records=r.options.datapath;
		records+=r;


		InfoDataService::DataResource Payload( r, records );
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

		InfoDb::Site::Roles roles( r.options.scheme, Payload.uri, r.headers, r.ipaddr, r.options.text );	
		InfoAuth::Authorization auth( Payload.payload.str(), Payload.contenttype, roles );
		const int AuthorizationStatus( auth );

		Responder( AuthorizationStatus, Payload.contenttype, ServiceName, records.IsNewCookie(), records.CookieName(), records.Cookie(), auth );
		Log( VERB_ALWAYS, "InfoSite::LoadPayload", Payload.uri );
		return ;
	}

	bool InfoSite::ProcessForm( const string formpath, stringmap& formdata )
	{
		stringstream ssmsg;  ssmsg << "InfoSite::ProcessForm" << fence << formpath << fence << formdata;
		Log( VERB_ALWAYS, "InfoSite::ProcessForm", ssmsg.str() );
		return true;
	}

	void InfoSite::PostProcessing( InfoKruncher::Responder&, InfoKruncher::RestResponse& DefaultResponse, const binarystring& PostedContent ) 
	{
		Log( VERB_ALWAYS, "InfoSite::PostProcessing", (char*) PostedContent.data() );
		return;
		
		stringmap formdata;
		PostProcessingXml::PostedXml xml( formdata, *this );
		xml.Load( (char*)PostedContent.c_str() );
		if ( ! xml ) Log( "InfoSite::PostProcessing", "Form processing failed" );
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
