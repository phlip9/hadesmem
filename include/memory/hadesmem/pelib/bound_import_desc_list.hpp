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
#include <hadesmem/pelib/bound_import_desc.hpp>
#include <hadesmem/pelib/pe_file.hpp>
#include <hadesmem/process.hpp>
#include <hadesmem/read.hpp>

// TODO: Add tests.

namespace hadesmem
{
// BoundImportDescriptorIterator satisfies the requirements of an input iterator
// (C++ Standard, 24.2.1, Input Iterators [input.iterators]).
template <typename BoundImportDescriptorT>
class BoundImportDescriptorIterator
{
public:
  using BaseIteratorT = std::iterator_traits<BoundImportDescriptorT*>;
  using value_type = typename BaseIteratorT::value_type;
  using difference_type = typename BaseIteratorT::difference_type;
  using pointer = typename BaseIteratorT::pointer;
  using reference = typename BaseIteratorT::reference;
  using iterator_category = std::input_iterator_tag;

  constexpr BoundImportDescriptorIterator() noexcept
  {
  }

  explicit BoundImportDescriptorIterator(Process const& process,
                                         PeFile const& pe_file)
  {
    try
    {
      BoundImportDescriptor const bound_import_desc{
        process, pe_file, nullptr, nullptr};
      if (!IsTerminator(bound_import_desc))
      {
        impl_ = std::make_shared<Impl>(process, pe_file, bound_import_desc);
      }
    }
    catch (std::exception const& /*e*/)
    {
      // Nothing to do here.
    }
  }

  explicit BoundImportDescriptorIterator(Process const&& process,
                                         PeFile const& pe_file) = delete;

  explicit BoundImportDescriptorIterator(Process const& process,
                                         PeFile&& pe_file) = delete;

  explicit BoundImportDescriptorIterator(Process const&& process,
                                         PeFile&& pe_file) = delete;

  reference operator*() const noexcept
  {
    HADESMEM_DETAIL_ASSERT(impl_.get());
    return *impl_->bound_import_desc_;
  }

  pointer operator->() const noexcept
  {
    HADESMEM_DETAIL_ASSERT(impl_.get());
    return &*impl_->bound_import_desc_;
  }

  BoundImportDescriptorIterator& operator++()
  {
    try
    {
      HADESMEM_DETAIL_ASSERT(impl_.get());

      auto const cur_base = static_cast<PIMAGE_BOUND_IMPORT_DESCRIPTOR>(
        impl_->bound_import_desc_->GetBase());
      auto const new_base = reinterpret_cast<PIMAGE_BOUND_IMPORT_DESCRIPTOR>(
        reinterpret_cast<IMAGE_BOUND_FORWARDER_REF*>(cur_base + 1) +
        impl_->bound_import_desc_->GetNumberOfModuleForwarderRefs());
      auto const start_base = static_cast<PIMAGE_BOUND_IMPORT_DESCRIPTOR>(
        impl_->bound_import_desc_->GetStart());
      impl_->bound_import_desc_ = BoundImportDescriptor{
        *impl_->process_, *impl_->pe_file_, start_base, new_base};

      if (IsTerminator(*impl_->bound_import_desc_))
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

  BoundImportDescriptorIterator operator++(int)
  {
    BoundImportDescriptorIterator const iter{*this};
    ++*this;
    return iter;
  }

  bool operator==(BoundImportDescriptorIterator const& other) const noexcept
  {
    return impl_ == other.impl_;
  }

  bool operator!=(BoundImportDescriptorIterator const& other) const noexcept
  {
    return !(*this == other);
  }

private:
  // TODO: Verify this.
  bool IsTerminator(BoundImportDescriptor const& bound_import_desc) const
  {
    // Apparently all three fields are supposed to be zero, but it seems that
    // may not be the case when it comes to the actual loader implementation?
    return !bound_import_desc.GetTimeDateStamp() ||
           !bound_import_desc.GetOffsetModuleName();
  }

  struct Impl
  {
    explicit Impl(Process const& process,
                  PeFile const& pe_file,
                  BoundImportDescriptor const& bound_import_desc) noexcept
      : process_{&process},
        pe_file_{&pe_file},
        bound_import_desc_{bound_import_desc}
    {
    }

    Process const* process_;
    PeFile const* pe_file_;
    hadesmem::detail::Optional<BoundImportDescriptor> bound_import_desc_;
  };

  // Shallow copy semantics, as required by InputIterator.
  std::shared_ptr<Impl> impl_;
};

class BoundImportDescriptorList
{
public:
  using value_type = BoundImportDescriptor;
  using iterator = BoundImportDescriptorIterator<BoundImportDescriptor>;
  using const_iterator =
    BoundImportDescriptorIterator<BoundImportDescriptor const>;

  explicit BoundImportDescriptorList(Process const& process,
                                     PeFile const& pe_file)
    : process_{&process}, pe_file_{&pe_file}
  {
  }

  explicit BoundImportDescriptorList(Process const&& process,
                                     PeFile const& pe_file) = delete;

  explicit BoundImportDescriptorList(Process const& process,
                                     PeFile&& pe_file) = delete;

  explicit BoundImportDescriptorList(Process const&& process,
                                     PeFile&& pe_file) = delete;

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
