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
#ifndef KRUNCHPROCESSTOOLS
#define KRUNCHPROCESSTOOLS

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <infosignals.h>
namespace KruncherTools
{

	struct Daemonizer
	{
		Daemonizer(const bool Daemonize, const char* LogTag)
		{
			if ( Daemonize )
			{
				pid_t pid(fork()); 
				if (pid < 0)  throw string("Cannot daemonize");
				if (pid > 0) exit(0);
				umask(0);

				pid_t sid(setsid());
				if (sid < 0) throw string("Cannot create process group");
				close(STDIN_FILENO);
				close(STDOUT_FILENO);
				close(STDERR_FILENO);
			}
	
			openlog(LogTag,LOG_NOWAIT|LOG_PID,LOG_USER); 
			//syslog(LOG_NOTICE, "Starting up"); 
		}
		~Daemonizer()
		{
			//syslog(LOG_NOTICE, "Shutting down"); 
		}
	};


	inline void Initialize()
	{
		srand( time( 0 ) );
		if ( ! SetSignals() ) throw string( "Cannot initialize" );
	}

	struct SubProcesses : private vector< pid_t >
	{
		void operator+=( pid_t p ) { push_back( p ); }
		size_t size() { return vector< pid_t >::size(); }
		void Terminate()
		{
			for ( vector<pid_t>::iterator pit=begin();pit!=end();pit++)
			{
				kill(*pit, SIGKILL);
			}
			clear();
		}
	};


} // KruncherTools
#endif // KRUNCHPROCESSTOOLS

