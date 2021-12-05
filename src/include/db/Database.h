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

#include <infotools.h>
#include "DbCursor.h"
#include <kruncherbdb.h>
#ifndef DATABASE_OBJECTS
#define  DATABASE_OBJECTS
namespace BdbSpace
{
	using namespace KruncherTools;
	inline string& assign( string& a, const string& s )
	{
		return a;
	}

	struct DbMethod
	{
		virtual void operator() ( Db& db, DbEnv& env, DbTxn *txn ) const = 0;
	};

	template <typename KeyType, typename ValueType >
		struct CrudSupport 
	{
		void CrudLog( const string what ) const
		{
			stringstream ss;
			ss << "Crud" << fence <<  what <<  fence;
			Log( VERB_CRUD, "CrudSupport" , ss.str() );
		}

		void CrudLog( const string what, const KeyType& key, const ValueType& value ) const
		{
			stringstream ss;
			//ss << "Crud" << fence << what << fence;
			//ss << "Key size" << " " << sizeof( KeyType ) << " " << dotted( key ) << " "; 
			//ss << "Data size" << " " << sizeof( ValueType ) << " " << value;
			Log( VERB_CRUD, "CrudSupport" , ss.str() );
		}
	};

	template <typename KeyType, typename ValueType >
		struct DbUpdateMethod : CrudSupport< KeyType, ValueType >, DbMethod
	{
		DbUpdateMethod( const KeyType& _key, const ValueType& _value ) : key( _key ), value( _value ) {}
		private:
		virtual void operator () ( Db& db, DbEnv& env, DbTxn *txn )  const
		{
			CrudSupport<KeyType,ValueType>::CrudLog( "Create", key, value );
			Dbt K( (void*) &key, sizeof( KeyType ) );
			Dbt data( (void*) &value, sizeof( ValueType ) );
			db.put(txn, &K, &data, 0);
		}

		const KeyType& key;
		const ValueType& value;
	}; 

	struct DataBaseEnvironment
	{
		DataBaseEnvironment( const u_int32_t _envflags ) : envflags( _envflags ), envp( NULL ) {}
		operator bool ()
		{
			envp = new DbEnv( 0 );
			envp->open(".", envflags, 0);
			envp->set_lk_detect(DB_LOCK_MINWRITE); 
			return true;
		}
		~DataBaseEnvironment( )
		{
			if ( envp ) delete envp;
		}
		operator DbEnv& ()
		{
			if ( ! envp ) throw string("No environment");
			return *envp;
		}
		private:
		const u_int32_t envflags;
		DbEnv *envp;
	};

	template <typename KeyType, typename ValueType >
		struct DataBase
	{
		public:
			DataBase( const string _databasefname, DataBaseEnvironment& _environment, const u_int32_t _openFlags, DBTYPE _dbflags = DB_BTREE, u_int32_t _extraFlags = 0 ) 
				: databasefname( _databasefname ), environment( _environment ), dbp( NULL ) , openFlags( _openFlags ), dbflags( _dbflags ), extraFlags( _extraFlags ) {}
			virtual ~DataBase()
			{
				Close();
			}

			void Close()
			{
				if ( dbp )
				{
					dbp->close(0);
					delete dbp;
				}
			}

			bool setflags( const u_int32_t flags )
			{
				if ( ! dbp ) throw string("Db not allocated, can't set flags ");
				return ( ( dbp->set_flags( flags) ) == 0 );
			}

			int setDupCompare( dup_compare_fcn comparator)
			{
				if ( ! dbp ) throw string("Db not allocated, can't set comparator ");
				return dbp->set_dup_compare( comparator );
			}

			operator Db& ()
			{
				if ( ! dbp ) throw string("Db not initialized");
				return *dbp;
			} 


			virtual void operator()( const KeyType& key, const ValueType& value )  const
			{ 
				DbUpdateMethod<KeyType,ValueType> method( key, value );
				Crud( method, true );
			}

			bool Crud( DbMethod& method, const bool transact )  const
			{ 
				stringstream ssexcept;

				if ( ! dbp ) throw string("Db not initialized");

				DbEnv& env( environment );
				DbTxn *txn;
				bool retry = true;
				int retry_count = 0;
				while (retry) 
				{
					try 
					{

						txn = NULL;
						if ( transact ) env.txn_begin(NULL, &txn, 0);

						method( *dbp, env, txn );

						if ( transact )
						{
							try 
							{
								cerr << "Commit record" << endl;
								txn->commit(0);
								retry = false;
								txn = NULL;
							} catch (const DbException &e) {
								cerr << "\033[31m" << "Cannot commit, retry = " << retry << "\033[0m" << endl;
								ssexcept << "DbException" << e.what();
							}
						} else {
							retry=false;
						}
					} catch (const DbDeadlockException &e) {
						ssexcept << "DbDeadlockException" << e.what();
						if (txn != NULL)
							(void)txn->abort();

						if (retry_count < 10) {
							retry_count++;
							retry = true;
						} else {
							retry = false;
						}
					} catch (const DbException &e) {
						ssexcept << "DbException" << e.what();
						if (txn != NULL)
							txn->abort();
						retry = false;
					} catch ( const exception &e) {
						ssexcept << "exception" << e.what();
						return false;
					}
				}


				if ( ! ssexcept.str().empty() ) Log( VERB_ALWAYS, "Database.h", ssexcept.str() );
				return true;

			}

			bool init()
			{ 
				DbEnv& env( environment );
				dbp = new Db(&env, 0); 
				return true;
			}
			bool open()
			{
				dbp->set_error_stream(&cerr);
				dbp->set_errpfx(databasefname.c_str());
				dbp->set_pagesize(1024); 
				dbp->open(NULL, databasefname.c_str(), NULL, dbflags, openFlags, 0664);
				return true;
			}

			ostream& stats( ostream& o ) const
			{
				if ( dbflags & DB_BTREE )
				{
					DB_BTREE_STAT *statp; 
					dbp->stat(NULL, &statp, 0);
					o << (u_long)statp->bt_ndata << " records" << endl; 
					free(statp);
				}
				return o;
			}
		private:
			ostream& operator<< ( ostream& o ) const;
			const string databasefname;
			DataBaseEnvironment& environment;
			Db *dbp;
			const u_int32_t openFlags;
			const DBTYPE dbflags;
			const u_int32_t extraFlags;
	};

	template <typename DataType>
		struct CreateMethod : BdbSpace::DbMethod
	{
		CreateMethod( const typename DataType::KeyType& _key, const typename DataType::ValueType& _value )
			: key( _key ), value( _value ) {}
		private:
		virtual void operator()( Db& db, DbEnv& env, DbTxn *txn )  const
		{
			records( key, value );
		}
		private:
		const typename DataType::KeyType& key;
		const typename DataType::ValueType& value;
		
	}; 

	template <typename DataType>
		struct QueryMethod : BdbSpace::DbMethod
	{
		QueryMethod( ostream& _o, const string _key ) : o( _o ), key( _key ) {}
		private:
		virtual void operator()( Db& db, DbEnv& env, DbTxn *txn )  const
		{
			typename DataType::KeyType K;
			assign(K, key);
			if ( key.empty() )
			{
				BdbSpace::RecordCursor< DataType > cursor( o, DB_FIRST, DB_NEXT );
				cursor( db, txn, K );
			} else {
				BdbSpace::RecordCursor< DataType > cursor( o, DB_SET, DB_NEXT_DUP );
				cursor( db, txn, K );
			}
		}
		private:
		ostream& o;
		const string key;
	}; 

	struct DbMetaData
	{
		DbMetaData( const KruncherTools::Args& _args ) : args( _args ) {}
		operator bool ()
		{
			Args::const_iterator showit( args.find("--show" ) );
			if ( showit == args.end () ) return false;
			tablename=showit->second;
			if ( tablename.empty() ) return false;
			Args::const_iterator keyit( args.find("--key" ) );
			key=( ( keyit == args.end() ) ? "" : keyit->second );
			return true;
		}
		const string TableName() { return tablename; }
		const string Key() { return key; }
		private:
		string key, tablename;
		const KruncherTools::Args& args;
	};
} //BdbSpace
#endif //  DATABASE_OBJECTS

