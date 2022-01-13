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
		BindingBase( const InfoKruncher::SocketProcessOptions& _options )
			: options( _options ), Status( 0 ) {}
		virtual pair< unsigned char*,size_t > operator()( const string method, const string what, const Hyper::MimeHeaders&, const binarystring& ) = 0;
		operator unsigned long () { return Status; }
		private:
		protected:
		const InfoKruncher::SocketProcessOptions& options;
		unsigned long Status;
	};

	template< typename What >
		struct Binding : BindingBase, What
	{
		Binding( const InfoKruncher::SocketProcessOptions& _options )
			: BindingBase( _options ) {}
		private:

		pair< unsigned char*,size_t > Results( const string& results )
		{
			const size_t Len( results.size() );
			// TBD: Consider placement new / delete, Find ~/Info malloc
			unsigned char* data=(unsigned char*) malloc( Len );
			memset( data, 0, Len );
			memcpy( (char*) data, results.c_str(), Len );
			pair< unsigned char*,size_t > ret( data, Len );
			return ret;
		}


		struct Getter : DbRecords::RecordGetter<What>
		{
			Getter( What& _record, const string key, const string datapath, const stringvector& _sv )
				: DbRecords::RecordGetter<What>( key, datapath ), sv( _sv ), record( _record ), Same( false ) {} 
			virtual bool Hit( const typename What::KeyType& key, typename What::ValueType& value )
			{
				Same=record( value, sv );
				if ( ! Same ) 
				{
					stringstream ssvalue; ssvalue << fence << key << value;
					found=ssvalue.str();
				}
				return false; // don't update
			}
			bool Same;
			string found;
			private:
			const stringvector& sv;
			What& record;
		};

		pair< unsigned char*,size_t > operator()( const string method, const string table, const Hyper::MimeHeaders& headers, const binarystring& PostedContent )
		{
			if ( method == "GET" )
			{
				stringstream ssresults;
				if ( table.find( "/list" ) == table.size()-strlen( "/list" ) )
				{
					DbRecords::KeyLister<What> lister( options.datapath );
					lister( ssresults, "" );
				} else {
					size_t q( table.find( "?" ) );
					string query;
					if ( q != string::npos ) { q++; query=table.substr( q, table.size()-q ); }
					DbRecords::RecordPrinter<What> printer( options.datapath );
					printer( ssresults, query );
				}
				return Results( ssresults.str() );
			}



			const string Integrity( mimevalue( headers, "integrity" ) );

			const string payload( (char*) PostedContent.data(), PostedContent.size() );
			stringvector sv;
			sv.split( payload, "\n" );


			stringstream ssresults;
			DbRecords::RecordUpdater<What> Update( options.datapath );
			DbRecords::RecordCreator<What> Create( options.datapath );

			for ( stringvector::const_iterator sit=sv.begin();sit!=sv.end();sit++)
			{
				const string line( *sit );
				What& record( *this );
				stringvector fields; 
				fields.split( line, "|" );
				record=fields;
				const string query( fields[ 1 ] );


				if ( ! Integrity.empty() )
				{
					Getter getter( (*this), query, options.datapath, fields );
					const bool ok( getter );
					if ( getter.Same )
					{
							ssresults << "|" << query << "|200|" << endl;
					} else {
						if ( getter.found.empty() )
							ssresults << "|JRN" << line << endl << "|NUL|" << endl;
						else 
							ssresults << "|JRN" << line << endl << "|DBR" << getter.found << endl; 
					}
					continue;
				}


				const unsigned long nUpdates( Update( query, What::record ) );
				if ( nUpdates > 1 ) ssresults << fence << query << fence << 500 << fence << endl;
				if ( nUpdates == 1 ) ssresults << fence << query << fence << 200 << fence << endl;
				if ( nUpdates == 0 )
				{
					const unsigned long Created( ! Create( query, What::record ) );
					if ( Created ) ssresults << fence << query << fence << 201 << fence << endl; 
					else ssresults << fence << query << fence << 501 << fence << endl;
				} 
			}
			return Results( ssresults.str() );
		}
	};
} //RestData
#endif // INFO_RECORDS_H

