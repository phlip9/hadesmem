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
#include <hadesmem/pelib/bound_import_fwd_ref.hpp>
#include <hadesmem/pelib/pe_file.hpp>
#include <hadesmem/process.hpp>
#include <hadesmem/read.hpp>

// TODO: Add tests.

namespace hadesmem
{
// BoundImportForwarderRefIterator satisfies the requirements of an input
// iterator (C++ Standard, 24.2.1, Input Iterators [input.iterators]).
template <typename BoundImportForwarderRefT>
class BoundImportForwarderRefIterator
{
public:
  using BaseIteratorT = std::iterator_traits<BoundImportForwarderRefT*>;
  using value_type = typename BaseIteratorT::value_type;
  using difference_type = typename BaseIteratorT::difference_type;
  using pointer = typename BaseIteratorT::pointer;
  using reference = typename BaseIteratorT::reference;
  using iterator_category = std::input_iterator_tag;

  constexpr BoundImportForwarderRefIterator() noexcept
  {
  }

  explicit BoundImportForwarderRefIterator(Process const& process,
                                           PeFile const& pe_file,
                                           BoundImportDescriptor const& desc)
  {
    try
    {
      if (desc.GetNumberOfModuleForwarderRefs())
      {
        auto const start =
          static_cast<PIMAGE_BOUND_IMPORT_DESCRIPTOR>(desc.GetStart());
        auto const base = reinterpret_cast<PIMAGE_BOUND_FORWARDER_REF>(
          static_cast<PIMAGE_BOUND_IMPORT_DESCRIPTOR>(desc.GetBase()) + 1U);
        BoundImportForwarderRef const bound_import_forwarder{
          process, pe_file, start, base};
        impl_ = std::make_shared<Impl>(
          process, pe_file, desc, bound_import_forwarder);
      }
    }
    catch (std::exception const& /*e*/)
    {
      // Nothing to do here.
    }
  }

  explicit BoundImportForwarderRefIterator(Process const&& process,
                                           PeFile const& pe_file,
                                           BoundImportDescriptor const& desc) =
    delete;

  explicit BoundImportForwarderRefIterator(Process const& process,
                                           PeFile&& pe_file,
                                           BoundImportDescriptor const& desc) =
    delete;

  explicit BoundImportForwarderRefIterator(Process const& process,
                                           PeFile const& pe_file,
                                           BoundImportDescriptor&& desc) =
    delete;

  explicit BoundImportForwarderRefIterator(Process const&& process,
                                           PeFile&& pe_file,
                                           BoundImportDescriptor const& desc) =
    delete;

  explicit BoundImportForwarderRefIterator(Process const&& process,
                                           PeFile const& pe_file,
                                           BoundImportDescriptor&& desc) =
    delete;

  explicit BoundImportForwarderRefIterator(
    Process&& process, PeFile&& pe_file, BoundImportDescriptor&& desc) = delete;

  reference operator*() const noexcept
  {
    HADESMEM_DETAIL_ASSERT(impl_.get());
    return *impl_->bound_import_forwarder_;
  }

  pointer operator->() const noexcept
  {
    HADESMEM_DETAIL_ASSERT(impl_.get());
    return &*impl_->bound_import_forwarder_;
  }

  BoundImportForwarderRefIterator& operator++()
  {
    try
    {
      HADESMEM_DETAIL_ASSERT(impl_.get());

      if (++impl_->cur_fwd_ref_ >=
          impl_->bound_import_desc_->GetNumberOfModuleForwarderRefs())
      {
        impl_.reset();
        return *this;
      }

      auto const start_base = static_cast<PIMAGE_BOUND_IMPORT_DESCRIPTOR>(
        impl_->bound_import_desc_->GetStart());
      auto const cur_base = reinterpret_cast<PIMAGE_BOUND_FORWARDER_REF>(
        impl_->bound_import_forwarder_->GetBase());
      auto const new_base = cur_base + 1;
      impl_->bound_import_forwarder_ = BoundImportForwarderRef{
        *impl_->process_, *impl_->pe_file_, start_base, new_base};
    }
    catch (std::exception const& /*e*/)
    {
      impl_.reset();
    }

    return *this;
  }

  BoundImportForwarderRefIterator operator++(int)
  {
    BoundImportForwarderRefIterator const iter{*this};
    ++*this;
    return iter;
  }

  bool operator==(BoundImportForwarderRefIterator const& other) const noexcept
  {
    return impl_ == other.impl_;
  }

  bool operator!=(BoundImportForwarderRefIterator const& other) const noexcept
  {
    return !(*this == other);
  }

private:
  struct Impl
  {
    explicit Impl(Process const& process,
                  PeFile const& pe_file,
                  BoundImportDescriptor const& desc,
                  BoundImportForwarderRef const& forwarder) noexcept
      : process_{&process},
        pe_file_{&pe_file},
        bound_import_desc_{&desc},
        bound_import_forwarder_{forwarder}
    {
    }

    Process const* process_;
    PeFile const* pe_file_;
    BoundImportDescriptor const* bound_import_desc_;
    hadesmem::detail::Optional<BoundImportForwarderRef> bound_import_forwarder_;
    WORD cur_fwd_ref_{};
  };

  // Shallow copy semantics, as required by InputIterator.
  std::shared_ptr<Impl> impl_;
};

class BoundImportForwarderRefList
{
public:
  using value_type = BoundImportForwarderRef;
  using iterator = BoundImportForwarderRefIterator<BoundImportForwarderRef>;
  using const_iterator =
    BoundImportForwarderRefIterator<BoundImportForwarderRef const>;

  explicit BoundImportForwarderRefList(Process const& process,
                                       PeFile const& pe_file,
                                       BoundImportDescriptor const& desc)
    : process_{&process}, pe_file_{&pe_file}, desc_{&desc}
  {
  }

  explicit BoundImportForwarderRefList(Process const&& process,
                                       PeFile const& pe_file,
                                       BoundImportDescriptor const& desc) =
    delete;

  explicit BoundImportForwarderRefList(Process const& process,
                                       PeFile&& pe_file,
                                       BoundImportDescriptor const& desc) =
    delete;

  explicit BoundImportForwarderRefList(Process const& process,
                                       PeFile const& pe_file,
                                       BoundImportDescriptor&& desc) = delete;

  explicit BoundImportForwarderRefList(Process const&& process,
                                       PeFile&& pe_file,
                                       BoundImportDescriptor const& desc) =
    delete;

  explicit BoundImportForwarderRefList(Process const&& process,
                                       PeFile const& pe_file,
                                       BoundImportDescriptor&& desc) = delete;

  explicit BoundImportForwarderRefList(Process const&& process,
                                       PeFile&& pe_file,
                                       BoundImportDescriptor&& desc) = delete;

  iterator begin()
  {
    return iterator{*process_, *pe_file_, *desc_};
  }

  const_iterator begin() const
  {
    return const_iterator{*process_, *pe_file_, *desc_};
  }

  const_iterator cbegin() const
  {
    return const_iterator{*process_, *pe_file_, *desc_};
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
  BoundImportDescriptor const* desc_;
};
}
