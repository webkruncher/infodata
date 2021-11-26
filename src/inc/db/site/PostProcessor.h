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

#ifndef INFOKRUNCHER_POSTPROCESSING_XML
#define INFOKRUNCHER_POSTPROCESSING_XML
namespace PostProcessingXml
{
	using namespace XmlFamily;

	void GetXmlPath( XmlFamily::XmlNode* node, stringstack& path )
	{
		if ( ! node ) return;
		path.push( node->name );
		GetXmlPath( (XmlNode*) node->Parent(), path );
	}


	struct Item : XmlNode
	{
		friend struct PostedXml;
		virtual XmlNodeBase* NewNode(Xml& _doc,XmlNodeBase* parent,stringtype name ) const
		{ 
			XmlNodeBase* ret(NULL);
			ret=new Item(_doc,parent,name,formdata, site );
			Item& n=static_cast<Item&>(*(ret));
			n.SetTabLevel( __tablevel+1 );
			return ret;
		}
		virtual bool operator()(ostream& o) { return XmlNode::operator()(o); }
		Item(Xml& _doc,const XmlNodeBase* _parent,stringtype _name, stringmap& _formdata, InfoKruncher::Site& _site ) 
			: XmlNode(_doc,_parent,_name ), formdata( _formdata ), site( _site )  {}
		operator bool()
		{
			if ( name == "Submit" ) 
			{
				stringstack path;
				GetXmlPath( (XmlNode*) Parent(), path );
				stringstream sspath; 
				while ( ! path.empty() )
				{
					sspath<<path.top() << "/";
					path.pop();
				}
				if (  ! site.ProcessForm( sspath.str(), formdata ) ) return false;
			} else {
				XmlFamily::XmlAttributes::const_iterator it( attributes.find( "input" ) );
				if ( ( it!=attributes.end() ) && ( it->second=="true" ) )
				{
					if ( !textsegments.empty() ) 
						formdata[ name ] = XmlFamilyUtils::StripCData( textsegments[ 0 ] );
				}
			}

			for (XmlFamily::XmlNodeSet::iterator it=children.begin();it!=children.end();it++) 
			{
				Item& n(static_cast<Item&>(*(*it)));
				if (!n) return false;
			}
			return true;
		}
	
		private:
		stringmap& formdata;
		InfoKruncher::Site& site;
	};


	struct PostedXml : Xml
	{
		PostedXml( stringmap& _formdata, InfoKruncher::Site& _site ) : formdata( _formdata ) , site( _site ) {}
		virtual XmlNode* NewNode(Xml& _doc,stringtype name) const { return new Item(_doc,NULL,name, formdata, site ); } 
		ostream& operator<<(ostream& o) const 
		{
			if ( ! Root ) return o;
			const Item& nodes( static_cast< const Item& >( *Root ) );
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
		stringmap& formdata;
		InfoKruncher::Site& site;
	};
	inline ostream& operator<<(ostream& o,PostedXml& xml){return xml.operator<<(o);}

} // PostProcessingXml
#endif // INFOKRUNCHER_POSTPROCESSING_XML
