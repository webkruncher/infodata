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



#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <errno.h>
#include <db_cxx.h>
using namespace std;
#include <Database.h>
#include <recordset.h>
#include <sys/stat.h>


using namespace BdbSpace;
using namespace DbRecords;

//#include <infokruncher.h>
//#include <infosite.h>

#include <Ticker.h>
using namespace StockMarket;
#include <inforecords.h>

namespace InfoMarketData
{

	pair< unsigned char*,size_t > MarketData::operator()( const string& payload ) 
	{	
		pair< unsigned char*,size_t > empty( nullptr, 0 );
		stringvector parts;
		parts.split( payload, "|" );
		const size_t npieces( parts.size() );
		if ( npieces < 3 ) return empty;

		int J( 1 );
		const string method( parts[ J++ ] );
		const string url( parts[ J++ ] );
		const string ticker( parts[ J++ ] );
		const string name( parts[ J++ ] );
		const string market( parts[ J++ ] );
		const string locale( parts[ J++ ] );
		const string primary_exchange( parts[ J++ ] );
		const string type( parts[ J++ ] );
		const string active( parts[ J++ ] );
		const string currency_name( parts[ J++ ] );
		const string cik( parts[ J++ ] );
		const string composite_figi( parts[ J++ ] );
		const string share_class_figi( parts[ J++ ] );
		const string last_updated( parts[ J++ ] );
		cout << 
			"method:" << method  << fence << "url:" << url  << fence << "ticker:" << ticker  << fence << "name:" << name  << fence << 
			"market:" << market  << fence << "locale:" << locale  << fence << "primary_exchange:" << primary_exchange  << fence << "type:" << type  << fence << 
			"active:" << active  << fence << "currency_name:" << currency_name  << fence << "cik:" << cik  << fence << "composite_figi:" << composite_figi  << fence << 
			"share_class_figi:" << share_class_figi  << fence << "last_updated:" << last_updated  << fence << endl;



		memset( &record, 0, sizeof( record ) );
		SetName( name );
		SetMarket( market );
		SetLocale( locale );
		SetPrimaryExchange( primary_exchange );
		SetType( type );
		SetActive( active );
		SetCurrencyName( currency_name );
		SetPrimaryCIK( cik );
		SetCompositeFigi( composite_figi );
		SetShareClassFigi( share_class_figi );
		SetLastUpdatedUTC( last_updated );
		DbRecords::RecordCreator<StockMarket::TickerBase> R( ticker, record, "./bdb/" );
		const unsigned long status( R );
		if ( R )
		{
			cerr << "Cannot create, updating" << endl;
			DbRecords::RecordUpdater<StockMarket::TickerBase> R( ticker, record, "./bdb/" );
			const unsigned long status( R );
			if ( R ) 
			{
				cerr << "Cannot update" << endl;
				stringstream ssr;
				ssr << "ERROR" << fence << payload;
				const size_t Len( ssr.str().size() );
				// TBD: Consider placement new / delete, Find ~/Info malloc
				unsigned char* data=(unsigned char*) malloc( Len );
				memset( data, 0, Len );
				memcpy( (char*) data, ssr.str().c_str(), Len );
				pair< unsigned char*,size_t > ret( data, Len );
				return ret;
			}
		}

		stringstream ssr;
		ssr << payload;
		const size_t Len( ssr.str().size() );
		// TBD: Consider placement new / delete, Find ~/Info malloc
		unsigned char* data=(unsigned char*) malloc( Len );
		memset( data, 0, Len );
		memcpy( (char*) data, ssr.str().c_str(), Len );
		pair< unsigned char*,size_t > ret( data, Len );
		return ret;
	}

} // namespace InfoMarketData

