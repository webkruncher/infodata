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

#ifndef INFOSTREAM_H
#define INFOSTREAM_H

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

#include <openssl/ssl.h>
#include <openssl/err.h>


	inline bool Serve(int sock,sockaddr_in& client,int backlog)
	{
		struct linger lingerer = {1,1};
		int optval = 1;
		if (-1==::setsockopt(sock, SOL_SOCKET, SO_LINGER, &lingerer,sizeof(linger))) 
			throw string("Can't set socket options");
		if (-1==::setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int))) 
			throw string("Can't set socket options");
		if (-1==::bind(sock, (struct sockaddr *) &client, sizeof(sockaddr_in))) 
			throw string("bind error");
		if (-1==::listen(sock, backlog)) throw string("listen error");
		return true;
	}

	inline bool Blocking(int sock,bool b)
	{
		if (!b) fcntl(sock, F_SETFL, O_NONBLOCK);
		else fcntl(sock, F_SETFL, 0);
		return b;
	}

	inline int Accept(int sock,sockaddr_in& client)
	{
		int addrlen=sizeof(client);
		return ::accept(sock,(struct sockaddr *) &client,(socklen_t*)&addrlen);
	}


	inline int Timeout(u_long sock,int s,int u)
	{
		int ret(1);
		struct timeval tv;
		tv.tv_sec = s;
		tv.tv_usec = u;
		if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv)) < 0) 
			{ret=0; throw string("Cannot set timeout");}
		if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) 
			{ret=0; throw string("Cannot set timeout");}
		return ret;
	}

	inline string dotted( const in_addr& addr)
	{
		stringstream ss;
		ss<<inet_ntoa(addr);
		return ss.str();
	}

	inline ostream& operator<<( ostream& o, const in_addr& addr )
	{
		o << dotted( addr );
		return o;
	}

	inline int operator<( const in_addr& a, const in_addr& b )
	{
		string k1( dotted( a ) );
		string k2( dotted( b ) );
		return k1<k2;
	}

	
class streamingsocket
{
	public:
		streamingsocket(const char* _host,int _port, const string _uuid) 
			: 
				host(_host), port(_port), uuid( _uuid ), sock(0),
				_fail(false),_open(false) {}		
	public:

		virtual bool blocking(bool b=false) { return Blocking(sock,b); }
		virtual bool listen(int n) { return Serve(n,client,10); }
		virtual bool listen() { return Serve(sock,client,100); }
		virtual int accept() { return Accept(sock,client); }
		virtual bool is_open(){return _open;}
		virtual int timeout(int s,int u) { return Timeout(sock,s,u); }

		virtual bool open(int socktype=SOCK_STREAM,int addressfamily = AF_INET,int schemer=IPPROTO_TCP) 
		{
			if (_open) return true;
			if (sock) return true;
			sock=socket(addressfamily,socktype,schemer); 
			if (sock==-1) 
			{
				cerr<<"Can't get a socket"<<endl;
				_fail=true; 
				return false;
			}
			memset(&client, 0, sizeof(struct sockaddr_in));	

			if (!GetHost().size())
			{
				client.sin_family 	= AF_INET;
				client.sin_port=htons(port);
			} else {
				client.sin_family = AF_INET;
				client.sin_addr.s_addr=inet_addr(GetHost().c_str());
				client.sin_port=htons(port);
				he = ::gethostbyname(GetHost().c_str());
				if (!he) 
				{
					_fail=true;
					close();
					return false;
				}	
				memcpy((char *) &client.sin_addr,(char *) he->h_addr,he->h_length);	
			}

			_open=true;
			return true;
		}

		virtual bool connect()
		{
			if (!GetHost().size())  return false;
			int rval=::connect(sock,(struct sockaddr *)&client,sizeof(client));
			if (rval!=0) _fail=true; 
			return !_fail;
		}

		virtual void close()
		{
			if (sock) ::shutdown(sock,SHUT_RDWR);
			if (sock) ::close(sock);
			sock=0; 
		}

		virtual int GetSock() const {return sock;}
		virtual int GetPort() const { return port; }
		virtual string GetHost() const {return host;}
		virtual void SetHost(char* h){host = h;}
		virtual void SetSock(int s) {sock=s;}
		operator const in_addr& () const
		{
			socklen_t saSize(sizeof(struct sockaddr_in));
			getpeername(sock,(struct sockaddr *)&client,&saSize);
			return client.sin_addr;
		}
	private:
		string host;
		int port;
	protected:
		const string uuid;
	private:
		int sock;
		hostent* he;
		bool _fail;
		bool _open;
	protected:
		struct sockaddr_in client,listenclient;
};


#endif // INFOSTREAM_H


