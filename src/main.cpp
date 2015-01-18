//
//  main.cpp
//  uv
//
//  Created by Christopher Tarquini on 10/10/13.
//  Copyright (c) 2013 Christopher Tarquini. All rights reserved.
//

#include <iostream>
#include <vector>
#include <queue>
#include "eventloop.h"
#include <typeinfo>
#include <cxxabi.h>
#include "optional.h"
#include <codecvt>
#include <locale>
#include <string_bytes.h>
#include <codecvt>
#include "handle.h"
#include <chrono>
#include <deque>
#include "folly/config.h"
#include "folly/Optional.h"
#include "folly/ScopeGuard.h"
#include "boost/config.hpp"
#include <atomic>
#include "io_buffer.h"
#include <tq/experimental/optional.h>
#include <tq/experimental/dynarray.h>
#include "buffer_allocator_adaptor.h"
#include <deque>
#include <thread>
#include "folly/Likely.h"
#include "event.h"


#include "isolate.h"

std::string demangle(const char* name) {
  
  int status = -4; // some arbitrary value to eliminate the compiler warning
  
  // enable c++11 by passing the flag -std=c++11 to g++
  std::unique_ptr<char, void(*)(void*)> res {
    abi::__cxa_demangle(name, NULL, NULL, &status),
    std::free
  };
  
  return (status==0) ? res.get() : name ;
}

std::string fff("foo");
void f1(const std::string& str) {
  std::cout << str << "\n";
}
using ngn::events::scoped_connection;
using ngn::events::connection;
using ngn::experimental::Buffer;

int main(int argc, const char * argv[])
{
  std::string foo("hello world");
  Buffer buff(foo.size());
  std::copy(foo.begin(), foo.end(), buff.begin());
  for (auto& c : buff) {
    std::cout << c;
  }
  std::cout << std::endl << "done." << std::endl;
}




