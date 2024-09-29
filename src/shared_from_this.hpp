#pragma once

#include <memory>      // std::enable_shared_from_this std::addressof
#include <type_traits> // std::remove_reference_t std::remove_cvref_t
#include <utility>     // std::forward

class enable_shared_from_this : public virtual std::enable_shared_from_this<void>
{
protected:
  template <typename U>
  std::shared_ptr<std::remove_reference_t<U>> shared_self(this U &&self)
  {
    return std::shared_ptr<std::remove_reference_t<U>>(self.shared_from_this(), std::addressof(self));
  }

  template <typename U>
  std::weak_ptr<std::remove_reference_t<U>> weak_self(this U &&self)
  {
    return std::forward<U>(self).shared_self();
  }
};
