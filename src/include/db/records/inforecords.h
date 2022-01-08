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
#include <Ticker.h>
namespace InfoMarketData
{
	struct MarketBase
	{
		MarketBase( const InfoKruncher::SocketProcessOptions& _options ) : options( _options ), Status( 0 ) {}
		pair< unsigned char*,size_t > operator()( const string& payload );
		operator unsigned long () { return Status; }
		private:
		virtual pair< unsigned char*,size_t > operator()( const string method, const string what, const stringvector& ) = 0;
		protected:
		const InfoKruncher::SocketProcessOptions& options;
		unsigned long Status;
	};

	template< typename What >
		struct MarketData : MarketBase, What
	{
		MarketData( const InfoKruncher::SocketProcessOptions& _options ) : MarketBase( _options ) {}
		private:
		pair< unsigned char*,size_t > operator()( const string method, const string what, const stringvector& sv )
		{

			//cout << record << endl;

			What::operator=( sv );

			DbRecords::RecordUpdater<What> Update( what, What::record, options.datapath );
			const unsigned long nupdates( Update );

			if ( nupdates > 1 ) throw string( "ERROR - Multiple records" );
			if ( nupdates == 0 )
			{
				cerr << "Cannot update, creating" << endl;
				DbRecords::RecordCreator<What> Create( what, What::record, options.datapath );
				const unsigned long createstatus( Create );
				if ( createstatus ) 
				{
					cerr << red << "Cannot create" << normal << endl;
				} else {
					cerr << blue << "Created " << what << normal << endl;
					Status=200;
				}
			} else {
				cerr << blue << "Updated " << nupdates << " " << what << normal << endl;
				Status=200;
			}

			stringstream ssr;
			ssr << "Krest" << fence << What::record;
			const size_t Len( ssr.str().size() );
			// TBD: Consider placement new / delete, Find ~/Info malloc
			unsigned char* data=(unsigned char*) malloc( Len );
			memset( data, 0, Len );
			memcpy( (char*) data, ssr.str().c_str(), Len );
			pair< unsigned char*,size_t > ret( data, Len );
			return ret;
		}
	};
} //InfoMarketData
#endif // INFO_RECORDS_H
