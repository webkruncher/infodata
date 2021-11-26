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
#include <exexml.h>
#include <infofigur.h>
#include <directory.h>


namespace InfoKruncher
{
	bool ServiceList::operator()( const KruncherTools::Args& options)
	{
		KruncherTools::Args::const_iterator xmlname( options.find( "--xml" ) );
		if ( xmlname != options.end() )
		{
			KruncherTools::Args::const_iterator filterit( options.find( "--filter" ) );
			if ( filterit == options.end() ) throw string( "Use of --xml requires --filter option" );
			KruncherTools::Args::const_iterator nodeit( options.find( "--node" ) );
			if ( nodeit == options.end() ) throw string( "Use of --xml requires --node option" );
			const string optionnode( nodeit->second );

			Log("Loading", optionnode );
			
			ServiceXml::Configuration xml( *this, optionnode, filterit->second );
			const string xmltxt( KruncherDirectory::LoadFile( xmlname->second, 1000, 512 ) );
			if ( xmltxt.empty() ) return false;
			xml.Load( (char*)xmltxt.c_str() );
			if (  ! xml ) return false;;
			if ( options.find( "--check-config" ) != options.end() )
				cerr << xml << endl;
			return true;
		}
			
		return false;
	}



} //InfoKruncher


