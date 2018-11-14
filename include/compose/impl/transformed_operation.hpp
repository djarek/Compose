
//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_IMPL_TRANSFORMED_OPERATION_HPP
#define COMPOSE_IMPL_TRANSFORMED_OPERATION_HPP

#include <compose/transformed_operation.hpp>

namespace compose
{

template<typename Op>
template<typename... Args>
transformed_operation<Op>::transformed_operation(Args&&... args)
  : op_{std::forward<Args>(args)...}
{
}

template<typename Op>
template<typename... Args>
void
transformed_operation<Op>::run(Args&&... args) &&
{
    op_.run(std::forward<Args>(args)...);
}

template<typename Op>
template<typename T, typename... Args>
transformed_operation<T>
transformed_operation<Op>::wrap(Args&&... args) &&
{
    return transformed_operation<T>{std::move(op_),
                                    std::forward<Args>(args)...};
}

} // namespace compose

#endif // COMPOSE_IMPL_TRANSFORMED_OPERATION_HPP
