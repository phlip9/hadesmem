// Copyright Joshua Boyce 2010-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// This file is part of HadesMem.
// <http://www.raptorfactor.com/> <raptorfactor@raptorfactor.com>

#include "hadesmem/pelib/pefile.hpp"

#define BOOST_TEST_MODULE pefile
#include "hadesmem/detail/warning_disable_prefix.hpp"
#include <boost/test/unit_test.hpp>
#include "hadesmem/detail/warning_disable_suffix.hpp"

#include "hadesmem/error.hpp"
#include "hadesmem/process.hpp"

// Boost.Test causes the following warning under GCC:
// error: base class 'struct boost::unit_test::ut_detail::nil_t' has a 
// non-virtual destructor [-Werror=effc++]
#if defined(HADESMEM_GCC)
#pragma GCC diagnostic ignored "-Weffc++"
#endif // #if defined(HADESMEM_GCC)

// Boost.Test causes the following warning under Clang:
// error: declaration requires a global constructor 
// [-Werror,-Wglobal-constructors]
#if defined(HADESMEM_CLANG)
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif // #if defined(HADESMEM_CLANG)

// TODO: Test PeFile stream overloads.

BOOST_AUTO_TEST_CASE(pefile)
{
  hadesmem::Process const process(::GetCurrentProcessId());
  
  hadesmem::pelib::PeFile pefile_1(&process, GetModuleHandle(nullptr), 
    hadesmem::pelib::PeFileType::Image);
  hadesmem::pelib::PeFile pefile_2(pefile_1);
  BOOST_CHECK_EQUAL(pefile_1, pefile_2);
  pefile_1 = pefile_2;
  BOOST_CHECK_EQUAL(pefile_1, pefile_2);
  hadesmem::pelib::PeFile pefile_3(std::move(pefile_2));
  BOOST_CHECK_EQUAL(pefile_1, pefile_3);
  hadesmem::pelib::PeFile pefile_4(pefile_1);
  pefile_1 = std::move(pefile_4);
  BOOST_CHECK_EQUAL(pefile_1, pefile_3);

  // TODO: Also test PeFileType::Data.
  // TODO: Actually test that RvaToVa is returning the correct value.
  hadesmem::pelib::PeFile const pefile_5(&process, GetModuleHandle(nullptr), 
    hadesmem::pelib::PeFileType::Image);
  BOOST_CHECK_EQUAL(pefile_5.GetBase(), reinterpret_cast<PVOID>(
    GetModuleHandle(nullptr)));
  BOOST_CHECK_EQUAL(static_cast<int>(pefile_5.GetType()), static_cast<int>(
    hadesmem::pelib::PeFileType::Image));
  BOOST_CHECK_EQUAL(RvaToVa(process, pefile_5, 0), 
    static_cast<PVOID>(nullptr));
}