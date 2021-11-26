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
#ifndef HYPER_H
#define HYPER_H

#include <hyperbase.h>
#include <directory.h>
#include <memory>

namespace Hyper
{ 

	inline string statusText(const int status)
	{
		switch ( status )
		{
			case 200: return "OK";
			case 403: return "already exists";
			case 404: return "not found";
			case 414: return "invalid content length";
			case 422: return "cookies are required to enter this site";
		}
		return "ERROR";
	}

	inline string RequestUrl(const char* _line)
	{
		if (!_line) return "";
		string line(_line);
		const size_t sp1(line.find(" "));
		if (sp1==string::npos) return "";
		const size_t sp2(line.find(" ", sp1+1));
		if (sp2==string::npos) return "";
		return line.substr(sp1,sp2-sp1);
	}


	inline bool IsBinaryContent(const string& contenttype)
	{
		if ( contenttype == "image/png" ) return true;
		if ( contenttype == "image/jpeg" ) return true;
		if ( contenttype == "image/x-icon" ) return true;
		return false;
	}

	inline string ContentType(const string& _what)
	{
		icstring what(_what.c_str());

		if (what.find(".png")!=string::npos) 
			return "image/png";

		if (what.find(".jpeg")!=string::npos) 
			return "image/jpeg";

		if (what.find(".jpg")!=string::npos) 
			return "image/jpeg";

		if (what.find(".ico")!=string::npos) 
			return "image/x-icon";

		if (what.find(".xslt")!=string::npos) 
			return "application/xhtml+xml";

		if (what.find(".css")!=string::npos) 
			return "text/css";

		if (what.find(".js")!=string::npos) 
			return "text/javascript";

		if (what.find(".xml")!=string::npos) 
			return "text/xml";

		if (what.find(".md")!=string::npos) 
			return "text/plain";

		return "text/html";
	}

	template <class SocketType>
		struct Response 
	{
		virtual ~Response(){}
		virtual void operator ()() 
		{
		    Log("Default Response::operator()()");
		}
	};

	template <class SocketType>
		struct Request : virtual HyperBase< SocketType >
	{
		Request(const string& _request, const MimeHeaders& _headers, SocketType& _sock ) :
			HyperBase< SocketType >(_request, _headers, _sock) {}
		virtual ~Request() { response.release(); } 
		const char* c_str() const {return HyperBase< SocketType >::request.c_str();}
		operator SocketType& () const {return HyperBase< SocketType >::sock;}
		string Host() const { return host; }
		string host; 
		virtual operator Response<SocketType>& () 
		{ 
		    if ( ! response.get() ) throw string( "Can't get response" );
		    return *response.get(); 
		}
		protected:

		auto_ptr<Response<SocketType> > response;
		virtual ostream& operator<<(ostream& o) const { /*o << fence << "[request]" << fence << Host() << fence << RequestUrl(request.c_str()) << fence; */ return o; }
	};


	struct JavaScripter : stringvector
	{
		JavaScripter( const string _path ) : path( _path ) {}
		operator bool ()
		{
			const string jsline( "<script type=\"text/javascript\" src=\"" );
			for ( iterator it=begin();it!=end();it++ )
			{
				string& line( *it );
				const size_t sojs( line.find( jsline ) );
				if ( sojs == string::npos ) continue;
				const string scripttag( line.substr( sojs, line.size()-sojs ) );
				line.clear();
				const size_t srctag( scripttag.find( "src=\"" ) );
				if ( srctag == string::npos ) {line=scripttag; continue;}
				const size_t sofn( srctag + strlen( "src=\"" ) );
				if ( sofn >= scripttag.size() ) {line=scripttag; continue;}
				const size_t eofn( scripttag.find( "\"", sofn ) );
				if ( srctag == string::npos ) {line=scripttag; continue;}
				const string fname( scripttag.substr( sofn, eofn-sofn ) );
			

				const string pathname( path + fname );
				const string jsstart( "<script>\n" );
				const string jsend( "\n</script>\n" );
				line=jsstart+KruncherDirectory::LoadFile( pathname, 3000, 512 )+jsend;
				
			}
			return true;
		}
		private:
		const string path;
	};

} // Hyper

#endif // HYPER_H


