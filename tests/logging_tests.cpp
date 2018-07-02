#include <boost/test/unit_test.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>

#include <fc/thread/thread.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/log/file_appender.hpp>
#include <fc/log/logger.hpp>
#include <fc/log/logger_config.hpp>
#include <fc/variant.hpp>
#include <fc/time.hpp>
#include <fc/io/json.hpp>

#include <thread>
#include <iostream>
#include <fstream>

BOOST_AUTO_TEST_SUITE(logging_tests)

BOOST_AUTO_TEST_CASE(log_reboot)
{
   BOOST_TEST_MESSAGE("Setting up logger");
   fc::file_appender::config conf;
   conf.filename = "/tmp/my.log";
   conf.format = "${timestamp} ${thread_name} ${context} ${file}:${line} ${method} ${level}]  ${message}";
   conf.flush = true;
   conf.rotate = true;
   conf.rotation_interval = fc::seconds(5); // rotate every 5 seconds
   conf.rotation_limit = fc::seconds(20); // Don't keep files older than 20 seconds
   conf.max_object_depth = 200;

   fc::file_appender fa( fc::variant(conf, 200) );

   BOOST_TEST_MESSAGE("Starting Loop");
   for(int i = 0; i < 30; i++)
   {
      fc::log_context ctx(fc::log_level::all, "my_file.cpp", 10, "my_method()");
      fc::log_message my_log_message( ctx, "${message}", {"message","This is a test"} );
      fa.log(my_log_message);
      fc::usleep(fc::seconds(1));
   }
   BOOST_TEST_MESSAGE("Loop complete");
}

BOOST_AUTO_TEST_SUITE_END()
