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
#include <algorithm>


using namespace std;
#include <infostream.h>
#include <infossock.h>
#include <infosock.h>
#include <mimes.h>


using namespace KruncherTools;

namespace  InfoKruncher
{
	template <>
		void ThreadQueue<ConsumerBase>::operator()() { callback( *this ); }

	void ConsumerBase::operator()( InfoKruncher::ThreadQueue<ConsumerBase>& q ) 
	{
		const SocketProcessOptions& options( q );
		stringstream ssexcept;
		try
		{
			Locker& lock( q );
			while ( !TERMINATE )
			{
				{const int T((rand()%1000)+100000); usleep(T); }
				int sock(0);
				{
					ThreadWidget locker(lock);
					sock=q;
				}

				if (!sock) continue;
				if ( options.scheme == https )
					GetHttps( sock, options );

				if ( options.scheme == http )
					GetHttp( sock, options );

			}
		}
		catch (const char* e) {ssexcept<<e;}
		catch (const string& e) {ssexcept<<e;}
		catch (const exception& e) {ssexcept<<e.what();}
		catch (...) {ssexcept<<"unknown exception";}
		if (!ssexcept.str().empty())
		{
			cerr << red << ssexcept.str()<<endl;
			stringstream ssout; ssout << fence << "[EXCEPT]" << fence << ssexcept.str(); Log(VERB_ALWAYS, "InfoConsumer", ssout.str());
		}
	}

	void* Get(void* lk)
	{
		ThreadQueue<ConsumerBase>& Q(*(ThreadQueue<ConsumerBase>*)lk);
		Q();
		return NULL;
	}

		void RequestBase::secure( int sock, ConsumerBase& base, const SocketProcessOptions& options )
		{
			SSL_CTX *ctx( SecureInformation::InitSslClient() );

			if ( (!options.cafile.empty()) && (!options.cadir.empty()) )
			{
				if (SSL_CTX_load_verify_locations(ctx, options.cafile.c_str(), options.cadir.c_str() ) <1 ) {
					printf("Error setting the verify locations.\n");
					return ;
				} 
			}



			SSL *ssl( SSL_new( ctx ) );
			if ( ssl == NULL ) return;
			SecureInformation::Socket ss( sock, ssl );


			ss.blocking( true );
			ss.timeout( options.timeout.first, options.timeout.second );



			SecureInformation::SslContext cleanup( ctx, ssl );
			if ( SSL_is_server( ssl ) != 0 )
			{ 
				Log( "Consumer::Request", "SSL is in server mode and cannot continue as a client" );
				return;
			}
			
			SSL_set_fd( ssl, ss.GetSock() );
			const int status( SSL_connect( ssl ) );
			if ( status != 1 )
			{
				Log( "Consumer::Request", "Cannot connect" );
				return;
			}

			const string Info( SecureInformation::CertInfo(ssl) );

			base.SendRequest( ss, options );
			KruncherMimes::SocketReadWriter< SecureInformation::Socket, 1024 > SocketComm( ss );
			base.Consume( SocketComm, options );
		}


		void RequestBase::plain( int sock, ConsumerBase& base, const SocketProcessOptions& options )
		{
			PlainInformation::Socket ss( sock, KruncherTools::GetUuid() );
			ss.blocking(true);
			ss.timeout( options.timeout.first, options.timeout.second );
			base.SendRequest( ss, options );
			KruncherMimes::SocketReadWriter< PlainInformation::Socket, 512 > SocketComm( ss );
			base.Consume( SocketComm, options );
		}



	void ConsumerBase::GetHttps( int sock, const SocketProcessOptions& options )
	{
		Requests< SecureInformation::Socket > client;
		client.secure( sock, *this, options );
	}

	void ConsumerBase::GetHttp( int sock, const SocketProcessOptions& options )
	{
		Requests< PlainInformation::Socket > client;
		client.plain( sock, *this, options );
	}

	template <class StreamingType >
		void ConsumerBase::SpawnClients( )
		{
			const SocketProcessOptions& Options( Q );
			Locker& lock( Q );
			while ( (size_t) subprocesses.size() < (size_t) Options.forks )
			{
				const pid_t newfork( fork() );
				if ( newfork == 0 )
				{
					ThreadFunction f( Get );
					Threader threader( Options.threads, f, &Q );
					while ( !TERMINATE )
					{
						StreamingType ssrvc( Options.host.c_str(), Options.port, KruncherTools::GetUuid() );
						ssrvc.blocking( true );
						if ( ssrvc.open() && ssrvc.connect() )
						{
							ThreadWidget locker(lock);
							const int S( ssrvc.GetSock() );
							if ( S ) 
							{
								Q+=S;
							} else {
								stringstream ssout; 
								ssout << fence << "[ERROR]" << fence << "Cannot connect" << fence 
									<< Options.host << ":" << Options.port << endl;
								Log(VERB_ALWAYS, "InfoConsumer", ssout.str());
								cout << ssout.str() << endl;
								ssrvc.close();
							}
						}  else {
							stringstream ssout; 
							ssout << fence << "[ERROR]" << fence << "Cannot open" << fence 
								<< Options.host << ":" << Options.port << endl;
							Log(VERB_ALWAYS, "InfoConsumer", ssout.str());
							cout << ssout.str() << endl;
							//ssrvc.close();
						}
						Throttle( Options );
					}

				}  else {
					subprocesses+=newfork;
				}
			} 

		}

	template < class StreamingType >
		void ConsumerBase::SpawnClients( const SocketProcessOptions& options )
		{
			if ( options.scheme == https )
			{
				ERR_load_crypto_strings();
				SSL_load_error_strings();
			}
			SpawnClients<StreamingType>( );
		}

	void ConsumerBase::RunClients( const SocketProcessOptions& options )
	{
		Q=options;
		const SocketProcessOptions& Options( Q );
		SpawnClients<streamingsocket>( Options ); 
	}


} // InfoKruncher





