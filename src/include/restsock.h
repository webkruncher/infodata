



#ifndef SOCKETREST_H
#define SOCKETREST_H

#include <netdb.h>
#include <arpa/inet.h>
#include <openssl/bio.h>

namespace KruncherTools
{
     struct ResourceLocator
     {
          enum class LocatorField
          {
               Url, Text, Address
          };

          ResourceLocator( const string _line ) 
               : line( _line ), port( 80 ), SecureProto( false ), expectation( 0 ) {}

          ResourceLocator( const ResourceLocator& that ) 
               : 
                    line( that.line ),
                    port( that.port ), 
                    SecureProto( that.SecureProto ), 
                    method( that.method ), 
                    address( that.address ), 
                    url( that.url ), 
                    user( that.user ), 
                    password( that.password ), 
                    text( that.text ), 
                    expectation( that.expectation ) 
          {}
          ResourceLocator& operator=(const ResourceLocator& that )
          {
               if ( &that != this )
               {
                    line=that.line;
                    port=that.port;
                    SecureProto=that.SecureProto;
                    method=that.method;
                    address=that.address;
                    url=that.url;
                    user=that.user;
                    password=that.password;
                    text=that.text;
                    expectation=that.expectation;
               }
               return *this;
          }

          operator bool () const;

          

          private: 
          mutable string line;
          public:
          mutable int port;
          mutable bool SecureProto;
          mutable string method, address, url, user, password, text;
          mutable int expectation;
          virtual string TextTokens( LocatorField how, string what ) const = 0;
          virtual void ConfigureRequestHeaders( stringmap& headers, stringstream& ss ) const { }
          void urlsplit( string what ) const;
          friend ostream& operator<<( ostream&, const ResourceLocator& ) ;
          ostream& operator<<( ostream& o ) const
          {
               o << "Https: " << boolalpha << SecureProto << endl;
               o << "Method: " << method << endl;
               o << "Address: " << address << endl;
               o << "Port: " << port << endl;
               o << "Url: " << url << endl;
               o << "Text: " << text << endl;
               return o;
          }
     };
     inline ostream& operator<<( ostream& o, const ResourceLocator& k ) { return k.operator<<(o); } 

     struct Socketer
     {
          Socketer( 
                    const ResourceLocator& _locator, int& _HttpStatus, int& _Verbosity, string& _Results, 
                    stringmap& _RequestHeaders,stringmap& _ResponseHeaders, 
                    const bool _bVerify,
                    const string& _ca_dir, const string& _ca_file 
               ) : 
                    locator( _locator ), HttpStatus( _HttpStatus ), Verbosity( _Verbosity), Results( _Results), 
                    RequestHeaders( _RequestHeaders ), ResponseHeaders( _ResponseHeaders ), 
                    bVerify( _bVerify ),
                    ca_dir( _ca_dir ), ca_file( _ca_file ) 
          {}
          operator bool ();
          const int GetHttpStatus() { return HttpStatus; }
          protected:
          string& Results;
          stringmap& ResponseHeaders;
          stringmap& RequestHeaders;
          const bool bVerify;
          const string& ca_dir;
          const string& ca_file;
          private:
          int GetHttp( const ResourceLocator& locator, stringstream& ssPayload );
          int GetHttps( const ResourceLocator& locator, stringstream& ssPayload );
          void LoadHeaders( stringstream& ss );
          void LoadSsl( BIO* bio, stringstream& Payload );
          void LoadHttp( int sock, stringstream& Payload );
          size_t ContentLength();
          const ResourceLocator& locator;
          int& HttpStatus;
          int& Verbosity;
     };

     struct RestfulBase
     {
          RestfulBase(
               const int _Verbosity,
               const bool _bVerify,
               const string _ca_dir,
               const string _ca_file
          ) :
                    HttpStatus( 0 ), 
                    Verbosity( _Verbosity ),
                    bVerify( _bVerify ),
                    ca_dir( _ca_dir ), 
                    ca_file( _ca_file ) 
          {}
          virtual operator ResourceLocator& () = 0;
          stringmap RequestHeaders,ResponseHeaders;
          int HttpStatus;
          int Verbosity;
          const bool bVerify;
          const string ca_dir;
          const string ca_file;
          string Results;
          friend ostream& operator<<( ostream&, const RestfulBase& );
          virtual ostream& operator<<( ostream& o ) const = 0;
     };
     inline ostream& operator<<( ostream& o, const RestfulBase& k ) { return k.operator<<(o); } 

     template <typename LOCATOR>
	     struct Restful : RestfulBase
     {
          Restful( LOCATOR _locator, const int _Verbosity=0, const bool _bVerify=false, const string _ca_dir="", const string _ca_file="" ) 
               : 
                    locator( _locator ), RestfulBase( _Verbosity, _bVerify, _ca_dir, _ca_file )
          {}
          virtual operator bool ()
          {
               Socketer sock( locator, HttpStatus, Verbosity, Results, RequestHeaders, ResponseHeaders, bVerify, ca_dir, ca_file ); 
               if ( !sock ) 
               {
                    return false;
               } else {
                    if ( locator.expectation == 0 ) return true;
                    else return HttpStatus == locator.expectation;
               }
          }
          ostream& operator<<( ostream& o ) const
          { 
               if ( Verbosity == 0 ) return o;
               const int status( HttpStatus );
               if ( locator.expectation!=HttpStatus ) o << redbk << yellow << rvid << ulin << HttpStatus << "!=" << locator.expectation << normal << endl;
               o << bold;
               if ( status < 200 )  o << redbk << yellow;
               if ( status == 200 )  o << green;
               if ( ( status >= 400 ) && ( status < 500 )  )  o << red;
               if ( ( status >= 300 ) && ( status < 400 )  )  o << yellow;
               o << status << tab << locator.url << normal << endl;
               if ( Verbosity < 2 ) return o;
               o << green << bold << RequestHeaders << normal << endl;
               if ( Verbosity < 3 ) return o;
               o << black << whitebk << ResponseHeaders << normal << endl;
               if ( Verbosity < 4 ) return o;
               o << whitebk << blue << Results<< normal << endl << endl; 
               return o;
          }
          virtual operator ResourceLocator& () { return locator; }
          LOCATOR locator;
     };


     struct CookieHolder : stringmap
     {
          void operator+=( const string cooks )
          {
               stringmap& me( *this );
               stringvector sv;
               sv.split( cooks, ";" );
               for ( stringvector::iterator it=sv.begin(); it!=sv.end(); it++ ) 
               {
                    string Cookie( *it );
                    const size_t fns( Cookie.find_first_not_of( " \r\n\t " ) );
                    if ( fns != string::npos ) Cookie.erase( 0, fns );
                    const size_t eq( Cookie.find( "=" ) );
                    if ( eq == string::npos )
                    {
                         me[ Cookie ] = "";
                    } else {
                         const string name( Cookie.substr( 0, eq ) );
                         const size_t eqp( eq+1 );
                         const string value( Cookie.substr( eqp, Cookie.size() - eqp ) );
                         me[ name ] = value;
                    } 
               }
               
          }
          private:
          friend ostream& operator<<(ostream&, const CookieHolder&);
          ostream& operator<<(ostream& o) const
          {
               for ( const_iterator it=begin(); it!=end(); it++ ) 
		       { o << it->first << ":" << it->second << endl; }
               return o;
          }
     };
     inline ostream& operator<<(ostream& o, const CookieHolder& k) { return k.operator<<(o);}

} // namespace KruncherTools

#endif // SOCKETREST_H


