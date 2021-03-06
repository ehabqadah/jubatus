// Jubatus: Online machine learning framework for distributed environment
// Copyright (C) 2011,2012 Preferred Networks and Nippon Telegraph and Telephone Corporation.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License version 2.1 as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef JUBATUS_SERVER_COMMON_GLOBAL_ID_GENERATOR_BASE_HPP_
#define JUBATUS_SERVER_COMMON_GLOBAL_ID_GENERATOR_BASE_HPP_

#include <stdint.h>
#include <string>

namespace jubatus {
namespace server {
namespace common {

class global_id_generator_base {
 public:
  virtual ~global_id_generator_base() {}

  virtual uint64_t generate() = 0;
};

}  // namespace common
}  // namespace server
}  // namespace jubatus

#endif  // JUBATUS_SERVER_COMMON_GLOBAL_ID_GENERATOR_BASE_HPP_
