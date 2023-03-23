
#include <iostream>
#include <string>

#include <boost/algorithm/string.hpp>

#include <fc/io/json.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/variant.hpp>

#include <hive/utilities/key_conversion.hpp>

#include <hive/protocol/types.hpp>

struct signing_request
{
  fc::sha256                         dig;
  std::string                        wif;
};

struct signing_result
{
  fc::sha256                         dig;
  hive::protocol::public_key_type key;
  hive::protocol::signature_type  sig;
};

FC_REFLECT( signing_request, (dig)(wif) )
FC_REFLECT( signing_result, (dig)(key)(sig) )

int main(int argc, char** argv, char** envp)
{
  // hash key pairs on stdin
  std::string chain_id, hash, wif;
  while( std::cin )
  {
    std::string line;
    std::getline( std::cin, line );
    boost::trim(line);
    if( line == "" )
      continue;

    fc::variant v = fc::json::from_string( line, fc::json::strict_parser );
    signing_request sreq;
    fc::from_variant( v, sreq );
    signing_result sres;
    sres.dig = sreq.dig;
    fc::ecc::private_key priv_key = *fc::ecc::private_key::generate_from_base58( sreq.wif );
    sres.sig = priv_key.sign_compact( sreq.dig );
    sres.key = hive::protocol::public_key_type( priv_key.get_public_key() );
    std::cout << fc::json::to_string( sres ) << std::endl;
  }
  return 0;
}
