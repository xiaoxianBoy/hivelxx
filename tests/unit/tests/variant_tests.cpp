#ifdef IS_TEST_NET
#include <boost/test/unit_test.hpp>

#include <fc/variant.hpp>
#include <fc/variant_object.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/reflect/typename.hpp>
#include <fc/exception/exception.hpp>

#include <string>
#include <functional>
#include <limits>

// Used later in `reflected` tests
namespace variant_tests
{

struct A
{
  int a = 0;
};

struct B
{
  A a;
  int b = 0;
};

struct C
{
  B b;
};

}

// This should auto-generate from_variant and to_variant functions
FC_REFLECT( variant_tests::A, (a) )
FC_REFLECT( variant_tests::B, (a)(b) )

// Custom functions for C struct
namespace fc {

void from_variant( const variant& var, variant_tests::C& c )
{ from_variant( var, c.b ); }

void to_variant( const variant_tests::C& c, variant& var )
{ to_variant( c.b, var ); }

}

BOOST_AUTO_TEST_SUITE( variant_tests )

// Eliminates code redundancy wrapping the actual tests in error handling and logging
#define HIVE_AUTO_TEST_CASE( name, ... )           \
  BOOST_AUTO_TEST_CASE( name )                     \
  {                                                \
    try                                            \
    {                                              \
      BOOST_TEST_MESSAGE( "--- Testing: " #name ); \
      __VA_ARGS__                                  \
    }                                              \
    FC_LOG_AND_RETHROW()                           \
  }

// Pre-defined types

HIVE_AUTO_TEST_CASE( as_int64,
  int64_t initial = -1;

  fc::variant v = initial;
  BOOST_REQUIRE_EQUAL( v.get_type(), fc::variant::int64_type );

  BOOST_REQUIRE_EQUAL( v.as_int64(), initial );

  BOOST_REQUIRE( v.is_int64() );
  BOOST_REQUIRE( v.is_numeric() );
  BOOST_REQUIRE( v.is_integer() );
)

HIVE_AUTO_TEST_CASE( as_uint64,
  uint64_t initial = -1;

  fc::variant v = initial;
  BOOST_REQUIRE_EQUAL( v.get_type(), fc::variant::uint64_type );

  BOOST_REQUIRE_EQUAL( v.as_uint64(), initial );

  BOOST_REQUIRE( v.is_uint64() );
  BOOST_REQUIRE( v.is_numeric() );
  BOOST_REQUIRE( v.is_integer() );
)

HIVE_AUTO_TEST_CASE( as_bool,
  fc::variant v = true;
  BOOST_REQUIRE_EQUAL( v.get_type(), fc::variant::bool_type );
  BOOST_REQUIRE_EQUAL( v.as_bool(), true );

  // Make sure that true is not a false-positive result
  v = false;
  BOOST_REQUIRE_EQUAL( v.as_bool(), false );

  BOOST_REQUIRE( v.is_bool() );
  BOOST_REQUIRE( v.is_numeric() );
  BOOST_REQUIRE( v.is_integer() );
)

HIVE_AUTO_TEST_CASE( as_double,
  double initial = 3.141592;

  fc::variant v = initial;
  BOOST_REQUIRE_EQUAL( v.get_type(), fc::variant::double_type );

  BOOST_REQUIRE_EQUAL( v.as_double(), initial );

  BOOST_REQUIRE( v.is_double() );
  BOOST_REQUIRE( v.is_numeric() );
  BOOST_REQUIRE( !v.is_integer() );
)

HIVE_AUTO_TEST_CASE( as_blob,
  fc::blob initial;
  initial.data = { 'a', 'l', 'i', 'c', 'e' };

  fc::variant v = initial;
  fc::blob out = v.as_blob();
  BOOST_REQUIRE_EQUAL( v.get_type(), fc::variant::blob_type );

  BOOST_REQUIRE_EQUAL( out.data.size(), initial.data.size() );

  for( size_t i = 0; i < out.data.size(); ++i )
    BOOST_REQUIRE_EQUAL( out.data.at(i), initial.data.at(i) );

  BOOST_REQUIRE( v.is_blob() );
  BOOST_REQUIRE( !v.is_numeric() );
  BOOST_REQUIRE( !v.is_integer() );
)

HIVE_AUTO_TEST_CASE( as_string,
  std::string initial = "alice";

  fc::variant v = initial;
  BOOST_REQUIRE_EQUAL( v.get_type(), fc::variant::string_type );

  BOOST_REQUIRE_EQUAL( v.as_string(), initial );

  BOOST_REQUIRE( v.is_string() );
  BOOST_REQUIRE( !v.is_numeric() );
  BOOST_REQUIRE( !v.is_integer() );
)

HIVE_AUTO_TEST_CASE( null_type,
  fc::variant v;
  BOOST_REQUIRE_EQUAL( v.get_type(), fc::variant::null_type );

  v = 3.141592;
  v.clear();
  BOOST_REQUIRE_EQUAL( v.get_type(), fc::variant::null_type );

  BOOST_REQUIRE( v.is_null() );
  BOOST_REQUIRE( !v.is_numeric() );
  BOOST_REQUIRE( !v.is_integer() );
)

HIVE_AUTO_TEST_CASE( mutable_variant_object,
  fc::variant_object vo;
  BOOST_REQUIRE_EQUAL( vo.size(), 0 );

  fc::mutable_variant_object mvo;
  vo = mvo( "a", 1 )( "b", true ).set( "c", 3.14 );
  BOOST_REQUIRE_EQUAL( mvo[ "a" ].get_type(), fc::variant::int64_type );
  BOOST_REQUIRE_EQUAL( mvo[ "b" ].get_type(), fc::variant::bool_type );
  BOOST_REQUIRE_EQUAL( mvo[ "c" ].get_type(), fc::variant::double_type );

  BOOST_REQUIRE_EQUAL( vo.size(), 3 );

  // Throws on const object as it cannot add another element to the entries storage
  BOOST_REQUIRE_THROW( static_cast< const fc::mutable_variant_object >( mvo )[ "d" ], fc::key_not_found_exception );
  // But inserts variant to the storage if used on non-const object
  BOOST_REQUIRE_NO_THROW( mvo[ "d" ] );

  BOOST_REQUIRE_EQUAL( mvo.size(), 4 );

  mvo.erase( "d" );
  BOOST_REQUIRE_EQUAL( mvo.size(), 3 );

  BOOST_REQUIRE_EQUAL( mvo[ "a" ].as_int64(),  vo[ "a" ].as_int64() );
  BOOST_REQUIRE_EQUAL( mvo[ "b" ].as_bool(),   vo[ "b" ].as_bool() );
  BOOST_REQUIRE_EQUAL( mvo[ "c" ].as_double(), vo[ "c" ].as_double() );
  BOOST_REQUIRE_EQUAL( mvo[ "d" ].get_type(), fc::variant::null_type );

  // Random value test
  BOOST_REQUIRE_EQUAL( mvo[ "c" ].as_double(), 3.14 );
)

HIVE_AUTO_TEST_CASE( variant_object,
  fc::variant_object vo;
  BOOST_REQUIRE_EQUAL( vo.size(), 0 );

  fc::mutable_variant_object mvo;
  vo = mvo( "a", 1 )( "b", true ).set( "c", 3.14 );

  BOOST_REQUIRE_EQUAL( vo.size(), 3 );

  BOOST_REQUIRE( vo.contains( "c" ) );

  BOOST_REQUIRE_EQUAL( vo[ "a" ].get_type(), fc::variant::int64_type );
  BOOST_REQUIRE_EQUAL( vo[ "b" ].get_type(), fc::variant::bool_type );
  BOOST_REQUIRE_EQUAL( vo[ "c" ].get_type(), fc::variant::double_type );

  BOOST_REQUIRE_THROW( vo[ "d" ], fc::key_not_found_exception );

  // Random value test
  BOOST_REQUIRE_EQUAL( vo[ "c" ].as_double(), 3.14 );
)

HIVE_AUTO_TEST_CASE( object_type,
  fc::variant v = fc::variant_object{ "a", 3.14 };
  BOOST_REQUIRE_EQUAL( v.get_type(), fc::variant::object_type );

  const auto vo = v.get_object();
  BOOST_REQUIRE_EQUAL( vo.size(), 1 );

  BOOST_REQUIRE_EQUAL( vo.find( "a" )->value().get_type(), fc::variant::double_type );

  BOOST_REQUIRE_EQUAL( vo.find( "a" )->value().as_double(), vo.begin()->value().as_double() );

  BOOST_REQUIRE_EQUAL( vo[ "a" ].get_type(), fc::variant::double_type );

  // Test variant [] operator
  BOOST_REQUIRE_EQUAL( v[ "a" ].get_type(), fc::variant::double_type );

  BOOST_REQUIRE_EQUAL( v[ "a" ].as_double(), 3.14 );

  BOOST_REQUIRE( v.is_object() );
  BOOST_REQUIRE( !v.is_numeric() );
  BOOST_REQUIRE( !v.is_integer() );
)

HIVE_AUTO_TEST_CASE( array_type,
  fc::variants vs = {
    { 1 },
    { true },
    { 3.14 }
  };

  fc::variant v = vs;
  BOOST_REQUIRE_EQUAL( v.get_type(), fc::variant::array_type );
  BOOST_REQUIRE_EQUAL( v.size(), 3 );

  const auto variants_out = v.get_array();
  BOOST_REQUIRE_EQUAL( variants_out.size(), 3 );

  BOOST_REQUIRE_EQUAL( variants_out[ 0 ].get_type(), fc::variant::int64_type );
  BOOST_REQUIRE_EQUAL( variants_out[ 1 ].get_type(), fc::variant::bool_type );
  BOOST_REQUIRE_EQUAL( variants_out[ 2 ].get_type(), fc::variant::double_type );

  // Test variant [] operator
  BOOST_REQUIRE_EQUAL( v[ 1 ].get_type(), fc::variant::bool_type );

  // Random value test
  BOOST_REQUIRE_EQUAL( v[ 2 ].as_double(), 3.14 );

  BOOST_REQUIRE( v.is_array() );
  BOOST_REQUIRE( !v.is_numeric() );
  BOOST_REQUIRE( !v.is_integer() );
)

// Type conversions

namespace
{

template< typename TypeTested >
using variant_retrieve_member_fn_t = TypeTested (fc::variant::*)() const;

#define HIVE_TEST_VARIANT_FUNCTION_INVOKE(object, member_ptr) ((object).*(member_ptr))()

template< typename TypeTested >
void test_type_conversion( variant_retrieve_member_fn_t< TypeTested > variant_retrieve_fn )
{
  fc::variant v;
  TypeTested expected = std::numeric_limits< unsigned >::max();

  const auto type_conversion_helper = [&]( auto&& input ) {
    v = input;
    BOOST_REQUIRE_EQUAL( (HIVE_TEST_VARIANT_FUNCTION_INVOKE( v, variant_retrieve_fn )), expected );
  };

  type_conversion_helper( std::to_string( expected ) );
  type_conversion_helper( double( expected ) );
  type_conversion_helper( uint64_t( expected ) );
  type_conversion_helper( int64_t( expected ) );

  expected = true;
  type_conversion_helper( bool( expected ) );
  expected = false;
  type_conversion_helper( bool( expected ) );

  v.clear();
  BOOST_REQUIRE_EQUAL( (HIVE_TEST_VARIANT_FUNCTION_INVOKE( v, variant_retrieve_fn )), 0 );

  // Objects:
  v = fc::variant_object{};
  BOOST_REQUIRE_THROW( (HIVE_TEST_VARIANT_FUNCTION_INVOKE( v, variant_retrieve_fn )), fc::bad_cast_exception );
  v = fc::mutable_variant_object{};
  BOOST_REQUIRE_THROW( (HIVE_TEST_VARIANT_FUNCTION_INVOKE( v, variant_retrieve_fn )), fc::bad_cast_exception );

  // Arrays:
  v = fc::variants{};
  BOOST_REQUIRE_THROW( (HIVE_TEST_VARIANT_FUNCTION_INVOKE( v, variant_retrieve_fn )), fc::bad_cast_exception );

  // Blob:
  v = fc::blob{};
  BOOST_REQUIRE_THROW( (HIVE_TEST_VARIANT_FUNCTION_INVOKE( v, variant_retrieve_fn )), fc::bad_cast_exception );
}

}

HIVE_AUTO_TEST_CASE( as_int64_conversions,
  test_type_conversion< int64_t >( &fc::variant::as_int64 );
)

HIVE_AUTO_TEST_CASE( as_uint64_conversions,
  test_type_conversion< uint64_t >( &fc::variant::as_uint64 );
)

HIVE_AUTO_TEST_CASE( as_double_conversions,
  test_type_conversion< double >( &fc::variant::as_double );
)

HIVE_AUTO_TEST_CASE( as_bool_conversions,
  bool expected = true;
  fc::variant v;

  const auto type_conversion_helper = [&]( auto&& input ) {
    v = input;
    BOOST_REQUIRE_EQUAL( v.as_bool(), expected );
  };

  type_conversion_helper( uint64_t( expected ) );
  type_conversion_helper( int64_t( expected ) );
  type_conversion_helper( double( expected ) );

  type_conversion_helper( std::string{ "true" } );

  v.clear();
  BOOST_REQUIRE_EQUAL( v.as_bool(), false );

  expected = false;
  type_conversion_helper( uint64_t( expected ) );
  type_conversion_helper( int64_t( expected ) );
  type_conversion_helper( double( expected ) );

  type_conversion_helper( std::string{ "false" } );

  BOOST_REQUIRE_THROW( type_conversion_helper( std::string{ "alice" } ), fc::bad_cast_exception );

  // Objects:
  v = fc::variant_object{};
  BOOST_REQUIRE_THROW( v.as_bool(), fc::bad_cast_exception );
  v = fc::mutable_variant_object{};
  BOOST_REQUIRE_THROW( v.as_bool(), fc::bad_cast_exception );

  // Arrays:
  v = fc::variants{};
  BOOST_REQUIRE_THROW( v.as_bool(), fc::bad_cast_exception );

  // Blob:
  v = fc::blob{};
  BOOST_REQUIRE_THROW( v.as_bool(), fc::bad_cast_exception );
)

HIVE_AUTO_TEST_CASE( as_string_conversions,
  const uint64_t expected_integer = 123;
  std::string expected = std::to_string( expected_integer );
  fc::variant v;

  const auto type_conversion_helper = [&]( auto&& input ) {
    v = input;
    BOOST_REQUIRE_EQUAL( v.as_string(), expected );
  };

  type_conversion_helper( uint64_t( expected_integer ) );
  type_conversion_helper( int64_t( expected_integer ) );

  expected = std::to_string( double( expected_integer ) );
  v = double( expected_integer );
  BOOST_REQUIRE_EQUAL( v.as_string().substr( 0, expected.size() ), expected );

  expected = "true";
  type_conversion_helper( true );
  expected = "false";
  type_conversion_helper( false );

  v.clear();
  BOOST_REQUIRE( v.as_string().empty() );

  // Objects:
  v = fc::variant_object{};
  BOOST_REQUIRE_THROW( v.as_string(), fc::bad_cast_exception );
  v = fc::mutable_variant_object{};
  BOOST_REQUIRE_THROW( v.as_string(), fc::bad_cast_exception );

  // Arrays:
  v = fc::variants{};
  BOOST_REQUIRE_THROW( v.as_string(), fc::bad_cast_exception );

  // Blob:
  fc::blob blob;
  blob.data = { 'a', 'l', 'i', 'c', 'e' };
  v = blob;
  BOOST_REQUIRE_EQUAL( v.as_string(), "YWxpY2U==" );
)

HIVE_AUTO_TEST_CASE( as_blob_conversions,
  fc::blob blob;
  blob.data = { 'a', 'l', 'i', 'c', 'e' };
  fc::variant v = "YWxpY2U==";

  const auto compare_blob = []( const auto& lhs, const auto& rhs ) {
    BOOST_REQUIRE_EQUAL( lhs.data.size(), rhs.data.size() );

    for( size_t i = 0; i < lhs.data.size(); ++i )
      BOOST_REQUIRE_EQUAL( lhs.data.at(i), rhs.data.at(i) );
  };

  compare_blob( v.as_blob(), blob );

  v = std::string{ "alice", 5 };
  compare_blob( v.as_blob(), blob );

  v.clear();
  BOOST_REQUIRE_EQUAL( v.as_blob().data.size(), 0 );

  const auto create_vector = []( auto&& input ) {
    return std::vector<char>( (char*)&input, (char*)&input + sizeof(input) );
  };

  const auto create_and_compare = [&]( auto&& input ) {
    v = input;
    blob.data = create_vector( input );
    compare_blob( v.as_blob(), blob );
  };

  create_and_compare( int64_t( -1 ) );
  create_and_compare( uint64_t( -1 ) );
  create_and_compare( double( -1 ) );

  // Objects:
  v = fc::variant_object{};
  BOOST_REQUIRE_THROW( v.as_string(), fc::bad_cast_exception );
  v = fc::mutable_variant_object{};
  BOOST_REQUIRE_THROW( v.as_string(), fc::bad_cast_exception );

  // Arrays:
  v = fc::variants{};
  BOOST_REQUIRE_THROW( v.as_string(), fc::bad_cast_exception );
)

// Test variant construction from non-standard types
HIVE_AUTO_TEST_CASE( extended_variant_construction,
  using fc::variant;
  variant v;

  const auto test_type = [&]( auto&& value, variant::type_id type ) {
    v = { value };
    BOOST_REQUIRE_EQUAL( v.get_type(), type );
  };

  test_type( nullptr_t{}, variant::null_type );

  char c_arr[6] = { 'a', 'l', 'i', 'c', 'e', '\0' };
  test_type( c_arr, variant::string_type );
  test_type( static_cast< const char* >( c_arr ), variant::string_type );

  wchar_t wc_arr[6] = { 'a', 'l', 'i', 'c', 'e', '\0' };
  test_type( wc_arr, variant::string_type );
  test_type( static_cast< const wchar_t* >( wc_arr ), variant::string_type );

  test_type( fc::string{ c_arr }, variant::string_type );

  test_type( float{}, variant::double_type );

  test_type( int8_t{}, variant::int64_type );
  test_type( int16_t{}, variant::int64_type );
  test_type( int32_t{}, variant::int64_type );

  test_type( uint8_t{}, variant::uint64_type );
  test_type( uint16_t{}, variant::uint64_type );
  test_type( uint32_t{}, variant::uint64_type );
)

namespace
{

class variant_visitor : public fc::variant::visitor
{
private:
  fc::variant::type_id accept_type;

public:
  variant_visitor( fc::variant::type_id accept_type = fc::variant::null_type );

  variant_visitor& operator[]( fc::variant::type_id accept_type );

  virtual ~variant_visitor() = default;

  virtual void handle()const override;
  virtual void handle( const int64_t& )const override;
  virtual void handle( const uint64_t& )const override;
  virtual void handle( const double& )const override;
  virtual void handle( const bool& )const override;
  virtual void handle( const std::string& )const override;
  virtual void handle( const fc::variant_object& )const override;
  virtual void handle( const fc::variants& )const override;
};

variant_visitor::variant_visitor( fc::variant::type_id accept_type )
  : accept_type( accept_type )
{}

variant_visitor& variant_visitor::operator[]( fc::variant::type_id accept_type )
{
  this->accept_type = accept_type;
  return *this;
}

void variant_visitor::handle()const
{ BOOST_REQUIRE_EQUAL( accept_type, fc::variant::null_type ); }

void variant_visitor::handle( const int64_t& )const
{ BOOST_REQUIRE_EQUAL( accept_type, fc::variant::int64_type ); }

void variant_visitor::handle( const uint64_t& )const
{ BOOST_REQUIRE_EQUAL( accept_type, fc::variant::uint64_type ); }

void variant_visitor::handle( const double& )const
{ BOOST_REQUIRE_EQUAL( accept_type, fc::variant::double_type ); }

void variant_visitor::handle( const bool& )const
{ BOOST_REQUIRE_EQUAL( accept_type, fc::variant::bool_type ); }

void variant_visitor::handle( const std::string& )const
{ BOOST_REQUIRE_EQUAL( accept_type, fc::variant::string_type ); }

void variant_visitor::handle( const fc::variant_object& )const
{ BOOST_REQUIRE_EQUAL( accept_type, fc::variant::object_type ); }

void variant_visitor::handle( const fc::variants& )const
{ BOOST_REQUIRE_EQUAL( accept_type, fc::variant::array_type ); }

}

HIVE_AUTO_TEST_CASE( visitor,
  using fc::variant;
  variant v;

  variant_visitor hv;

  const auto test_visitor = [&]( auto&& value, variant::type_id required ) {
    v = value;
    v.visit( hv[ required ] );
  };

  test_visitor( nullptr_t{}, variant::null_type );
  test_visitor( std::string{}, variant::string_type );
  test_visitor( bool{}, variant::bool_type );
  test_visitor( int64_t{}, variant::int64_type );
  test_visitor( uint64_t{}, variant::uint64_type );
  test_visitor( double{}, variant::double_type );
  test_visitor( fc::variant_object{}, variant::object_type );
  test_visitor( fc::variants{}, variant::array_type );
)

// From variant, to variant

namespace
{

template< typename T >
void test_from_variant( const T& expected )
{
  fc::variant v;
  T result = T{};
  v = expected;
  fc::from_variant( v, result );
  BOOST_REQUIRE_EQUAL( expected, result );
}

template< typename T >
void test_to_variant( const T& expected )
{
  fc::variant v;
  fc::to_variant( expected, v );
  T result = T{};
  fc::from_variant( v, result );
  BOOST_REQUIRE_EQUAL( expected, result );
}

}

HIVE_AUTO_TEST_CASE( from_variant_base,
  test_from_variant( true );

  test_from_variant( uint8_t( -1 ) );
  test_from_variant( int8_t( -1 ) );
  test_from_variant( uint16_t( -1 ) );
  test_from_variant( int16_t( -1 ) );
  test_from_variant( uint32_t( -1 ) );
  test_from_variant( int32_t( -1 ) );
  test_from_variant( uint64_t( -1 ) );
  test_from_variant( int64_t( -1 ) );

  test_from_variant( double( 3.14 ) );

  test_from_variant( std::string{ "alice" } );
)

HIVE_AUTO_TEST_CASE( to_variant_base,
  test_to_variant( true );

  test_to_variant( uint8_t( -1 ) );
  test_to_variant( int8_t( -1 ) );
  test_to_variant( uint16_t( -1 ) );
  test_to_variant( int16_t( -1 ) );
  test_to_variant( uint32_t( -1 ) );
  test_to_variant( int32_t( -1 ) );
  test_to_variant( uint64_t( -1 ) );
  test_to_variant( int64_t( -1 ) );

  test_to_variant( double( 3.14 ) );

  test_to_variant( std::string{ "alice" } );
)

HIVE_AUTO_TEST_CASE( reflected,
  C c = { { { 1 }, 2 } };

  fc::variant v;
  fc::to_variant( c, v );
  BOOST_REQUIRE( v.is_object() );

  C c_out;
  fc::from_variant( v, c_out );

  BOOST_REQUIRE_EQUAL( c.b.a.a, c_out.b.a.a );
  BOOST_REQUIRE_EQUAL( c.b.b, c_out.b.b );
)

HIVE_AUTO_TEST_CASE( variant_negation,
  BOOST_REQUIRE( !fc::variant{ false } );
  BOOST_REQUIRE( !fc::variant{ 0 } );
)

HIVE_AUTO_TEST_CASE( variant_compare_eq,
  BOOST_REQUIRE( (fc::variant{ 1 } == fc::variant{ 1 }) );
  BOOST_REQUIRE( (fc::variant{ "alice" } == fc::variant{ "alice" }) );
  BOOST_REQUIRE( !(fc::variant{ 2 } == fc::variant{ 1 }) );
  BOOST_REQUIRE( !(fc::variant{ true } == fc::variant{ 1 }) );
  BOOST_REQUIRE( !(fc::variant{ "alice" } == fc::variant{ "bob" }) );
)

HIVE_AUTO_TEST_CASE( variant_compare_ne,
  BOOST_REQUIRE( !(fc::variant{ 1 } != fc::variant{ 1 }) );
  BOOST_REQUIRE( !(fc::variant{ "alice" } != fc::variant{ "alice" }) );
  BOOST_REQUIRE( (fc::variant{ 2 } != fc::variant{ 1 }) );
  BOOST_REQUIRE( (fc::variant{ true } != fc::variant{ 1 }) );
  BOOST_REQUIRE( (fc::variant{ "alice" } != fc::variant{ "bob" }) );
)

BOOST_AUTO_TEST_SUITE_END()
#endif
