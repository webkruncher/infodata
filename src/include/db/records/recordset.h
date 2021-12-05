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

#ifndef KRUNCHER_DB_DATASERVICE_RECORDS
#define KRUNCHER_DB_DATASERVICE_RECORDS

#include <infotools.h>
#include <directory.h>
#include <db_cxx.h>
#include <db/Database.h>

using namespace KruncherTools;
using namespace KruncherDirectory;
namespace DbRecords
{

	struct RecordSetBase {};

	template <class Data>
		struct RecordSet : Data
	{
		RecordSet( const string _datapath ) : datapath( _datapath ) {}
		virtual string DataFile() const
		{
			const string datafile( datapath + Data::TableName() );
			const size_t ls( datafile.find_last_of( "/" ) );
			if ( ls == string::npos ) throw Data::TableName();
			const string datapath( datafile.substr( 0, ls ) );
			KruncherDirectory::Directory d( datapath );
			d(0777);
			return datafile;
		}

		virtual const DBTYPE DatabaseType() const { return Data::DatabaseType(); }
		virtual const u_int32_t EnvironmentFlags() const { return Data::EnvironmentFlags(); }
		virtual const u_int32_t OpenFlags() const { return Data::OpenFlags(); }

		void operator()( BdbSpace::DbMethod& method, bool transact=false )
		{
			const string datafile( DataFile() );
			stringstream ssexcept;
			try
			{
				const u_int32_t envFlags( EnvironmentFlags() );
				BdbSpace::DataBaseEnvironment environment(  envFlags );
				if ( ! environment ) throw string("Can't create environment");
				const u_int32_t openFlags( OpenFlags()  );
				BdbSpace::DataBase< typename Data::KeyType, typename Data::ValueType> database( datafile.c_str(), environment, openFlags, DatabaseType() );
				if ( ! database.init() ) throw string( "Error initializing db" ); 

				const u_int32_t Flags( Data::DbFlags() );
				if ( Flags ) database.setflags( Flags );
				dup_compare_fcn comparator( Data::Comparator() );
				if ( comparator ) database.setDupCompare( comparator );

				if ( ! database.open() ) throw string( "Error opening db" ); 
				database.Crud( method, transact );
			}
			catch (const DbDeadlockException  &dbe) { ssexcept << "DbDeadlockException: " << dbe.what() << endl; }
			catch (const DbException &dbe) { ssexcept << "DbException: " << dbe.what() << endl; }
			catch (const string& e ) { ssexcept << "Exception: " << e << endl; }
			catch (const exception& e ) { ssexcept << "Std Exception: " << e.what() << endl; }
			catch (...) { ssexcept << "Unknown Exception: " << endl; }

			if ( ! ssexcept.str().empty() ) KruncherTools::Log( VERB_ALWAYS, "Database.h", ssexcept.str() );
		}

		private:
		mutable string datapath;
	};

	template <typename DataType>
		struct RecordPrinter
	{
		RecordPrinter( ostream& o, const string key, const string datapath )
			: method( o, key ), records( datapath ) 
		{
			records( method );
		}
		private:
		BdbSpace::QueryMethod<DataType> method;
		RecordSet<DataType> records;
	};

	template <typename DataType>
		struct RecordCreator
	{
		RecordCreator( const typename DataType::KeyType& _key, const typename DataType::ValueType& _value, const string datapath )
			: method( _key, _value ), records( datapath ) 
		{
			records( method );
		}
		private:
		BdbSpace::DbUpdateMethod< typename DataType::KeyType,  typename DataType::ValueType> method;
		RecordSet<DataType> records;
	};

	template <typename DataType>
		struct RecordUpdater
	{
		RecordUpdater( const typename DataType::KeyType& _key, const typename DataType::ValueType& _value, const string datapath )
			: umethod( _key, _value ), records( datapath ) 
		{
cout << "Look for " << _key << ", then update" << endl;
			records( umethod );
		}
		private:
		BdbSpace::DbUpdateMethod< typename DataType::KeyType,  typename DataType::ValueType> umethod;
		RecordSet<DataType> records;
	};
	
} // DbRecords

#endif  //KRUNCHER_DB_DATASERVICE_RECORDS


