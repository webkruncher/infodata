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

#ifndef INFOSSOCKS_H
#define INFOSSOCKS_H

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

#include <infostream.h>

#pragma GCC diagnostic ignored "-Woverloaded-virtual"

#include <infotools.h>

namespace SecureInformation
{
	#define thing (*this)
	#define BufferSize 256

	template <class SocketType>
		struct SocketBuffer : protected streambuf
	{
		SocketBuffer(SocketType& st, SSL* _ssl=NULL) : 
			streambuf(),
			much(0), 
			D(st),
			sock(0), 
			buffersize(BufferSize), 
			inb(NULL), 
			outb(NULL),
			ssl( _ssl )
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
			outb(NULL),
			ssl( NULL )
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
			const int ret(SSL_read(ssl, destination, byts )); 
			GCount=ret;
			return ret;
		}
		virtual SSL* GetSsl(){return ssl;}

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
			const int ret(SSL_write(ssl, source, byts )); 
			return ret;
		}
		virtual void throttle () = 0;
		int sock;
		int buffersize;
		char* inb,*outb;
		bool Blocking;
		SSL* ssl;
		int GCount;
	};


	struct Socket : public streamingsocket, public iostream
	{
		unsigned long auth;
		Socket(int sock, const string _uuid="") : 
			streamingsocket((char*)"",0, _uuid), 
			iostream((streambuf*)&buffer),
			auth( 0 ), 
			buffer(*this)
		{
			KruncherTools::Log(VERB_SSOCKETS, string("infossock|")+uuid, "creating" );
			buffer=sock;
			SetSock(sock);
		}
		Socket(int sock, SSL* _ssl, const string _uuid="" ) : 
			streamingsocket((char*)"",0, _uuid), 
			iostream((streambuf*)&buffer),
			auth( 0 ), 
			buffer(*this, _ssl)
		{
			KruncherTools::Log(VERB_SSOCKETS, string("infossock|")+uuid, "creating" );
			buffer=sock;SetSock(sock);
		}
		Socket(int sock,int buffersize, const string _uuid="") : 
			streamingsocket((char*)"",0, _uuid), 
			iostream((streambuf*)&buffer),
			auth( 0 ), 
			buffer(*this,buffersize)
		{
			KruncherTools::Log(VERB_SSOCKETS, string("infossock|")+uuid, "creating" );
			buffer=sock;SetSock(sock);
		}
		Socket(char* host, int port, const string _uuid="") : 
			streamingsocket(host,port, _uuid), 
			iostream((streambuf*)&buffer),
			auth( 0 ), 
			buffer(*this)
		{
			KruncherTools::Log(VERB_SSOCKETS, string("infossock|")+uuid, "creating" );
		}

		Socket(char* host, int port,int socktype, const string _uuid="") : 
			streamingsocket(host,port, _uuid), 
			iostream((streambuf*)&buffer),
			auth( 0 ), 
			buffer(*this)
		{
			KruncherTools::Log(VERB_SSOCKETS, string("infossock|")+uuid, "creating" );
		}

		virtual bool open(int socktype=SOCK_STREAM,int addressfamily = AF_INET,int schemer=IPPROTO_TCP) 
		{
			KruncherTools::Log(VERB_SSOCKETS, string("infossock|")+uuid, "opening" );
			if (!streamingsocket::open(socktype,addressfamily,schemer)) return false; 
			buffer=GetSock(); 
			return true;
		}
				
		virtual void close()
		{
			KruncherTools::Log(VERB_SSOCKETS, string("infossock|")+uuid, "Closing secure socket");
			sync();
			streamingsocket::close();
			buffer=0;
			buffer.much=0; buffer.bread=0; 
		}
		virtual ~Socket()
		{
			KruncherTools::Log(VERB_SSOCKETS, string("infossock|")+uuid, "destroying" );
			close();
		}
		virtual SSL* GetSsl(){return buffer.GetSsl();}
		void getline(KruncherTools::binarystring& line, const  int limit=0)
		{
			int bread( 0 );
			while (true)
			{
				int c(get());
				bread++;
				if ( bread >= limit ) break;
				line.append( 1, (unsigned char) c );
				if ( bread >= 2 ) 
				{
					if (
						( line[ bread-2 ] == '\r' ) 
							&&
						( line[ bread-1 ] == '\n' ) 
					) { line.erase( line.size()-2, 2 );break;  }
				}
			}
			
		}
		void getline(string& line, const  int limit=0)
		{
			throw string("wip - needs work");
			int bread( 0 );
			stringstream ss;
			while (true)
			{
				int c(get());
				bread++;
				if ( bread >= limit ) break;
				if (!isprint(c)) break;
				ss << (char) c;
				if ( bread >= 2 ) 
				{
					if (
						( ss.str()[ bread-2 ] == '\r' ) 
							&&
						( ss.str()[ bread-1 ] == '\n' ) 
					) break; 
				}
			}
			line=ss.str();
			KruncherTools::Log(VERB_PSOCKETS, string("infossock|")+string("ssgetline"), line );
		}


		virtual int write(const char* source,long byts)
		{
			KruncherTools::Log(VERB_SSOCKETS, string("infossock|")+uuid, "writing" );
			return buffer.write((char*)&source[0],byts);
		}
		virtual int read(char* dest,long byts)
		{
			KruncherTools::Log(VERB_SSOCKETS, string("infossock|")+uuid, "reading" );
			return buffer.read(dest,byts);
		}
		virtual void throttle() {}

		virtual bool blocking(bool b=false) 
		{ 
			KruncherTools::Log(VERB_SSOCKETS, string("infossock|")+uuid, "blocking" );
			streamingsocket::blocking(b); 
			buffer.blocking(b);
			return true;
		}
		size_t gcount() { return buffer.gcount(); }
		protected:
		struct  StreamBuffer : public SocketBuffer  <StreamBuffer>
		{
			friend struct Socket;
			private:
			StreamBuffer(Socket& _linkage, SSL* _ssl=NULL) : 
				SocketBuffer<StreamBuffer>(thing, _ssl),
				linkage(_linkage) ,
				much(0),bread(0)
			 {}
			StreamBuffer(Socket& _linkage,int bsize) :
				SocketBuffer<StreamBuffer>(thing,bsize),linkage(_linkage),
				much(0),bread(0)
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




	inline void init_openssl()
	{ 
		//SSL_library_init();
		SSL_load_error_strings();	
		ERR_load_crypto_strings();
		//OpenSSL_add_ssl_algorithms();
	}

	inline void cleanup_openssl()
	{
		//EVP_cleanup();
	}

	inline SSL_CTX *create_context() 
	{
		const SSL_METHOD *method;
		SSL_CTX *ctx;

		method = SSLv23_server_method();

		ctx = SSL_CTX_new( method );

		if (!ctx)
		{
			ERR_print_errors_fp(stderr);
			throw string( "Cannot create SSL context" );
		}

		return ctx;
	}



	struct SslContextInfo
	{
		SslContextInfo(
				const string _cafile="",
				const string _cadir="",
				const string _certfile="",
				const string _keypasswd="",
				const string _keyfile=""
		) :
				cafile(_cafile),
				cadir( _cadir),
				certfile( _certfile ),
				keypasswd( _keypasswd),
				keyfile(_keyfile)
		{}

		const string cafile;
		const string cadir;
		const string certfile;
		const string keypasswd;
		const string keyfile;
	};


	inline STACK_OF(X509_NAME) * configure_context( SSL_CTX *ctx, SslContextInfo& sslctx )
	{
		if ( ! ctx ) return NULL;
		SSL_CTX_set_ecdh_auto(ctx, 1);

		if ( !sslctx.certfile.empty() )
			if (SSL_CTX_use_certificate_file(ctx, sslctx.certfile.c_str(), SSL_FILETYPE_PEM) <= 0) {
				ERR_print_errors_fp(stderr);
				exit(EXIT_FAILURE);
			}

		if ( !sslctx.keypasswd.empty() )
			SSL_CTX_set_default_passwd_cb_userdata(ctx,( char* ) sslctx.keypasswd.c_str() );

		if ( !sslctx.keyfile.empty() )
		{
			if (SSL_CTX_use_PrivateKey_file(ctx, sslctx.keyfile.c_str(), SSL_FILETYPE_PEM) <= 0 ) {
				ERR_print_errors_fp(stderr);
				exit(EXIT_FAILURE);
			}


			if (SSL_CTX_check_private_key(ctx) == 0) {
				printf("Private key does not match the certificate public key\n");
				exit(0);
			}
		}


		if ( false )
			SSL_CTX_set_verify(ctx,SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT,NULL);


		if ( (!sslctx.cafile.empty()) && (!sslctx.cadir.empty()) )
		{
			if (SSL_CTX_load_verify_locations(ctx, sslctx.cafile.c_str(), sslctx.cadir.c_str() ) <1 ) {
				printf("Error setting the verify locations.\n");
				return NULL;
			} 
		}

		STACK_OF(X509_NAME)* calist( SSL_load_client_CA_file(sslctx.cafile.c_str()) );
		SSL_CTX_set_client_CA_list( ctx, calist );
		return calist;
	}

	inline string GetSslError( const SSL* ssl, const int err )
	{
		stringstream sserr;
		const int sslerr( SSL_get_error( ssl, err ) );
		char errstring[ 256 ];

		ERR_error_string_n( sslerr, errstring, 255);
		const string errreason( ERR_reason_error_string( sslerr ) );

		stringstream ssmsg;
		ssmsg << "Cannot connect ssl. " << endl <<
			"Error: " << errstring << " " << endl <<
			"Reason: " << errreason;
		string msg( ssmsg.str() );
		return msg;
	}

	struct SslContext
	{
		SslContext( SSL_CTX* _ctx, SSL* _ssl ) :
			ctx( _ctx ), ssl( _ssl )  {}
		~SslContext()
		{
			if ( ssl ) SSL_free(ssl);
			if ( ctx ) SSL_CTX_free( ctx );
		}
		private:
		SSL_CTX* ctx;
		SSL* ssl;
	};

	inline SSL_CTX *InitSslClient()
	{
	    const SSL_METHOD *method( TLS_client_method() );
	    SSL_CTX *ctx( SSL_CTX_new(method) );
	    return ctx;
	}

	inline string CertInfo(SSL *ssl)
	{
		stringstream ss;
		X509 *cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
		if (cert != NULL)
		{
			char *line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
			ss << "Subject:" << line << endl;
			delete line;
			line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
			ss << "Issuer:" << line << endl;
			delete line;
			X509_free(cert);
		}
		return ss.str();
	}
} // SecureInformation

#endif //INFOSSOCKS_H
// WebKruncher.com





