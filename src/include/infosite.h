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




#ifndef WEBKRUNCHER_SITE_H
#define WEBKRUNCHER_SITE_H
#include <infotools.h>
using namespace KruncherTools;
#include <infossock.h>
#include <infosock.h>
#include <hyper.h>
#include <mimes.h>
#include <uuid.h>

namespace InfoKruncher
{
	using namespace std;

	template <class WorkerListType >
		struct Options : KruncherTools::Args
	{
		Options() {}
		Options( int _argc, char** _argv ) : 
			Args( _argc,  _argv ),
			daemonize( true )
		{}
		virtual operator bool ()
		{
			if ( ! Args::operator bool () ) return false;
			if ( find( "-d" ) != end() ) daemonize=false;
			return workerlist( *this );
		}
		WorkerListType workerlist;
		bool daemonize;
	};

	struct Responder;
	struct RestResponse
	{
		RestResponse( const Responder& _responder) : responder( _responder ), data( NULL ) {}
		virtual ~RestResponse()
		{
			if ( data )
			{
				//Log( VERB_ALWAYS, "RestResponse", "FreeData" );
				free( data );
			}
		}
		void operator()
		(
			const int _status, 
			const string _contenttype, 
			const string _servicename, 
			const bool _setcookie, 
			const string _cookiename, 
			const string _cookie, 
			unsigned char* _data ,
			const size_t _datalength
		){
			status=( _status );
			contenttype=( _contenttype );
			servicename=( _servicename );
			setcookie=( _setcookie );
			cookiename=( _cookiename );
			cookie=( _cookie );
			data=( _data );
			datalength=( _datalength );
		}
		void operator()
		(
			const int _status, 
			const string _contenttype, 
			const string _servicename, 
			const bool _setcookie, 
			const string _cookiename, 
			const string _cookie, 
			const string& _text 
		)
		{
			status=( _status );
			contenttype=( _contenttype );
			servicename=( _servicename );
			setcookie=( _setcookie );
			cookiename=( _cookiename );
			cookie=( _cookie );
			text=( _text );
		}

		int Status() const { return status; }

		operator const size_t () { return datalength; }
		operator const unsigned char* () { return data; }


		private:
		const Responder& responder;
		int status;
		string contenttype; 
		string servicename; 
		bool setcookie; 
		string cookiename; 
		string cookie; 
		string text;
		unsigned char* data;
		size_t datalength;

//		friend ostream& operator<<( ostream&, const RestResponse&);
		public:
		virtual void operator>>( KruncherMimes::SocketManager& o ) const;
	};

//	inline ostream& operator<<( ostream& o, const RestResponse& r )
//		{ return r.operator<<( o ); } 

	struct Responder
	{
		Responder
		(
			const string& _method,
			const string& _resource,
			const Hyper::MimeHeaders& _headers,
			const in_addr& _ipaddr,
			const int _ContentLength,
			const InfoKruncher::SocketProcessOptions& _options
		) 
			: 
				response( *this ),
				method( _method ),
				resource( _resource ),
				headers( _headers ),
				ipaddr( _ipaddr ),
				ContentLength( _ContentLength ),
				options( _options )
		{}
		
		RestResponse response;
		const string& method;
		const string& resource;
		const Hyper::MimeHeaders& headers;
		const in_addr& ipaddr;
		const int ContentLength;
		const InfoKruncher::SocketProcessOptions& options;
		bool IsDefault() const { return ( ( method == "GET" ) && ( resource == "/" ) ) ; }
	};

	inline void RestResponse::operator>>( KruncherMimes::SocketManager& o ) const
	{
		stringstream statusline;
		statusline << status << " " << Hyper::statusText(status);
		stringstream response;
		response << "HTTP/1.1 ";
		response << statusline.str() << Endl;
		response << "Content-Type: " << contenttype << Endl;
		response << "Server: InfoSite" << Endl;
		response << "Connection: close" << Endl;
		response << "Request:" << responder.resource << Endl;
		if ( setcookie ) response << "Set-Cookie:" << cookiename << "=" << cookie << ";" << Endl;

		if ( data )
		{
			response << "Content-Length:" << datalength << Endl;
			response << Endl;
			string s( response.str() );
			o.write( (unsigned char* )s.c_str(), s.size() );
			o.write( data, datalength );
//cout << "Response:" << s << endl;
//cout << "Data:" << data << endl;
		} else {
			response << "Content-Length:" << text.size() << Endl;
			response << Endl;
			response << text;
			string s( response.str().c_str() );
//cout << "Response:" << s << endl;
			o.write( (unsigned char* )s.c_str(), s.size() );
		}
		return ;
	}


	struct Site : ServiceBase
	{
		virtual void LoadResponse( InfoKruncher::Responder& r, InfoKruncher::RestResponse& Responder ) {}
		virtual void PostProcessing( Responder&, RestResponse& DefaultResponse, const binarystring& PostedContent ) 
		{
			cout << "ERROR->No PostProcessor" << endl;
		}
		void ForkAndServe( const SocketProcessOptions& );
		virtual bool ProcessForm( const string, stringmap& ){return false;}
		void Terminate();
		virtual void Throttle( const SocketProcessOptions& );

		void GetPage
			(
				KruncherMimes::SocketManager&,
				const Hyper::MimeHeaders&, 
				const in_addr&,
				const SocketProcessOptions&
			);

		void ServePage( KruncherMimes::SocketManager&, const in_addr& ipaddr,const SocketProcessOptions&  );
		void ServeRequest( KruncherMimes::SocketManager& ss, const in_addr& ipaddr, const SocketProcessOptions& options )
			{ ServePage(ss, ipaddr, options ); } 
	};


	struct Client : ConsumerBase
	{
		struct Requester
		{
			Requester
			(
				const InfoKruncher::SocketProcessOptions& _options
			) 
				: 
					options( _options )
			{}
			
			const InfoKruncher::SocketProcessOptions& options;
			stringstream ss;
		};
		virtual void LoadRequest( Requester& ) {}
		void ForkAndRequest( const SocketProcessOptions& );
		void Terminate();
		virtual void Throttle( const SocketProcessOptions& );



		template< class SocketType >
			void LoadRequestHeaders( SocketType&, const SocketProcessOptions& );

		template< class SocketType >
			void SendRequestHeaders( const Hyper::Request< SocketType >& , const string&, const Hyper::MimeHeaders&, SocketType&, const SocketProcessOptions& );

		template< class SocketType >
			void SendRequestHeaders( SocketType&, const SocketProcessOptions&  );

		void SendRequest( SecureInformation::Socket& ss, const SocketProcessOptions& options )
			{ SendRequestHeaders(ss, options ); }

		void SendRequest( PlainInformation::Socket& ss, const SocketProcessOptions& options )
			{ SendRequestHeaders(ss, options );  }

	};


	struct ServiceList : vector< InfoKruncher::SocketProcessOptions >
	{
		virtual bool operator ()( const KruncherTools::Args& );
		private:
		friend ostream& operator<<(ostream&,const ServiceList &);
		virtual ostream& operator<<(ostream& o) const
		{
			for (const_iterator it=begin();it!=end();it++) o << (*it) << endl;
			return o;
		}
	}; 

	inline ostream& operator<<(ostream& o,const ServiceList & m) { return m.operator<<(o); }

	struct InfoSite : InfoKruncher::Site
	{
		virtual void LoadResponse( InfoKruncher::Responder& r, InfoKruncher::RestResponse& Responder );
		virtual void PostProcessing( InfoKruncher::Responder&, InfoKruncher::RestResponse& DefaultResponse, const string& PostedContent ); 
		virtual void Throttle( const InfoKruncher::SocketProcessOptions& );
		virtual bool ProcessForm( const string, stringmap& );
	};

} //namespace InfoKruncher

#endif // WEBKRUNCHER_SITE_H


