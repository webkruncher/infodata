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

	struct Record : private DbRecords::RecordSetBase, StockMarket::TickerBase
	{
		void Create( const string symbol, const string name, const string market )
		{
			memset( &record, 0, sizeof( record ) );
			SetName( name );
			SetMarket( market );
			SetLastUpdatedUTC( time( 0 ) );
			DbRecords::RecordCreator<StockMarket::TickerBase> R( symbol, record, "./db/" );
		}

		void Update( const string symbol, const string name, const string market )
		{
			memset( &record, 0, sizeof( record ) );
			SetName( name );
			SetMarket( market );
			SetLastUpdatedUTC( time( 0 ) );
			DbRecords::RecordUpdater<StockMarket::TickerBase> R( symbol, record, "./db/" );
		}

		void Retrieve( const string symbol="" )
		{
			memset( &record, 0, sizeof( record ) );
			DbRecords::RecordPrinter<StockMarket::TickerBase> R( cout, symbol, "./db/"  );
		}
	};

	pair< unsigned char*,size_t > MarketData::operator()( const string& payload ) 
	{	
		string tickersymbol( "WIP" );
		Record record;
		if ( false )
		{
			record.Update( tickersymbol, tickersymbol + string(" updated" ), "updr" );
		} else {
			record.Create( tickersymbol, tickersymbol + string(" created" ), "newr" );
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

