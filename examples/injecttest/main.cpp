// Copyright (C) 2010-2015 Joshua Boyce
// See the file COPYING for copying permission.

#include <windows.h>

#include <hadesmem/config.hpp>
#include <hadesmem/detail/trace.hpp>

extern "C" __declspec(dllimport) DWORD_PTR InjectTestDep_Foo();

extern "C" __declspec(dllexport) DWORD_PTR Load()
{
  InjectTestDep_Foo();

  HADESMEM_DETAIL_TRACE_A("Called");

  return 0;
}

extern "C" __declspec(dllexport) DWORD_PTR Free()
{
  HADESMEM_DETAIL_TRACE_A("Called");

  return 0;
}

BOOL WINAPI
  DllMain(HINSTANCE /*instance*/, DWORD /*reason*/, LPVOID /*reserved*/)
{
  return TRUE;
}
