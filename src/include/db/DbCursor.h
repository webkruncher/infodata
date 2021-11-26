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

#ifndef DB_CURSOR_H
#define DB_CURSOR_H


namespace BdbSpace
{
	inline string dbflagbugger( const unsigned long flag )
	{
		switch ( flag )
		{
			case DB_SET: return "DB_SET" ; break;
			case DB_NEXT_DUP: return "DB_NEXT_DUP" ; break;
			case DB_NEXT_NODUP: return "DB_NEXT_NODUP" ; break;
			case DB_FIRST: return "DB_FIRST" ; break;
		}
		return "";
	}

	template <typename KeyType, typename ValueType >
		struct DbCursor
	{
		DbCursor( const unsigned long _startflag, const unsigned long _nextflag )
			: startflag( _startflag ), nextflag( _nextflag ) {}
		void operator()( Db& bdb, DbTxn* txn, const KeyType& key )
		{
			//cerr << dbflagbugger( startflag ) <<  "|" << dbflagbugger( nextflag ) << endl;
			Dbc *dbcp( NULL );
			Dbt data( NULL, sizeof( ValueType ) );
			Dbt K( (void*) &key, sizeof( KeyType ) );
			bdb.cursor(txn, &dbcp, 0 ); 
			if ( ! dbcp ) return;
			if (  dbcp->get(&K, &data, startflag )  != DB_NOTFOUND )  
				do hit( K, data );
					while ( dbcp->get(&K, &data, nextflag ) != DB_NOTFOUND ); 
			dbcp->close();
		}
		private:
		virtual void hit( const Dbt& key, const Dbt& value ) = 0;
		const unsigned long startflag;
		const unsigned long nextflag;
	};

	template <typename DataType>
		struct RecordCursor : BdbSpace::DbCursor< typename DataType::KeyType, typename DataType::ValueType >
	{
		RecordCursor( ostream& _o, const unsigned long startflag, const unsigned long nextflag ) : 
			BdbSpace::DbCursor< typename DataType::KeyType, typename DataType::ValueType>( startflag, nextflag ),
			o( _o )
		{}
		private:
		virtual void hit( const Dbt& key, const Dbt& value ) 
		{
			const typename DataType::KeyType& K( *static_cast<typename DataType::KeyType*>( key.get_data() ) );
			const typename DataType::ValueType& V( *static_cast<typename DataType::ValueType*>( value.get_data() ) );
			o << "|" << K << "|" << V << endl;
		}
		private:
		ostream& o;
	};


} //BdbSpace

#endif // DB_CURSOR_H

