#pragma once
/**
 * @file log_message.hpp
 * @brief Defines types and helper macros necessary for generating log messages.
 */
#include <fc/config.hpp>
#include <fc/time.hpp>
#include <fc/variant_object.hpp>
#include <fc/shared_ptr.hpp>
#include <memory>
#include <string>

namespace fc
{
   namespace detail
   {
      class log_context_impl;
      class log_message_impl;
   }

   /**
    * Named scope for log_level enumeration.
    */
   class log_level
   {
      public:
         /**
          * @brief Define's the various log levels for reporting.
          *
          * Each log level includes all higher levels such that
          * Debug includes Error, but Error does not include Debug.
          */
         enum values
         {
            all,
            debug,
            info,
            warn,
            error,
            off
         };
         log_level( values v = off ):value(v){}
         explicit log_level( int v ):value( static_cast<values>(v)){}
         operator int()const { return value; }
         values value;
   };

   void to_variant( log_level e, variant& v, uint32_t max_depth = 1 );
   void from_variant( const variant& e, log_level& ll, uint32_t max_depth = 1 );

   /**
    *  @brief provides information about where and when a log message was generated.
    *  @ingroup AthenaSerializable
    *
    *  @see FC_LOG_CONTEXT
    */
   class log_context
   {
      public:
        log_context();
        log_context( log_level ll,
                     const char* file,
                     uint64_t line,
                     const char* method );
        ~log_context();
        explicit log_context( const variant& v, uint32_t max_depth );
        variant to_variant( uint32_t max_depth )const;

        uint64_t      get_msg_id()const;
        std::string   get_file()const;
        uint64_t      get_line_number()const;
        std::string   get_method()const;
        std::string   get_thread_name()const;
        std::string   get_task_name()const;
        std::string   get_host_name()const;
        time_point    get_timestamp()const;
        log_level     get_log_level()const;
        std::string   get_context()const;

        void          append_context( const std::string& c );

        std::string   to_string()const;
      private:
        std::shared_ptr<detail::log_context_impl> my;
   };

   void to_variant( const log_context& l, variant& v, uint32_t max_depth );
   void from_variant( const variant& l, log_context& c, uint32_t max_depth );

   /**
    *  @brief aggregates a message along with the context and associated meta-information.
    *  @ingroup AthenaSerializable
    *
    *  @note log_message has reference semantics, all copies refer to the same log message
    *  and the message is read-only after construction.
    *
    *  When converted to JSON, log_message has the following form:
    *  @code
    *  {
    *     "context" : { ... },
    *     "format"  : "string with ${keys}",
    *     "data"    : { "keys" : "values" }
    *  }
    *  @endcode
    *
    *  @see FC_LOG_MESSAGE
    */
   class log_message
   {
      public:
         log_message();
         /**
          *  @param ctx - generally provided using the FC_LOG_CONTEXT(LEVEL) macro
          */
         log_message( log_context ctx, std::string format, variant_object args = variant_object() );
         ~log_message();

         log_message( const variant& v, uint32_t max_depth );
         variant        to_variant(uint32_t max_depth)const;

         std::string    get_message()const;

         log_context    get_context()const;
         std::string    get_format()const;
         variant_object get_data()const;

      private:
         std::shared_ptr<detail::log_message_impl> my;
   };

   void    to_variant( const log_message& l, variant& v, uint32_t max_depth );
   void    from_variant( const variant& l, log_message& c, uint32_t max_depth );

   typedef std::vector<log_message> log_messages;


} // namespace fc

FC_REFLECT_TYPENAME( fc::log_message );

#ifndef __func__
#define __func__ __FUNCTION__
#endif

/**
 * @def FC_LOG_CONTEXT(LOG_LEVEL)
 * @brief Automatically captures the File, Line, and Method names and passes them to
 *        the constructor of fc::log_context along with LOG_LEVEL
 * @param LOG_LEVEL - a valid log_level::Enum name.
 */
#define FC_LOG_CONTEXT(LOG_LEVEL) \
   fc::log_context( fc::log_level::LOG_LEVEL, (const char*)__FILE__, __LINE__, (const char*)__func__ )

/**
 * @def FC_LOG_MESSAGE(LOG_LEVEL,FORMAT,...)
 *
 * @brief A helper method for generating log messages.
 *
 * @param LOG_LEVEL a valid log_level::Enum name to be passed to the log_context
 * @param FORMAT A const char* string containing zero or more references to keys as "${key}"
 * @param ...  A set of key/value pairs denoted as ("key",val)("key2",val2)...
 */
#define FC_LOG_MESSAGE( LOG_LEVEL, FORMAT, ... ) \
   fc::log_message( FC_LOG_CONTEXT(LOG_LEVEL), \
                    FORMAT, \
                    fc::limited_mutable_variant_object( FC_MAX_LOG_OBJECT_DEPTH, true )__VA_ARGS__ )

