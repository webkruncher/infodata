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


#ifndef INFOBASE_H
#define INFOBASE_H

#include <infotools.h>
#include <spinlock.h>
#include <iostream>
#include <iostream>
#include <sstream>
#include <process.h>
#include <deque>
using namespace std;

#include <hypertools.h>
#include <infossock.h>
#include <infosock.h>

namespace InfoKruncher
{
	using namespace std;
	pair<string, string> TransformHeaderNameToLower( const string& line );

	enum Scheme{ http, https };
	struct ServiceBase;
	struct ConsumerBase;
	struct SocketProcessOptions
	{
		SocketProcessOptions() :
				datapath( "/var/db/infokruncher/" ),
				host( "localhost" ),
				scheme( http ),
				port( 80 ),
				forks( 8 ),
				threads( 5 ),
				timeout( 2, 0 )
		{}

		string datapath;
		string host;
		Scheme scheme;
		int port;
		int forks;
		int threads;
		pair<int,int> timeout;	
		string path;
		string certfile;
		string keypasswd;
		string keypasswdfile;
		string keyfile;
		string cafile;
		string cadir;


		string text;

		mutable KruncherTools::stringmap metadata; // cookies, oauth

		void operator()( const string name, const string  value )
		{
			if ( value.empty() ) return;
			if ( name == "scheme" ) 
			{
				if ( value == "http" ) scheme=http;
				if ( value == "https" ) scheme=https;
			}

			if ( name == "path" ) path=value;
			if ( name == "host" ) host=value;
			if ( name == "certfile" ) certfile=value;
			if ( name == "keypasswd" ) keypasswd=value;
			if ( name == "keypasswdfile" ) keypasswdfile=value;
			if ( name == "keyfile" ) keyfile=value;
			if ( name == "cafile" ) cafile=value;
			if ( name == "cadir" ) cadir=value;
			if ( name == "port" ) port=atoi( value.c_str() );
			if ( name == "forks" ) forks=atoi( value.c_str() );
			if ( name == "threads" ) threads=atoi( value.c_str() );
		}

		friend ostream& operator<<(ostream&,const SocketProcessOptions&);

		ostream& operator<<(ostream& o) const
		{
			if ( ! path.empty() ) o << "path:" << path << endl;
			if ( ! host.empty() ) o << "host:" << host << endl;
			if ( ! certfile.empty() ) o << "certfile:" << certfile << endl;
			if ( ! keypasswd.empty() ) o << "keypasswd:" << keypasswd << endl;
			if ( ! keypasswdfile.empty() ) o << "keypasswdfile:" << keypasswdfile << endl;
			if ( ! keyfile.empty() ) o << "keyfile:" << keyfile << endl;
			if ( ! cafile.empty() ) o << "cafile:" << cafile << endl;
			if ( ! cadir.empty() ) o << "cadir:" << cadir << endl;
			o << "forks:" << forks << endl;
			o << "port:" << port << endl;
			o << "threads:" << threads << endl;
			if ( scheme == http ) o << "scheme:http" << endl;
			if ( scheme == https ) o << "scheme:https" << endl;
			return o;
		}
	}; 
	inline ostream& operator<<(ostream& o,const SocketProcessOptions& m) {return m.operator<<(o);}

	template <class BaseType>
		struct ThreadQueue
	{
		ThreadQueue( BaseType& _sb ) : callback( _sb ) { lock.Init(); }
		void operator+=(int j){socks.push_back(j);}
		operator int ()
		{ 
			if (socks.empty()) return 0;
			const int j(socks.front());
			socks.pop_front();
			return j;
		}
		operator Locker& () { return lock; }
		void operator=( const SocketProcessOptions& o ) { svcoptions=o; }
		operator const SocketProcessOptions& () const { return svcoptions; }
		void operator()(); 
		Locker lock;
		private:
		BaseType& callback;
		SocketProcessOptions svcoptions;
		deque<int> socks;
	};



} // namespace InfoKruncher
#endif //INFOBASE_H

