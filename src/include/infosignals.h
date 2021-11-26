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
#ifndef KRUNCHSIGNALS_H
#define KRUNCHSIGNALS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
namespace KruncherTools
{
	typedef void (*SigFunction)(int) ;
	void TerminateSignalHandler (int signum); 
	void SignalHandler (int signum);

	struct SignalSpec
	{
		SignalSpec( const int _signo, const string _name, const string _action, const string _description, SigFunction _sigfunction )
			: signo( _signo ), name( _name ), action( _action ), description( _description ), sigfunction( _sigfunction  )  {}
		operator const int () const { return signo; }
		operator const string () const { return name + string(", " ) + description; }
		operator SigFunction () const { return sigfunction; }
		private:
		const int signo;
		const string name;
		const string action;
		const string description;
		SigFunction sigfunction;
	};

	struct Signals : map< int, SignalSpec >
	{
		operator bool ();
		void operator+=( const SignalSpec& spec )
		{
			const int signo( spec );
			const_iterator found( find( signo ) );
			if ( found != end() )
			{
				const SignalSpec& that( found->second );
				stringstream ss;
				const string& thisname( spec );
				const string& thatname( that );
				ss << "Signal " << thisname << " already added as " << thatname << endl;
				Log( ss.str() );
			}
			insert( pair< int, SignalSpec >( signo, spec ) ); 
		}
	};



	inline void TerminateSignalHandler (int signum) 
	{ 
		Signals sigs;
		if ( ! sigs ) { Log( "Cannot load signal table" ); sleep( 1 ); return; }
		if ( ( (size_t) signum < (size_t) 0 ) || ( (size_t) signum > (size_t) sigs.size() ) )
		{
			Log( "Invalid signal" );
			sleep( 1 );
			return;
		}

		Signals::const_iterator found( sigs.find( signum ) );
		if ( found != sigs.end() )
		{
			const SignalSpec& that( found->second );
			const string& name( that );
			Log( string("Signal: " ) + name + string(" Setting TERMINATE ") );
		}

		usleep( 1e5 );
		TERMINATE=true; 
	}

	inline void SegfaultSignalHandler (int signum)
	{ 
		Signals sigs;
		if ( ! sigs ) { Log( "Cannot load signal table" ); sleep( 1 ); return; }
		if ( ( (size_t) signum < (size_t) 0 ) || ( (size_t) signum > (size_t) sigs.size() ) )
		{
			Log( "Invalid signal" );
			sleep( 1 );
			return;
		}
		Signals::const_iterator found( sigs.find( signum ) );
		if ( found != sigs.end() )
		{
			const SignalSpec& that( found->second );
			const string& name( that );
			Log( VERB_SIGNALS, "SegfaultSignalHandler", name );
		}

		signal(signum, SIG_DFL);
		kill(getpid(), signum);

		usleep( 2 );
	}


	inline void SignalHandler (int signum)
	{ 
		Signals sigs;
		if ( ! sigs ) { Log( "Cannot load signal table" ); sleep( 1 ); return; }
		if ( ( (size_t) signum < (size_t) 0 ) || ( (size_t) signum > (size_t) sigs.size() ) )
		{
			Log( "Invalid signal" );
			sleep( 1 );
			return;
		}
		Signals::const_iterator found( sigs.find( signum ) );
		if ( found != sigs.end() )
		{
			const SignalSpec& that( found->second );
			const string& name( that );
			Log( VERB_SIGNALS, "SignalHandler", name );
		}
		usleep( 1e5 );
	}



	inline bool SetSignals()
	{
		Signals sigs;
		if ( ! sigs ) { Log( "Cannot load signal table" ); sleep( 1 ); return -1; }

		for ( Signals::const_iterator it=sigs.begin(); it!=sigs.end(); it++ )
		{
			const SignalSpec& sig( it->second );
			const int& signo( sig );
			SigFunction sigfun( sig );
			if (signal (signo, sigfun) == SIG_IGN)
				signal (signo, SIG_IGN);
		}

		return true;
	}

	inline Signals::operator bool ()
	{
		Signals& me( *this );
		me+=( SignalSpec( SIGHUP, "SIGHUP", "terminate process", "terminal line hangup", SignalHandler ));
		me+=( SignalSpec( SIGILL, "SIGILL", "create core image", "illegal instruction", SignalHandler ));
		me+=( SignalSpec( SIGTRAP, "SIGTRAP", "create core image", "trace trap", SignalHandler ));
		me+=( SignalSpec( SIGABRT, "SIGABRT", "create core image", "abort(3) call (formerly SIGIOT)", SignalHandler ));
		me+=( SignalSpec( SIGEMT, "SIGEMT", "create core image", "emulate instruction executed", SignalHandler ));
		me+=( SignalSpec( SIGFPE, "SIGFPE", "create core image", "floating-point exception", SignalHandler ));
		me+=( SignalSpec( SIGKILL, "SIGKILL", "terminate process", "kill program (cannot be caught or", SignalHandler ));
		me+=( SignalSpec( SIGBUS, "SIGBUS", "create core image", "bus error", SignalHandler ));
		me+=( SignalSpec( SIGSEGV, "SIGSEGV", "create core image", "segmentation violation", SegfaultSignalHandler ));
		me+=( SignalSpec( SIGPIPE, "SIGPIPE", "terminate process", "write on a pipe with no reader", SignalHandler ));
		me+=( SignalSpec( SIGALRM, "SIGALRM", "terminate process", "real-time timer expired", SignalHandler ));
		me+=( SignalSpec( SIGURG, "SIGURG", "discard signal", "urgent condition present on socket", SignalHandler ));
		me+=( SignalSpec( SIGTSTP, "SIGTSTP", "stop process", "stop signal generated from keyboard", SignalHandler ));
		me+=( SignalSpec( SIGCONT, "SIGCONT", "discard signal", "continue after stop", SignalHandler ));
		me+=( SignalSpec( SIGCHLD, "SIGCHLD", "discard signal", "child status has changed", SignalHandler ));
		me+=( SignalSpec( SIGTTIN, "SIGTTIN", "stop process", "background read attempted from control", SignalHandler ));
		me+=( SignalSpec( SIGTTOU, "SIGTTOU", "stop process", "background write attempted to control", SignalHandler ));
		me+=( SignalSpec( SIGIO, "SIGIO", "discard signal", "I/O is possible on a descriptor (see", SignalHandler ));
		me+=( SignalSpec( SIGXCPU, "SIGXCPU", "terminate process", "CPU time limit exceeded (see", SignalHandler ));
		me+=( SignalSpec( SIGXFSZ, "SIGXFSZ", "terminate process", "file size limit exceeded (see", SignalHandler ));
		me+=( SignalSpec( SIGVTALRM, "SIGVTALRM", "terminate process", "virtual time alarm (see setitimer(2))", SignalHandler ));
		me+=( SignalSpec( SIGPROF, "SIGPROF", "terminate process", "profiling timer alarm (see", SignalHandler ));
		me+=( SignalSpec( SIGWINCH, "SIGWINCH", "discard signal", "window size change", SignalHandler ));
		me+=( SignalSpec( SIGINFO, "SIGINFO", "discard signal", "status request from keyboard", SignalHandler ));
		me+=( SignalSpec( SIGUSR1, "SIGUSR1", "terminate process", "user-defined signal 1", SignalHandler ));
		me+=( SignalSpec( SIGUSR2, "SIGUSR2", "terminate process", "user-defined signal 2", SignalHandler ));
		me+=( SignalSpec( SIGTHR, "SIGTHR", "discard signal", "thread AST", SignalHandler ));

		me+=( SignalSpec( SIGSYS, "SIGSYS", "create core image", "system call given invalid argument", TerminateSignalHandler ));
		me+=( SignalSpec( SIGSTOP, "SIGSTOP", "stop process", "stop (cannot be caught or ignored)", TerminateSignalHandler  ));
		me+=( SignalSpec( SIGTERM, "SIGTERM", "terminate process", "software termination signal", TerminateSignalHandler  ));
		me+=( SignalSpec( SIGINT, "SIGINT", "terminate process", "interrupt program", TerminateSignalHandler   ));
		me+=( SignalSpec( SIGQUIT, "SIGQUIT", "create core image", "quit program", TerminateSignalHandler  ));
		return true;
	}
} // KruncherTools 
#endif // KRUNCHSIGNALS_H


