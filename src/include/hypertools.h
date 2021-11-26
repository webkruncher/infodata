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
#ifndef HYPERTOOLS_H
#define HYPERTOOLS_H
#include <infotools.h>
using namespace KruncherTools;
namespace Hyper
{
	inline string mimevalue(string line)
	{
		size_t c(line.find(":"));
		if (c==string::npos) return "";
		c++;
		line.erase(0, c);
		size_t s(line.find_first_not_of("\r\n\f\t "));
		if ( (s!=0) && (s!=string::npos) ) line.erase(0, s);
		size_t e(line.find_first_of("\r\n\f\t "));
		if (e!=string::npos) line.erase(e, line.size()-e);
		return line;
	}

	struct MimeHeaders : KruncherTools::stringmap
	{
		MimeHeaders( const string& mimeheaders )
		{
			split( mimeheaders, "\r" );
		}

		virtual void split( string what, string _how )
		{
			while ( true )
			{
				const size_t where( what.find( _how ) );
				if ( where  == string::npos )
				{
					if ( what.size() ) mimeinsert( what );
					return;
				}
				mimeinsert( what.substr(0, where ) );
				what.erase( 0, where + _how.size() );
			}
		}

		const string TransferEncoding() 
		{
			MimeHeaders::const_iterator it( find( "transfer-encoding" ) ); 
			if ( it == end() ) return transferencoding;
			transferencoding=it->second;
			KruncherTools::trim( transferencoding );
			return transferencoding;
		}
		const size_t ContentLength() const
		{
			MimeHeaders::const_iterator it( find( "content-length" ) ); 
			if ( it == end() ) return 0;
			char *Ender( NULL );
			return strtol( it->second.c_str(), &Ender, 10 );
		}
		const string Method() const { return method; }
		const string Resource() const { return resource; }
		private:
		string method, resource;
		string transferencoding;
		void GetRequestLine( const string& line )
		{
			KruncherTools::stringvector parts;
			parts.split( line, " " );
			if ( parts.size() > 0 ) method=parts[ 0 ];
			if ( parts.size() > 1 ) resource=parts[ 1 ];
		}

		void trim( string& line )
		{
			while ( ! line.empty() )
			{
				char s( line[ 0 ] );
				if ( ( s == '\r' ) || ( s == '\n' ) ) line.erase( 0, 1 );
				else break;
			}
			const size_t e( line.find_first_of( "\r\n" ) );
			if ( e != string::npos ) line.resize( e );
		}

		void mimeinsert( const string line )
		{
			if ( method.empty() )
				{ GetRequestLine( line ); return; }
			const size_t coln( line.find(":") );
			if ( coln == string::npos ) return;
			string name( line.substr( 0, coln ) );
			string value( line.substr( coln+1, line.size()-coln+1 ) );
			trim( name );
			trim( value );
			std::transform(name.begin(), name.end(), name.begin(), ::tolower); 
			map<string, string>& me( *this );
			me.insert( pair<string, string>( name, value ) );
		}
	};

	inline string mimevalue( const MimeHeaders& headers, string what ) 
	{
		MimeHeaders::const_iterator it=headers.find( what ); 
		if ( it != headers.end() )
			return it->second;
		return "";
	}


	struct Xml2HtmlEncoder : private map< char, string >
	{
		Xml2HtmlEncoder ( const string _text ) : text( _text ) 
		{
			Load(); // DO NOT VIRTUALIZE
		}

		private:

		void Load()
		{
			if ( ! empty() ) return;
			insert( pair< char, string> ('&', "&amp;" ) );
			insert( pair< char, string> ('<', "&lt;" ) );
			insert( pair< char, string> ('>', "&gt;" ) );
			insert( pair< char, string> ('"', "&quot;" ) );
			insert( pair< char, string> ('\'', "&apos;" ) );
		}

		string change( const char c ) const
		{
			stringstream ss;
			const_iterator found( find( c ) );
			if ( found == end() ) ss<<c;
			else ss<<found->second;
			return ss.str();
		}


		string EncodeXmlText() const
		{
			stringstream ss;
			for ( size_t i = 0; i < text.size(); i++ )
				ss <<  change( (char) text[ i ] );
			return ss.str().c_str();
		}
		const string text;
		friend ostream& operator<<( ostream&, const Xml2HtmlEncoder&);
		ostream& operator<<( ostream& o ) const
		{
			o << EncodeXmlText();
			return o;
		}
	};

	inline ostream& operator<<( ostream& o, const Xml2HtmlEncoder& x2h ) { return x2h.operator<<( o ); } 
} // Hyper

#endif // HYPERTOOLS_H

