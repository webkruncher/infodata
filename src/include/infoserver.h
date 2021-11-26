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


#ifndef INFOSERVER_H
#define INFOSERVER_H

#include <infotools.h>
#include <spinlock.h>
#include <iostream>
#include <iostream>
#include <sstream>
#include <process.h>
#include <deque>
using namespace std;

#include <infossock.h>
#include <infosock.h>
#include <mimes.h>

namespace InfoKruncher
{
	using namespace std;


	struct ServiceBase
	{
		ServiceBase( ) :  Q( *this ) {}
		virtual void operator()( ThreadQueue<ServiceBase>& ) = 0;
		virtual void ServeRequest( KruncherMimes::SocketManager& ss, const in_addr&, const SocketProcessOptions& ) {}
		
		protected:
		void RunService( const SocketProcessOptions& );
		KruncherTools::SubProcesses subprocesses;
		private:
		virtual void Throttle( const SocketProcessOptions& ) = 0;
		void InitializeService( streamingsocket& );
		void SpawnService( const SocketProcessOptions& );
		void SpawnService( streamingsocket& );
		void ServeHttps( const int sock, const SocketProcessOptions& );
		void ServeHttp( const int sock, const SocketProcessOptions& );
		ThreadQueue<ServiceBase> Q;
	};

	template <class Curious>
		struct Service : Curious
	{
		void ForkAndServe( const SocketProcessOptions& );
		void Terminate();
		private:
		virtual void operator()( ThreadQueue<ServiceBase>& q ) {Curious::operator()( q );}
	};
	void* Serve(void* lk);


} // namespace InfoKruncher
#endif //INFOSERVER_H


