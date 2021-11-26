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

#include <infokruncher.h>
#include <infosite.h>
#include <site/infodb.h>
#include <exexml.h>

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

		Roles::operator stringset& ()
		{
			if ( scheme != InfoKruncher::https )
			{
				UserRoles.clear();
				return UserRoles;
			}
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


