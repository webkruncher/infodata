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


#ifndef INFOCLIENT_H
#define INFOCLIENT_H

#include <infotools.h>
#include <spinlock.h>
#include <iostream>
#include <iostream>
#include <sstream>
#include <process.h>
#include <deque>
using namespace std;

#include <infotools.h>
#include <infossock.h>
#include <mimes.h>

namespace InfoKruncher
{
	using namespace std;
	pair<string, string> TransformHeaderNameToLower( const string& line );

	struct ConsumerBase
	{
		ConsumerBase( ) : Q( *this ) {}
		virtual void operator()( ThreadQueue<ConsumerBase>& ) ;
		virtual void SendRequest( SecureInformation::Socket& ss, const SocketProcessOptions& ) {}
		virtual void SendRequest( PlainInformation::Socket& ss, const SocketProcessOptions& ) {}
		virtual void Consume( KruncherMimes::SocketManager&, const SocketProcessOptions& ) throw() {}
		protected:
		void RunClients( const SocketProcessOptions& );
		KruncherTools::SubProcesses subprocesses;
		private:
		virtual void Throttle( const SocketProcessOptions& ) = 0;

		template < class StreamingType >
			void SpawnClients( const SocketProcessOptions& );

		template <class StreamingType >
			void SpawnClients( );

		void GetHttps( const int sock, const SocketProcessOptions& );
		void GetHttp( const int sock, const SocketProcessOptions& );
		ThreadQueue<ConsumerBase> Q;
	};

	template <class Curious>
		struct Consumer : Curious
	{
		virtual ~Consumer() {}
		void GetSiteMetaData( const SocketProcessOptions& );
		void ForkAndRequest( const SocketProcessOptions& );
		void Terminate();
		private:
		virtual void operator()( ThreadQueue<ConsumerBase>& q ) {Curious::operator()( q );}
	};
	void* Get(void* lk);

	struct RequestBase
	{
		void secure( int sock, ConsumerBase& base, const SocketProcessOptions& options );
		void plain( int sock, ConsumerBase& base, const SocketProcessOptions& options );
	};

	template <class SocketType>
		struct Requests : RequestBase
		{
			virtual ~Requests() {}
		};
} // namespace InfoKruncher
#endif //INFOCLIENT_H


