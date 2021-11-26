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
#include <infoxmlauth.h>
#include <exexml.h>

namespace InfoAuth
{
	using namespace XmlFamily;

	struct Item : XmlNode
	{
		friend struct Configuration;
		virtual XmlNodeBase* NewNode(Xml& _doc,XmlNodeBase* parent,stringtype name ) const
		{ 
			XmlNodeBase* ret(NULL);
			ret=new Item(_doc,parent,name,authorization, filter); 
			Item& n=static_cast<Item&>(*(ret));
			n.SetTabLevel( __tablevel+1 );
			return ret;
		}
		virtual ostream& operator<<(ostream& o) const ;
		Item(Xml& _doc,const XmlNodeBase* _parent,stringtype _name, Authorization& _authorization, const stringset& _filter ) 
			: XmlNode(_doc,_parent,_name ), authorization( _authorization ), filter( _filter )  {}
		operator bool () 
		{

			for (XmlFamily::XmlNodeSet::iterator it=children.begin();it!=children.end();it++) 
			{
				Item& n=static_cast<Item&>(*(*it));
				if (!n) return false;
			}
			return true;
		}
	
		private:
		bool Filtered() const
		{
			XmlFamily::XmlAttributes::const_iterator found( attributes.find( "role" ) );
			if ( found == attributes.end() ) return false;
			const string NodeRole( found->second );
			if ( NodeRole.empty() ) 
			{
				if ( filter.empty() ) return false;
			} else return true;

			stringset NodeRoles;
			NodeRoles.split( NodeRole, "," );

			stringset intersection;
			set_intersection( NodeRoles.begin(), NodeRoles.end(), filter.begin(), filter.end(), inserter(intersection, intersection.begin()) );
			if ( intersection.empty() ) return true;
			return false;

		}
		Authorization& authorization;
		const stringset& filter;
	};
	inline ostream& operator<<(ostream& o,const Item& xmlnode) { return xmlnode.operator<<(o); }

	ostream& Item::operator<<(ostream& o)  const
	{
		if ( Filtered() ) return o;
		return XmlNode::operator<<(o);
	}

	struct Configuration : Xml
	{
		Configuration( Authorization& _authorization, const stringset& _filter ) : authorization( _authorization ), filter( _filter ) {}
		virtual XmlNode* NewNode(Xml& _doc,stringtype name) const { return new Item(_doc,NULL,name, authorization, filter ); } 
		ostream& operator<<(ostream& o) const 
		{
			if ( ! Root ) return o;
			const Item& nodes( static_cast< Item& >( *Root ) );
			o << nodes;
			return o;
		}
		operator Item& () { if (!Root) throw string("No root node"); return static_cast<Item&>(*Root); }

		operator bool() 
		{
			if ( ! Root ) return false;
			Item& item( static_cast< Item& >( *Root ) );
			return !!item;
		}

		private:
		Authorization& authorization;
		const stringset& filter;
	};
	inline ostream& operator<<(ostream& o,Configuration& xml){return xml.operator<<(o);}

	int Authorization::Authorize()
	{
		InfoAuth::Configuration xml( *this, roles );
		xml.Load( (char*)text.c_str() );
		if (  ! xml ) return 401;;
		stringstream ss;
		ss << xml;
		AuthorizedText=ss.str();
		return 200;
	}

	Authorization::operator int ()
	{
		if ( contenttype == string( "text/xml" ) )
		{
			const int authorized( Authorize() );
			//cerr << redbk << contenttype << fence << "Authorizing" <<  fence << greenbk << authorized << normal << endl;
			return authorized;
		} else {
			AuthorizedText=text;
			//cerr << red << contenttype << fence << "Allowing anyone" <<  normal << endl;
			return 200;
		} 
	}
} // InfoAuth


