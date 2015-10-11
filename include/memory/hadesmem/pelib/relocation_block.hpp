// Copyright (C) 2010-2015 Joshua Boyce
// See the file COPYING for copying permission.

#pragma once

#include <cstddef>
#include <ostream>
#include <utility>

#include <windows.h>
#include <winnt.h>

#include <hadesmem/config.hpp>
#include <hadesmem/error.hpp>
#include <hadesmem/pelib/nt_headers.hpp>
#include <hadesmem/pelib/pe_file.hpp>
#include <hadesmem/process.hpp>
#include <hadesmem/read.hpp>
#include <hadesmem/write.hpp>

// TODO: Add tests.

namespace hadesmem
{
class RelocationBlock
{
public:
  RelocationBlock(Process const& process,
                  PeFile const& pe_file,
                  PIMAGE_BASE_RELOCATION base,
                  void const* reloc_dir_end)
    : process_{&process},
      pe_file_{&pe_file},
      base_{reinterpret_cast<std::uint8_t*>(base)},
      reloc_dir_end_{reloc_dir_end}
  {
    UpdateRead();
  }

  RelocationBlock(Process&& process,
                  PeFile const& pe_file,
                  PIMAGE_BASE_RELOCATION base,
                  void const* reloc_dir_end) = delete;

  RelocationBlock(Process const& process,
                  PeFile&& pe_file,
                  PIMAGE_BASE_RELOCATION base,
                  void const* reloc_dir_end) = delete;

  RelocationBlock(Process&& process,
                  PeFile&& pe_file,
                  PIMAGE_BASE_RELOCATION base,
                  void const* reloc_dir_end) = delete;

  void* GetBase() const noexcept
  {
    return base_;
  }

  void UpdateRead()
  {
    data_ = Read<IMAGE_BASE_RELOCATION>(*process_, base_);
  }

  void UpdateWrite()
  {
    Write(*process_, base_, data_);
  }

  DWORD GetVirtualAddress() const noexcept
  {
    return data_.VirtualAddress;
  }

  void SetVirtualAddress(DWORD virtual_address) noexcept
  {
    data_.VirtualAddress = virtual_address;
  }

  DWORD GetSizeOfBlock() const noexcept
  {
    return data_.SizeOfBlock;
  }

  void SetSizeOfBlock(DWORD size_of_block) noexcept
  {
    data_.SizeOfBlock = size_of_block;
  }

  DWORD GetNumberOfRelocations() const noexcept
  {
    DWORD const size_of_block = GetSizeOfBlock();
    return size_of_block ? (static_cast<DWORD>(
                             (size_of_block - sizeof(IMAGE_BASE_RELOCATION)) /
                             sizeof(WORD)))
                         : 0;
  }

  PWORD GetRelocationDataStart() const noexcept
  {
    return reinterpret_cast<PWORD>(reinterpret_cast<std::uintptr_t>(base_) +
                                   sizeof(IMAGE_BASE_RELOCATION));
  }

  bool IsInvalid() const noexcept
  {
    auto const reloc_data = GetRelocationDataStart();
    void const* const reloc_data_end = reinterpret_cast<void const*>(
      reinterpret_cast<std::uintptr_t>(reloc_data) +
      (GetNumberOfRelocations() * sizeof(WORD)));
    return (reloc_data_end < reloc_data || reloc_data_end > reloc_dir_end_);
  }

private:
  Process const* process_;
  PeFile const* pe_file_;
  PBYTE base_;
  void const* reloc_dir_end_;
  IMAGE_BASE_RELOCATION data_ = IMAGE_BASE_RELOCATION{};
};

inline bool operator==(RelocationBlock const& lhs,
                       RelocationBlock const& rhs) noexcept
{
  return lhs.GetBase() == rhs.GetBase();
}

inline bool operator!=(RelocationBlock const& lhs,
                       RelocationBlock const& rhs) noexcept
{
  return !(lhs == rhs);
}

inline bool operator<(RelocationBlock const& lhs,
                      RelocationBlock const& rhs) noexcept
{
  return lhs.GetBase() < rhs.GetBase();
}

inline bool operator<=(RelocationBlock const& lhs,
                       RelocationBlock const& rhs) noexcept
{
  return lhs.GetBase() <= rhs.GetBase();
}

inline bool operator>(RelocationBlock const& lhs,
                      RelocationBlock const& rhs) noexcept
{
  return lhs.GetBase() > rhs.GetBase();
}

inline bool operator>=(RelocationBlock const& lhs,
                       RelocationBlock const& rhs) noexcept
{
  return lhs.GetBase() >= rhs.GetBase();
}

inline std::ostream& operator<<(std::ostream& lhs, RelocationBlock const& rhs)
{
  std::locale const old = lhs.imbue(std::locale::classic());
  lhs << rhs.GetBase();
  lhs.imbue(old);
  return lhs;
}

inline std::wostream& operator<<(std::wostream& lhs, RelocationBlock const& rhs)
{
  std::locale const old = lhs.imbue(std::locale::classic());
  lhs << rhs.GetBase();
  lhs.imbue(old);
  return lhs;
}
}
