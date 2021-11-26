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

#ifndef INFOSOCKS_H
#define INFOSOCKS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sstream>
#include <memory>
#include <set>
#include <signal.h>

#include <infostream.h>


#include <infotools.h>

namespace PlainInformation
{
	#define thing (*this)
	#define BufferSize 256

	template <class SocketType>
		struct SocketBuffer : protected streambuf
	{
		SocketBuffer(SocketType& st) : 
			streambuf(),
			much(0),
			D(st),
			sock(0) ,
			buffersize(BufferSize), 
			inb(NULL), 
			outb(NULL)
		{
			inb=(char*)malloc(buffersize+6);setg(inb+4,inb+4,inb+4);
			outb=(char*)malloc(buffersize+6);setp(outb,outb+(buffersize-1));
		}
		SocketBuffer(SocketType& st,int _bsize) : 
			streambuf(),
			much(0),
			D(st), 
			sock(0), 
			buffersize(_bsize), 
			inb(NULL), 
			outb(NULL)
		{
			inb=(char*)malloc(buffersize+6);setg(inb+4,inb+4,inb+4);
			outb=(char*)malloc(buffersize+6);setp(outb,outb+(buffersize-1));
		}
		virtual ~SocketBuffer() {sync(); if (outb) free(outb); if (inb) free(inb);}
		void operator =(int s){sock=s;}
		streamsize gcount() const { return GCount; }
	protected:
		void blocking(bool b=false){Blocking=b;}
		unsigned long much;
		virtual int underflow()
		{
			if (gptr() < egptr()) 
				return *gptr();
			int putback( gptr()-eback() );
			if (putback>4) putback=4;
			memcpy(inb+(buffersize-putback),gptr()-putback,putback);
			int num(read((char*)inb+4,(size_t)buffersize-4));
			if (num<=0) return EOF;
			setg(inb+(4-putback),inb+4,inb+4+num);
			return *gptr();
		}
		virtual int overflow(int c)
		{
			if (c!=EOF) {*pptr()=c; pbump(1);}
			if (Flush()==EOF) {return EOF;}
			return c;
		}
		virtual int sync(){if (Flush()==EOF) return -1;			return 0;}
		virtual int read(char* destination,size_t byts)
		{
			const int ret(recv(sock, destination, byts, 0));
			GCount=ret;
			return ret;
		}

	private:
		virtual void close() = 0;
		virtual int Flush()
		{
			if (!sock) return 0;
			const int num( pptr()-pbase() );
			if (!num) return 0;
			if (write(outb,num)!=num) return EOF;
			pbump(-num);
			return num;
		}		
		SocketType& D;
	protected:
		virtual int write(const char* source,size_t byts)
		{ 
			fd_set wfds;
			struct timeval timeout;
			timeout.tv_sec = 0;
			timeout.tv_usec = 100; 
			int sel_value = select(sock+1, &wfds, NULL, NULL, &timeout);
			if(sel_value == -1) throw string("Select failed on send");

		    const int ret(send(sock, source, byts, MSG_NOSIGNAL)); 
		    return ret;
		}
		virtual void throttle () = 0;
		int sock;
		int buffersize;
		char* inb,*outb;
		bool Blocking;
		int GCount;
	};


	struct Socket : public streamingsocket, public iostream
	{
		unsigned long auth;
		Socket(int sock, const string _uuid ) : 
			streamingsocket((char*)"",0, _uuid), 
			iostream((streambuf*)&buffer),
			auth( 0 ), 
			buffer(*this)
		{
			KruncherTools::Log(VERB_PSOCKETS, string("infosock|")+uuid, "creating" );
			buffer=sock;SetSock(sock);
		}
		Socket(int sock,int buffersize, const string _uuid="") : 
			streamingsocket((char*)"",0, _uuid), 
			iostream((streambuf*)&buffer),
			auth( 0 ), 
			buffer(*this,buffersize)
		{
			KruncherTools::Log(VERB_PSOCKETS, string("infosock|")+uuid, "creating" );
			buffer=sock;SetSock(sock);
		}
		Socket(char* host, int port, const string _uuid="") : 
			streamingsocket(host,port, _uuid), 
			iostream((streambuf*)&buffer),
			auth( 0 ), 
			buffer(*this)
		{
			KruncherTools::Log(VERB_PSOCKETS, string("infosock|")+uuid, "creating" );
		}

		Socket(char* host, int port,int socktype, const string _uuid="") : 
			streamingsocket(host,port, _uuid), 
			iostream((streambuf*)&buffer),
			auth( 0 ), buffer(*this)
		{
			KruncherTools::Log(VERB_PSOCKETS, string("infosock|")+uuid, "creating" );
		}

		virtual bool open(int socktype=SOCK_STREAM,int addressfamily = AF_INET,int schemer=IPPROTO_TCP) 
		{
			KruncherTools::Log(VERB_PSOCKETS, string("infosock|")+uuid, "opening" );
			if (!streamingsocket::open(socktype,addressfamily,schemer)) return false; 
			buffer=GetSock(); 
			return true;
		}
				
		virtual void close()
		{
			KruncherTools::Log(VERB_PSOCKETS, string("infosock|")+uuid, "Closing secure socket");
			sync();
			streamingsocket::close();
			buffer=0;
			buffer.much=0; buffer.bread=0; 
		}
		virtual ~Socket() 
		{
			KruncherTools::Log(VERB_PSOCKETS, string("infosock|")+uuid, "destroying");
			close();
		}
		void getline(KruncherTools::binarystring& line, const  int limit=0)
		{
			throw string("wip - see infossock");
		}
		void getline(string& line, const  int limit=0)
		{
			throw string("wip - see infossock");
			KruncherTools::Log(VERB_PSOCKETS, string("infosock|")+string("sgetline"), line );
		}

		virtual int write(const char* source,long byts)
		{
			KruncherTools::Log(VERB_PSOCKETS, string("infosock|")+uuid, "writing" );
			return buffer.write((char*)&source[0],byts);
		}
		virtual int read(char* dest,long byts)
		{
			KruncherTools::Log(VERB_PSOCKETS, string("infosock|")+uuid, "reading" );
			return buffer.read(dest,byts);
		}
		virtual void throttle() {}

		virtual bool blocking(bool b=false) 
		{ 
			KruncherTools::Log(VERB_PSOCKETS, string("infosock|")+uuid, "blocking" );
			streamingsocket::blocking(b); 
			buffer.blocking(b);
			return b;
		}

		size_t gcount() { return buffer.gcount(); }
		protected:
		struct  StreamBuffer : public SocketBuffer  <StreamBuffer>
		{
			friend struct Socket;
			private:
			StreamBuffer(Socket& _linkage) : 
				SocketBuffer<StreamBuffer>(thing),
				linkage(_linkage),
				much(0),bread(0)
			 {}
			StreamBuffer(Socket& _linkage,int bsize) :
				SocketBuffer<StreamBuffer>(thing,bsize),
				linkage(_linkage),
				much(0), bread(0)
			{}
			virtual void close() {linkage.close();}
			virtual void throttle (){linkage.throttle();}
			virtual streamsize gcount() const { return SocketBuffer<StreamBuffer>::gcount(); }
			virtual int write(const char* source,size_t byts)
				{ return SocketBuffer<StreamBuffer>::write(source,byts); }
			virtual int read(char* dest,size_t byts)
				{ return SocketBuffer<StreamBuffer>::read((char*)dest,byts); }
			void operator=(int s){SocketBuffer<StreamBuffer>::operator=(s);}
			Socket& linkage;
			unsigned long long much,bread;
		} buffer;		
	};




} // PlainInformation

#endif //INFOSOCKS_H
// WebKruncher.com




