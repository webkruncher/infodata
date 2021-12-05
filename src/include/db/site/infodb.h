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

#ifndef INFOKRUNCHER_DATA_H
#define INFOKRUNCHER_DATA_H
namespace InfoDb
{
	struct Roles
	{
		Roles( const InfoKruncher::Scheme _scheme, const string _uri, const stringmap& _headers, const in_addr& _ipaddr, const string& _text )
			: scheme( _scheme ), uri( _uri ), headers( _headers ), ipaddr( _ipaddr ), text( _text ) {}
		protected:
		const InfoKruncher::Scheme scheme;
		const string uri;
		const stringmap& headers;
		const in_addr& ipaddr;
		const string& text;

		virtual operator stringset&() = 0;
	};
	
	namespace Site
	{
		struct Roles : private InfoDb::Roles
		{
			Roles( InfoKruncher::Scheme _scheme, const string _uri, const stringmap& _headers, const in_addr& _ipaddr, const string& _text )
				: InfoDb::Roles( _scheme, _uri, _headers, _ipaddr, _text ) {}
			operator stringset&();
			stringset UserRoles;
		};
			
	} // Site


} // InfoDb


inline in_addr& assign( in_addr& a, const string& s )
{
	if ( s.empty() ) return a;
	inet_aton( s.c_str(), &a );
	return a;
}

#endif // INFOKRUNCHER_DATA_H
