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

namespace ServiceXml
{
	using namespace XmlFamily;

	struct Item : XmlNode
	{
		friend struct Configuration;
		virtual XmlNodeBase* NewNode(Xml& _doc,XmlNodeBase* parent,stringtype name ) const
		{ 
			XmlNodeBase* ret(NULL);
			ret=new Item(_doc,parent,name,servicelist, optionnode, filter); 
			Item& n( static_cast<Item&>(*(ret)) );
			n.SetTabLevel( __tablevel+1 );
			return ret;
		}

		virtual bool operator()(ostream& o) { return XmlNode::operator()(o); }
		Item(Xml& _doc,const XmlNodeBase* _parent,stringtype _name, InfoKruncher::ServiceList& _servicelist, const string _optionnode, const string _filter ) 
			: XmlNode(_doc,_parent,_name ), servicelist( _servicelist ), optionnode( _optionnode ), filter( _filter )  {}
		operator bool () 
		{
			Load( NodeOptions );
			if ( Filtered() ) return true;
			if ( name == optionnode )
			{
				InfoKruncher::SocketProcessOptions o;
				Load( o );
				servicelist.push_back( o );
			}

			for (XmlFamily::XmlNodeSet::iterator it=children.begin();it!=children.end();it++) 
			{
				Item& n(static_cast<Item&>(*(*it)));
				n.NodeOptions=NodeOptions;
				if (!n) return false;
			}
			return true;
		}
	
		private:
		bool Filtered() const
		{
			if ( name == "filter" ) 
			{
				XmlFamily::XmlAttributes::const_iterator it( attributes.find( "hostname" ) );
				if ( it != attributes.end() )
					if ( filter != it->second ) return true;
			}
			return false;
		}
		void Load( InfoKruncher::SocketProcessOptions& options ) 
		{
			options=NodeOptions;
			XmlNode& node( *this );
			stringstream ss;
			ss <<  node ;
			options.text=ss.str();
			for(XmlFamily::XmlAttributes::const_iterator it=attributes.begin();
				it!=attributes.end();it++)
				{
					const string name( it->first.c_str() );
					const string value( it->second.c_str() );
					options( name, value );
				}
		}
		InfoKruncher::ServiceList& servicelist;
		InfoKruncher::SocketProcessOptions NodeOptions;
		const string optionnode;
		const string filter;
	};

	struct Configuration : Xml
	{
		Configuration( InfoKruncher::ServiceList& _servicelist, const string _optionnode, const string _filter ) 
			: servicelist( _servicelist ), optionnode( _optionnode ), filter( _filter ) {}
		virtual XmlNode* NewNode(Xml& _doc,stringtype name) const { return new Item(_doc,NULL,name, servicelist, optionnode, filter ); } 
		ostream& operator<<(ostream& o) const 
		{
			if ( ! Root ) return o;
			Item& nodes( static_cast<Item& >( *Root ) );
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
		InfoKruncher::ServiceList& servicelist;
		const string optionnode;
		const string filter;
	};
	inline ostream& operator<<(ostream& o,Configuration& xml){return xml.operator<<(o);}
} // ServiceXml




