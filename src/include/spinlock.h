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
#include <pthread.h>
#include <vector>

#ifndef INFOLOCKER
#define INFOLOCKER

struct Locker 
{
	void Init()
	{
		pthread_mutexattr_init(&attrs);
		pthread_mutex_init(&cs,&attrs);
	}

	void Terminate()
	{
		pthread_mutex_destroy(&cs);
	}

	bool Enter()
	{
		return (!pthread_mutex_lock(&cs));
	}

	void Leave()
	{
		pthread_mutex_unlock(&cs);
	}

private:
	pthread_mutex_t cs;
	pthread_mutexattr_t attrs;
};

struct ThreadWidget
{
	ThreadWidget(Locker& _lock) : lock(_lock)
	{
		if ( ! _lock.Enter() ) throw std::string("Cannot get mutex lock" );
	}
	~ThreadWidget() {lock.Leave();}
private:
	Locker& lock;
};







typedef void* (*ThreadFunction)(void*) ;

struct Threader : private std::vector< pthread_t >
{
	Threader() : nthreads( 1 ) {}
	Threader( const int _nthreads, ThreadFunction tf, void* Q ) 
		: nthreads( _nthreads ), threadfunction( tf )
	{
		pthread_t t( (pthread_t) NULL);
		for ( int j=0; j<nthreads; j++ )
		{
			pthread_create(&t,0,threadfunction, Q);
			if (!t) throw "Can't thread";
			push_back(t);
		}
	}


	~Threader()
	{
		for (std::vector<pthread_t>::iterator it=begin(); it!=end(); it++)
		{
			pthread_t t(*it);
			pthread_join(t, 0);
		}
	}
	private:
	const int nthreads;
	ThreadFunction threadfunction;
	
};

#endif // INFOLOCKER


