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


#ifndef INFODATASERVICE_H
#define INFODATASERVICE_H

#include <recordset.h>
#include <db/site/infodb.h>
#include <visitors/visitor.h>

#include <exexml.h>
namespace InfoKruncher
{
	extern string ServiceName;
} // namespace InfoKruncher

namespace InfoDataService
{
	void SetupDB( const string datapath );
	void TeardownDB();
	struct Visitor : private DbRecords::RecordSetBase, Visitors::VisitorBase
	{
		Visitor() : NewVisitor( false ) { }
		private:

		string HostCookie( InfoKruncher::Responder& r, const string host ) 
		{
			const string Cookie( Hyper::mimevalue( r.headers, "cookie" ) );
			if ( Cookie.empty() ) return "";
			const size_t StartOfMyCookie( Cookie.find( host ) );
			if ( StartOfMyCookie == string::npos ) return "";
			size_t EndOfMyCookie( Cookie.find( ";", StartOfMyCookie ) );
			if ( EndOfMyCookie == string::npos ) EndOfMyCookie=Cookie.size();
			const string MyCookie( Cookie.substr( StartOfMyCookie, EndOfMyCookie-StartOfMyCookie ) );
			const size_t StartOfValue( Cookie.find( "=" ) );
			if ( StartOfValue == string::npos ) return "";
			const string Value(  MyCookie.substr( StartOfValue, Cookie.size() - StartOfValue ) );
			return Value.substr( 1, Value.size()-1 );
		}

		public:
		void operator+=( InfoKruncher::Responder& r )
		{
			SetIp( r.ipaddr );
			SetResource( r.resource );
			if ( r.options.scheme == InfoKruncher::http  ) SetCharacteristicsBit( VISITOR_CHARACTER_HTTP );
			if ( r.options.scheme == InfoKruncher::https ) SetCharacteristicsBit( VISITOR_CHARACTER_HTTPS );
			if ( r.method == "GET" ) SetCharacteristicsBit( VISITOR_CHARACTER_GET );
			if ( r.method == "POST" ) SetCharacteristicsBit( VISITOR_CHARACTER_POST );
			if ( r.method == "PUT" ) SetCharacteristicsBit( VISITOR_CHARACTER_PUT );
			if ( r.method == "DELETE" ) SetCharacteristicsBit( VISITOR_CHARACTER_DELETE );
			if ( r.method == "PATCH" ) SetCharacteristicsBit( VISITOR_CHARACTER_PATCH );


			const string host( Hyper::mimevalue( r.headers, "host" ) );
			if ( host.empty() ) return;
			cookiename=host;
			
			const string ExistingCookie( HostCookie( r, host ) );

			if ( ExistingCookie.empty() )
			{
				const string NewCookie( KruncherTools::GetUuid() );
				NewVisitor=true;
				{stringstream ssl; ssl<<"NewVisitor:" << NewCookie; Log( ssl.str() );}
				SetCookie( NewCookie );
			}  else
				SetCookie( ExistingCookie );

			//commit();
			//DbRecords::RecordCreator<Visitors::VisitorData>( r.ipaddr, hit, r.options.datapath  );
		}

		bool IsNewCookie() const { return NewVisitor; }
		string Cookie() const { return string( hit.cookie ); }
		string CookieName() const { return cookiename; }
		private:
		bool NewVisitor;
		string cookiename;
	};

	struct Resource
	{
		Resource( const InfoKruncher::Responder& _responder ) : data( NULL ), responder( _responder ) {}
		virtual ~Resource()
		{
			if ( data ) 
			{
				//Log( VERB_ALWAYS, "ERROR: Resource", "FreeData" );
				free( data );
			}
		}
		virtual operator int () = 0;
		string uri;
		string contenttype;
		stringstream payload;
		const bool IsBinary() const { return !!data; } 
		unsigned char* Data()
		{
			unsigned char* d( data );
			data=NULL;
			return d;
		}
		size_t DataLength() const { return datalength; }
		protected:
		unsigned char* data;
		size_t datalength;
		const InfoKruncher::Responder& responder;
		bool CookieCheck( const bool IsDefault, const Visitors::VisitorBase& );
		int HttpError( const int HttpErrorNumber );
	};

	struct DataResource : Resource
	{
		DataResource( const InfoKruncher::Responder& _responder, Visitors::VisitorBase& _visitor  ) 
			: 
				Resource( _responder ), 
				visitor( _visitor )
		{}
		virtual operator int ();
		protected:
		Visitors::VisitorBase& visitor;
	};
	
	inline DataResource::operator int ()
	{
		const bool IsDefault( responder.IsDefault() );
		uri= ( IsDefault  ? "index.html" : string(".") + responder.resource );
		contenttype=( Hyper::ContentType( uri ) );
		if ( ! CookieCheck( IsDefault, visitor ) )  return HttpError( 422 );

		const string filename( responder.options.path + uri );
		if ( ! FileExists( filename ) ) return HttpError( 404 );

		//Log( VERB_ALWAYS, "DataResource", filename );

		if ( ! IsDefault  )
		{
			const size_t fsize( FileSize( filename ) );
			data=(unsigned char*) malloc( fsize );
			if ( ! data ) throw filename;
			datalength=fsize;
			LoadBinaryFile( filename, data, fsize );
		} else {
			stringstream sfile;
			LoadFile( filename.c_str(), sfile );

			if ( contenttype == "text/html" )
			{
				Hyper::JavaScripter js(responder.options.path );
				js.split( sfile.str(), "\n" );
				if ( ! js ) payload << sfile.str();
				else payload << js;
			} else 
				payload<<sfile.str();
		}
		return 0;
	}




	inline int Resource::HttpError( const int HttpErrorNumber )
	{
		payload << "<html><h1>Error " << HttpErrorNumber <<  ", " << Hyper::statusText( HttpErrorNumber ) << "</h1></html>" << endl;
		contenttype="text/html";
		return HttpErrorNumber;
	}
	
	inline bool Resource::CookieCheck( const bool IsDefault, const Visitors::VisitorBase& visitor )
	{
		if ( 
			( ! IsDefault ) && 
			( visitor.IsNewCookie() )  &&
			( contenttype != "text/javascript" ) &&
			( contenttype != "application/xhtml+xml" ) &
			( contenttype != "text/css" )
		)
			return false;
		return true;
	}

} // InfoDataService

namespace InfoDb
{
	namespace Site
	{
		using namespace XmlFamily;

		struct Item : XmlNode
		{
			friend struct Configuration;
			virtual XmlNodeBase* NewNode(Xml& _doc,XmlNodeBase* parent,stringtype name ) const
			{ 
				XmlNodeBase* ret(NULL);
				ret=new Item(_doc,parent,name,authorization, filter); 
				nodemap( name, ret );
				Item& n=static_cast<Item&>(*(ret));
				n.SetTabLevel( __tablevel+1 );
				return ret;
			}

			Item(Xml& _doc,const XmlNodeBase* _parent,stringtype _name, Roles& _authorization, const string _filter ) 
				: XmlNode(_doc,_parent,_name ), authorization( _authorization ), filter( _filter )  {}

			void operator ()( stringstack path )
			{
				const string p( path.top() );
				path.pop();
				if ( path.empty() )
				{
					XmlFamily::XmlAttributes::const_iterator ipaddrit( attributes.find( "ipaddr" ) );
					XmlFamily::XmlAttributes::const_iterator roleit( attributes.find( "role" ) );
					if ( roleit != attributes.end() ) 
					{
						if ( ipaddrit != attributes.end() ) 
						{
							if ( ipaddrit->second == filter )
								authorization.UserRoles.split( roleit->second, ","  ); 
						}
					}
					return;
				} 
				const string next( path.top() );
				NodeMap::iterator nit( nodemap.find( next ) );
				if ( nit != nodemap.end() )
				{
					for ( vector<XmlNodeBase*>::iterator vit=nit->second.begin();vit!=nit->second.end();vit++)
					{
						XmlNodeBase* b( *vit );
						Item& n( static_cast<Item&>( *b ) );
						n( path );
					}
				}
			}
		
			private:
			Roles& authorization;
			const string filter;
			mutable NodeMap nodemap;
		};


		struct Configuration : Xml
		{
			Configuration( Roles& _authorization, const string _filter ) : authorization( _authorization ), filter( _filter ) {}
			virtual XmlNode* NewNode(Xml& _doc,stringtype name) const { return new Item(_doc,NULL,name, authorization, filter ); } 
			operator Item& () { if (!Root) throw string("No root node"); return static_cast<Item&>(*Root); }

			void operator ()( stringstack path)
			{
				if ( ! Root ) return;
				Item& item( static_cast< Item& >( *Root ) );
				item( path );
			}

			private:
			Roles& authorization;
			const string filter;
		};

		inline Roles::operator stringset& ()
		{
			if ( scheme != InfoKruncher::https )
			{
				UserRoles.clear();
				return UserRoles;
			}
cerr << "InfoDataService.h LoadXml" << endl;
			const string ip( dotted( ipaddr ) );
			Configuration xml( *this, ip );
			xml.Load( (char*) text.c_str() );
			stringstack path;
			path.split( "data/roles/user", "/" );
			xml( path );
			return UserRoles;
		}	
	
	} // Site
} // InfoDb

#endif //INFODATASERVICE_H

