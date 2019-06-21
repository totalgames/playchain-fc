#include <fc/exception/exception.hpp>
#include <boost/exception/all.hpp>
#include <fc/io/sstream.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/json.hpp>

#include <iostream>

namespace fc
{
   FC_IMPLEMENT_EXCEPTION( timeout_exception, timeout_exception_code, "Timeout" )
   FC_IMPLEMENT_EXCEPTION( file_not_found_exception, file_not_found_exception_code, "File Not Found" )
   FC_IMPLEMENT_EXCEPTION( parse_error_exception, parse_error_exception_code, "Parse Error" )
   FC_IMPLEMENT_EXCEPTION( invalid_arg_exception, invalid_arg_exception_code, "Invalid Argument" )
   FC_IMPLEMENT_EXCEPTION( key_not_found_exception, key_not_found_exception_code, "Key Not Found" )
   FC_IMPLEMENT_EXCEPTION( bad_cast_exception, bad_cast_exception_code, "Bad Cast" )
   FC_IMPLEMENT_EXCEPTION( out_of_range_exception, out_of_range_exception_code, "Out of Range" )
   FC_IMPLEMENT_EXCEPTION( method_not_found_exception, method_not_found_exception_code, "Method Not Found" );
   FC_IMPLEMENT_EXCEPTION( invalid_operation_exception, invalid_operation_exception_code, "Invalid Operation" )
   FC_IMPLEMENT_EXCEPTION( unknown_host_exception, unknown_host_exception_code, "Unknown Host" )
   FC_IMPLEMENT_EXCEPTION( canceled_exception, canceled_exception_code, "Canceled" )
   FC_IMPLEMENT_EXCEPTION( assert_exception, assert_exception_code, "Assert Exception" )
   FC_IMPLEMENT_EXCEPTION( eof_exception, eof_exception_code, "End Of File" )
   FC_IMPLEMENT_EXCEPTION( null_optional, null_optional_code, "null optional" )
   FC_IMPLEMENT_EXCEPTION( aes_exception, aes_error_code, "AES error" )
   FC_IMPLEMENT_EXCEPTION( overflow_exception, overflow_code, "Integer Overflow" )
   FC_IMPLEMENT_EXCEPTION( underflow_exception, underflow_code, "Integer Underflow" )
   FC_IMPLEMENT_EXCEPTION( divide_by_zero_exception, divide_by_zero_code, "Integer Divide By Zero" )

   FC_REGISTER_EXCEPTIONS( (timeout_exception)
                           (file_not_found_exception)
                           (parse_error_exception)
                           (invalid_arg_exception)
                           (invalid_operation_exception)
                           (key_not_found_exception)
                           (bad_cast_exception)
                           (out_of_range_exception)
                           (canceled_exception)
                           (assert_exception)
                           (eof_exception)
                           (unknown_host_exception)
                           (null_optional)
                           (aes_exception)
                           (overflow_exception)
                           (underflow_exception)
                           (divide_by_zero_exception)
                         )

   namespace detail
   {
      class exception_impl
      {
         public:
            std::string     _name;
            std::string     _what;
            int64_t         _code;
            log_messages    _elog;
      };
   }
   exception::exception( log_messages&& msgs, int64_t code,
                                    const std::string& name_value,
                                    const std::string& what_value )
   :my( new detail::exception_impl() )
   {
      my->_code = code;
      my->_what = what_value;
      my->_name = name_value;
      my->_elog = std::move(msgs);
   }

   exception::exception(
      const log_messages& msgs,
      int64_t code,
      const std::string& name_value,
      const std::string& what_value )
   :my( new detail::exception_impl() )
   {
      my->_code = code;
      my->_what = what_value;
      my->_name = name_value;
      my->_elog = msgs;
   }

   unhandled_exception::unhandled_exception( log_message&& m, std::exception_ptr e )
   :exception( std::move(m) )
   {
      _inner = e;
   }
   unhandled_exception::unhandled_exception( const exception& r )
   :exception(r)
   {
   }
   unhandled_exception::unhandled_exception( log_messages m )
   :exception()
   { my->_elog = std::move(m); }

   std::exception_ptr unhandled_exception::get_inner_exception()const { return _inner; }

   [[noreturn]] void unhandled_exception::dynamic_rethrow_exception()const
   {
      if( !(_inner == std::exception_ptr()) ) std::rethrow_exception( _inner );
      else { fc::exception::dynamic_rethrow_exception(); }
   }

   std::shared_ptr<exception> unhandled_exception::dynamic_copy_exception()const
   {
      auto e = std::make_shared<unhandled_exception>( *this );
      e->_inner = _inner;
      return e;
   }

   exception::exception( int64_t code,
                         const std::string& name_value,
                         const std::string& what_value )
   :my( new detail::exception_impl() )
   {
      my->_code = code;
      my->_what = what_value;
      my->_name = name_value;
   }

   exception::exception( log_message&& msg,
                         int64_t code,
                         const std::string& name_value,
                         const std::string& what_value )
   :my( new detail::exception_impl() )
   {
      my->_code = code;
      my->_what = what_value;
      my->_name = name_value;
      my->_elog.push_back( std::move( msg ) );
   }
   exception::exception( const exception& c )
   :my( new detail::exception_impl(*c.my) )
   { }
   exception::exception( exception&& c )
   :my( std::move(c.my) ){}

   const char*  exception::name()const throw() { return my->_name.c_str(); }
   const char*  exception::what()const throw() { return my->_what.c_str(); }
   int64_t      exception::code()const throw() { return my->_code;         }

   exception::~exception(){}

   void to_variant( const exception& e, variant& v, uint32_t max_depth )
   {
      FC_ASSERT( max_depth > 0, "Recursion depth exceeded!" );
      variant v_log;
      to_variant( e.get_log(), v_log, max_depth - 1 );
      mutable_variant_object tmp;
      tmp( "code", e.code() )
         ( "name", e.name() )
         ( "message", e.what() )
         ( "stack", v_log );
      v = variant( tmp, max_depth );

   }
   void from_variant( const variant& v, exception& ll, uint32_t max_depth )
   {
      FC_ASSERT( max_depth > 0, "Recursion depth exceeded!" );
      auto obj = v.get_object();
      if( obj.contains( "stack" ) )
         ll.my->_elog =  obj["stack"].as<log_messages>( max_depth - 1 );
      if( obj.contains( "code" ) )
         ll.my->_code = obj["code"].as_int64();
      if( obj.contains( "name" ) )
         ll.my->_name = obj["name"].as_string();
      if( obj.contains( "message" ) )
         ll.my->_what = obj["message"].as_string();
   }

   const log_messages&   exception::get_log()const { return my->_elog; }
   void                  exception::append_log( log_message m )
   {
      my->_elog.emplace_back( std::move(m) );
   }

   /**
    *   Generates a detailed string including file, line, method,
    *   and other information that is generally only useful for
    *   developers.
    */
   string exception::to_detail_string( log_level ll )const
   {
      fc::stringstream ss;
      ss << variant(my->_code).as_string() <<" " << my->_name << ": " <<my->_what<<"\n";
      for( auto itr = my->_elog.begin(); itr != my->_elog.end();  )
      {
         ss << itr->get_message() <<"\n";
         try
         {
            ss << "    " << json::to_string( itr->get_data() )<<"\n";
         }
         catch( const fc::assert_exception& e )
         {
            ss << "ERROR: Failed to convert log data to string!\n";
         }
         ss << "    " << itr->get_context().to_string();
         ++itr;
         if( itr != my->_elog.end() ) ss<<"\n";
      }
      return ss.str();
   }

   /**
    *   Generates a user-friendly error report.
    */
   string exception::to_string( log_level ll )const
   {
      fc::stringstream ss;
      ss << what() << ":";
      for( auto itr = my->_elog.begin(); itr != my->_elog.end(); ++itr )
      {
         if( itr->get_format().size() )
            ss << " " << fc::format_string( itr->get_format(), itr->get_data() );
      }
      return ss.str();
   }

   [[noreturn]] void exception_factory::rethrow( const exception& e )const
   {
      auto itr = _registered_exceptions.find( e.code() );
      if( itr != _registered_exceptions.end() )
         itr->second->rethrow( e );
      throw e;
   }
   /**
    * Rethrows the exception restoring the proper type based upon
    * the error code.  This is used to propagate exception types
    * across conversions to/from JSON
    */
   [[noreturn]] void exception::dynamic_rethrow_exception()const
   {
      exception_factory::instance().rethrow( *this );
   }

   exception_ptr exception::dynamic_copy_exception()const
   {
       return std::make_shared<exception>(*this);
   }

   std::string except_str()
   {
       return boost::current_exception_diagnostic_information();
   }

   void throw_bad_enum_cast( int64_t i, const char* e )
   {
      FC_THROW_EXCEPTION( bad_cast_exception,
                          "invalid index '${key}' in enum '${enum}'",
                          ("key",i)("enum",e) );
   }
   void throw_bad_enum_cast( const char* k, const char* e )
   {
      FC_THROW_EXCEPTION( bad_cast_exception,
                          "invalid name '${key}' in enum '${enum}'",
                          ("key",k)("enum",e) );
   }

   bool assert_optional(bool is_valid )
   {
      if( !is_valid )
         throw null_optional();
      return true;
   }
   exception& exception::operator=( const exception& copy )
   {
      *my = *copy.my;
      return *this;
   }

   exception& exception::operator=( exception&& copy )
   {
      my = std::move(copy.my);
      return *this;
   }

   void throw_assertion_failure( const std::string& message )
   {
      FC_THROW_EXCEPTION( fc::assert_exception, message );
   }

   void record_assert_trip(
      const char* filename,
      uint32_t lineno,
      const char* expr
      )
   {
      fc::mutable_variant_object assert_trip_info =
         fc::mutable_variant_object()
         ("source_file", filename)
         ("source_lineno", lineno)
         ("expr", expr)
         ;
      try
      {
         std::cout
            << "FC_ASSERT triggered:  "
            << fc::json::to_string( assert_trip_info ) << "\n";
      }
      catch( const fc::assert_exception& e )
      { // this should never happen. assert_trip_info is flat.
         std::cout << "ERROR: Failed to convert info to string?!\n";
      }
   }

   bool enable_record_assert_trip = false;

} // fc
