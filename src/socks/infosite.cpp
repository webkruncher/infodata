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


#include <iostream>
#include <sstream>
#include <infokruncher.h>
#include <infosite.h>

using namespace std;

#include <infossock.h>
#include <infosock.h>
#include <infotools.h>

#include <process.h>

using namespace KruncherTools;
using namespace Hyper;

namespace InfoKruncher
{
	void Site::GetPage
	(
		KruncherMimes::SocketManager& sock,
		const Hyper::MimeHeaders& headers, 
		const in_addr& ipaddr,
		const SocketProcessOptions& options
	)
	{
		const string schemer( (  options.scheme == InfoKruncher::http  ) ? "http" : "https" );
		const size_t ContentLength ( headers.ContentLength() );
		const string resource( headers.Resource() );
		const string method( headers.Method() );

		if ( method.empty() ) return;
		if ( resource.empty() ) return;
		//cerr << "Method:" << method << ", Resource:" << resource << ";" << endl;
		//cerr << headers << endl << endl;
		

		Responder respond( method, resource, headers, ipaddr, ContentLength, options );
		InfoKruncher::RestResponse response( respond.response );
		LoadResponse( respond, response );


		string postprocessed;
		if ( ( response.Status() == 200 ) && ( ( method == "POST" ) || ( method == "PUT" ) || ( method == "PATCH" ) ) )
		{
			stringstream sspost;
			if ( ContentLength )
			{
				const binarystring& posted( sock.Payload( ContentLength ) );
				PostProcessing( respond, response, posted );
			}
		} 

		sock.flush();
		response >> sock;
		sock.flush();
	}


	void Site::ServePage( KruncherMimes::SocketManager& sock, const in_addr& ipaddr, const SocketProcessOptions& options )
	{
		if ( ! sock ) return;
		const string& headertext( sock.Headers() );
		Hyper::MimeHeaders headers( headertext );
		GetPage( sock, headers, ipaddr, options );
	}

	template < class SocketType >
		void Client::LoadRequestHeaders(  SocketType& sock, const SocketProcessOptions& options )
	{
		Requester request( options );
		LoadRequest( request ); 
		sock.write(request.ss.str().c_str(), request.ss.str().size());
		sock.flush();
	}

	template < class SocketType >
		void Client::SendRequestHeaders( SocketType& sock, const SocketProcessOptions& options )
	{
		LoadRequestHeaders< SocketType >( sock, options );
	}


	void Site::Throttle( const SocketProcessOptions& svcoptions ) 
	{
		Log(VERB_ALWAYS, "Site::Throttle()", "NO SLEEP");
	}

	void Client::Throttle( const SocketProcessOptions& svcoptions ) { }

} //namespace InfoKruncher


