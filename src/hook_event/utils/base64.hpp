#pragma once

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <vector>

std::string encode_base64(const std::vector<unsigned char> &input) {
  using namespace boost::archive::iterators;
  using base64_enc_iterator = base64_from_binary<
      transform_width<std::vector<unsigned char>::const_iterator, 6, 8>>;

  std::stringstream os;
  std::copy(
      base64_enc_iterator(input.begin()),
      base64_enc_iterator(input.end()),
      std::ostream_iterator<char>(os));

  // Add padding if needed
  size_t num_pad = (3 - input.size() % 3) % 3;
  for (size_t i = 0; i < num_pad; ++i) {
    os.put('=');
  }
  return os.str();
}