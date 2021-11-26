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
#include <infokruncher.h>
#include <process.h>
#include <iostream>
#include <algorithm>

using namespace std;
#include <infostream.h>
#include <infossock.h>
#include <infosock.h>

using namespace KruncherTools;

namespace  InfoKruncher
{
	// SERVERS
	template <>
		void ThreadQueue<ServiceBase>::operator()() { callback( *this ); }


	void* Serve(void* lk)
	{
		ThreadQueue<ServiceBase>& Q(*static_cast<ThreadQueue<ServiceBase>*>(lk));
		Q();
		return NULL;
	}

	template <class SocketType>
		struct Server 
	{
		void serve(SocketType& ss, ServiceBase& base, const SocketProcessOptions& options )
		{
			ss.blocking(true);
			KruncherMimes::SocketReadWriter< SocketType, 512 > SocketComm( ss );
			const in_addr& ipaddr( ss );
			base.ServeRequest( SocketComm, ipaddr, options );
		}
	};



	void ServiceBase::ServeHttps( int sock, const SocketProcessOptions& options )
	{
		SecureInformation::SslContextInfo ctxinfo
			( options.cafile, options.cadir, options.certfile, options.keypasswd, options.keyfile );

		SSL_CTX* ctx( SecureInformation::create_context( ) );
		if ( ! ctx ) return;

		STACK_OF(X509_NAME) * x509names( SecureInformation::configure_context( ctx, ctxinfo ) ); 
		if ( x509names == NULL )
		{
			Log("Cannot configure ssl context");
			SSL_CTX_free( ctx );
			return;
		}

		SSL* ssl( SSL_new( ctx ) );

		SSL_set_fd( ssl, sock );
			
		const int accepted( SSL_accept( ssl ) );
		if ( accepted <= 0 )
		{
			SSL_free( ssl );
			return;
		}

		SecureInformation::Socket ss( sock, ssl, KruncherTools::GetUuid() );
		SecureInformation::SslContext sslcontext( ctx, ssl );
		ss.timeout( options.timeout.first, options.timeout.second );
		Server<SecureInformation::Socket> server;
		server.serve( ss, *this, options );
	}

	void ServiceBase::ServeHttp( int sock, const SocketProcessOptions& options )
	{
		PlainInformation::Socket ss( sock, KruncherTools::GetUuid() );
		ss.timeout( options.timeout.first, options.timeout.second );
		Server<PlainInformation::Socket> service;
		service.serve( ss, *this, options );
	}


	void ServiceBase::operator()( InfoKruncher::ThreadQueue<ServiceBase>& q ) 
	{
		const SocketProcessOptions& options( q );
		stringstream ssexcept;
		try
		{
			Locker& lock( q );
			while (!TERMINATE)
			{
				{const int T((rand()%1000)+100000); usleep(T); }
				int sock(0);
				{
					ThreadWidget locker(lock);
					sock=q;
				}

				if (!sock) continue;

				if ( options.scheme == https )
					ServeHttps( sock, options );

				if ( options.scheme == http )
					ServeHttp( sock, options );
			}
		}
		catch (const char* e) {ssexcept<<e;}
		catch (const string& e) {ssexcept<<e;}
		catch (const exception& e) {ssexcept<<e.what();}
		catch (...) {ssexcept<<"unknown exception";}
		if (!ssexcept.str().empty())
		{
			const string schemer( ( options.scheme == https ) ? "https" : "http" );
			const string what( string("ServiceBase::operator(")  + schemer + string( ")" ) );
			stringstream ssout; ssout << fence << "[EXCEPT]" << fence << ssexcept.str(); Log(VERB_ALWAYS, what, ssout.str());
		}
	}


	void ServiceBase::InitializeService( streamingsocket& ssrvc )
	{
		const SocketProcessOptions& Options( Q );
		if (!ssrvc.open()) throw "Cannot open";
		ssrvc.blocking( true );
		ssrvc.timeout( Options.timeout.first, Options.timeout.second );
		if (!ssrvc.listen()) throw "Cannot listen";
	}

	void ServiceBase::SpawnService( streamingsocket& ssrvc )
	{
		const SocketProcessOptions& Options( Q );
		Locker& lock( Q );
		while ( (size_t) subprocesses.size() < (size_t) Options.forks )
		{
			const pid_t newfork( fork() );
			if ( newfork == 0 )
			{
				ThreadFunction f( Serve );
				Threader threader( Options.threads, f, &Q );
				while ( !TERMINATE )
				{
					const int incoming(ssrvc.accept());
					ThreadWidget locker(lock);
					Q+=incoming;
					Throttle( Options );
				}

			}  else {
				subprocesses+=newfork;
			}
		} 

	}

	void ServiceBase::SpawnService( const SocketProcessOptions& options )
	{
		streamingsocket ssrvc("", options.port,KruncherTools::GetUuid() );
		InitializeService( ssrvc );
		SpawnService( ssrvc );
	}

	void ServiceBase::RunService ( const SocketProcessOptions& options )
	{
		Q=options;
		const SocketProcessOptions& Options( Q );
		SpawnService( Options ); 
	}

} // InfoKruncher





