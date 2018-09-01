// Copyright (C) 2010-2015 Joshua Boyce
// See the file COPYING for copying permission.

#pragma once

#include <iterator>
#include <memory>
#include <utility>

#include <windows.h>

#include <hadesmem/config.hpp>
#include <hadesmem/detail/assert.hpp>
#include <hadesmem/detail/optional.hpp>
#include <hadesmem/error.hpp>
#include <hadesmem/pelib/import_dir.hpp>
#include <hadesmem/pelib/pe_file.hpp>
#include <hadesmem/process.hpp>
#include <hadesmem/read.hpp>

namespace hadesmem
{
// ImportDirIterator satisfies the requirements of an input iterator
// (C++ Standard, 24.2.1, Input Iterators [input.iterators]).
template <typename ImportDirT>
class ImportDirIterator
{
public:
  using BaseIteratorT = std::iterator_traits<ImportDirT*>;
  using value_type = typename BaseIteratorT::value_type;
  using difference_type = typename BaseIteratorT::difference_type;
  using pointer = typename BaseIteratorT::pointer;
  using reference = typename BaseIteratorT::reference;
  using iterator_category = std::input_iterator_tag;

  constexpr ImportDirIterator() noexcept
  {
  }

  explicit ImportDirIterator(Process const& process, PeFile const& pe_file)
  {
    try
    {
      ImportDir const import_dir{process, pe_file, nullptr};
      if (!IsTerminator(import_dir))
      {
        impl_ = std::make_shared<Impl>(process, pe_file, import_dir);
      }
    }
    catch (std::exception const& /*e*/)
    {
      // Nothing to do here.
    }
  }

  explicit ImportDirIterator(Process const&& process, PeFile const& pe_file) = delete;

  explicit ImportDirIterator(Process const& process, PeFile&& pe_file) = delete;

  explicit ImportDirIterator(Process const&& process, PeFile&& pe_file) = delete;

  reference operator*() const noexcept
  {
    HADESMEM_DETAIL_ASSERT(impl_.get());
    return *impl_->import_dir_;
  }

  pointer operator->() const noexcept
  {
    HADESMEM_DETAIL_ASSERT(impl_.get());
    return &*impl_->import_dir_;
  }

  ImportDirIterator& operator++()
  {
    try
    {
      HADESMEM_DETAIL_ASSERT(impl_.get());

      auto const cur_base = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(
        impl_->import_dir_->GetBase());
      impl_->import_dir_ =
        ImportDir{*impl_->process_, *impl_->pe_file_, cur_base + 1};

      if (IsTerminator(*impl_->import_dir_))
      {
        impl_.reset();
        return *this;
      }
    }
    catch (std::exception const& /*e*/)
    {
      impl_.reset();
    }

    return *this;
  }

  ImportDirIterator operator++(int)
  {
    ImportDirIterator const iter{*this};
    ++*this;
    return iter;
  }

  bool operator==(ImportDirIterator const& other) const noexcept
  {
    return impl_ == other.impl_;
  }

  bool operator!=(ImportDirIterator const& other) const noexcept
  {
    return !(*this == other);
  }

private:
  bool IsTerminator(ImportDir const& import_dir) const
  {
    // If the Name is NULL then the other fields can be non-NULL
    // but the entire entry will still be skipped by the Windows
    // loader.
    bool const has_name = !!import_dir.GetNameRaw();
    bool const has_iat = !!import_dir.GetFirstThunk();
    return (!has_name || !has_iat);
  }

  struct Impl
  {
    explicit Impl(Process const& process,
                  PeFile const& pe_file,
                  ImportDir const& import_dir) noexcept
      : process_{&process},
        pe_file_{&pe_file},
        import_dir_{import_dir}
    {
    }

    Process const* process_;
    PeFile const* pe_file_;
    hadesmem::detail::Optional<ImportDir> import_dir_;
  };

  // Shallow copy semantics, as required by InputIterator.
  std::shared_ptr<Impl> impl_;
};

class ImportDirList
{
public:
  using value_type = ImportDir;
  using iterator = ImportDirIterator<ImportDir>;
  using const_iterator = ImportDirIterator<ImportDir const>;

  explicit ImportDirList(Process const& process, PeFile const& pe_file)
    : process_{&process}, pe_file_{&pe_file}
  {
  }

  explicit ImportDirList(Process const&& process, PeFile const& pe_file) = delete;

  explicit ImportDirList(Process const& process, PeFile&& pe_file) = delete;

  explicit ImportDirList(Process const&& process, PeFile&& pe_file) = delete;

  iterator begin()
  {
    return iterator{*process_, *pe_file_};
  }

  const_iterator begin() const
  {
    return const_iterator{*process_, *pe_file_};
  }

  const_iterator cbegin() const
  {
    return const_iterator{*process_, *pe_file_};
  }

  iterator end() noexcept
  {
    return iterator{};
  }

  const_iterator end() const noexcept
  {
    return const_iterator{};
  }

  const_iterator cend() const noexcept
  {
    return const_iterator{};
  }

private:
  Process const* process_;
  PeFile const* pe_file_;
};
}
