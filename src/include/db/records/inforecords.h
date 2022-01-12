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

#ifndef INFO_RECORDS_H
#define INFO_RECORDS_H
#include <infokruncher.h>
#include <infosite.h>
namespace RestData
{
	struct BindingBase
	{
		BindingBase( const InfoKruncher::SocketProcessOptions& _options, const Hyper::MimeHeaders& _headers, const string& _datapath ) 
			: options( _options ), headers( _headers ), datapath( _datapath ), Status( 0 ) {}
		pair< unsigned char*,size_t > operator()( const string& payload );
		operator unsigned long () { return Status; }
		private:
		virtual pair< unsigned char*,size_t > operator()( const string method, const string what, const stringvector& ) = 0;
		protected:
		const InfoKruncher::SocketProcessOptions& options;
		const Hyper::MimeHeaders& headers;
		const string datapath;
		unsigned long Status;
	};

	template< typename What >
		struct Binding : BindingBase, What
	{
		Binding( const InfoKruncher::SocketProcessOptions& _options, const Hyper::MimeHeaders& _headers, const string& _datapath ) 
			: BindingBase( _options, _headers, _datapath ) {}
		private:

		pair< unsigned char*,size_t > Results( const string query="", const bool integrity=false, const bool pass=false )
		{
			stringstream ssr;
			if ( integrity )
			{
				ssr << fence << boolalpha << pass << fence << query << What::record; 
			} else { 
				ssr << "KrestResult" << fence << What::record; 
			}
cerr << ssr.str() << endl;
			const size_t Len( ssr.str().size() );
			// TBD: Consider placement new / delete, Find ~/Info malloc
			unsigned char* data=(unsigned char*) malloc( Len );
			memset( data, 0, Len );
			memcpy( (char*) data, ssr.str().c_str(), Len );
			pair< unsigned char*,size_t > ret( data, Len );
			return ret;
		}

		pair< unsigned char*,size_t > CheckIntegrity( const string integrity, const string method, const string query, const stringvector& sv )
		{
			stringstream ss;
			struct Getter : DbRecords::RecordGetter<What>
			{
				Getter( What& _record, const string key, const string datapath, const stringvector& _sv )
					: DbRecords::RecordGetter<What>( key, datapath ), sv( _sv ), record( _record ), Same( false ) {} 
				virtual bool Hit( const typename What::KeyType& key, typename What::ValueType& value )
				{
					//cerr << "Loaded:" << blue << italic << "|" << key << "|" << value << normal << fence;
					//cerr << "Compare:" << endl << sv << endl;
					Same=record( value, sv );
					return false; // don't update
				}
				bool Same;
				private:
				const stringvector& sv;
				What& record;
			} getter( (*this), query, datapath, sv );
			const bool ok( !!getter );
			return Results( query, true, getter.Same );
		}

		pair< unsigned char*,size_t > operator()( const string method, const string query, const stringvector& sv )
		{
			const string Integrity( mimevalue( headers, "integrity" ) );

			if ( ! Integrity.empty() )
				return CheckIntegrity( Integrity, method, query, sv );

			What::operator=( sv );

			DbRecords::RecordUpdater<What> Update( query, What::record, options.datapath );
			const unsigned long nupdates( Update );

			if ( nupdates > 1 ) throw string( "ERROR - Multiple records" );
			if ( nupdates == 0 )
			{
				cerr << "Cannot update, creating" << endl;
				DbRecords::RecordCreator<What> Create( query, What::record, options.datapath );
				const unsigned long createstatus( Create );
				if ( createstatus ) 
				{
					cerr << red << "Cannot create" << normal << endl;
				} else {
					cerr << blue << "Created " << query << normal << endl;
					Status=200;
				}
			} else {
				cerr << blue << "Updated " << nupdates << " " << query << normal << endl;
				Status=200;
			}
			return Results();
		}
	};
} //RestData
#endif // INFO_RECORDS_H

