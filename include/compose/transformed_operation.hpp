//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_TRANSFORMED_OPERATION_HPP
#define COMPOSE_TRANSFORMED_OPERATION_HPP

#include <utility>

namespace compose
{

/**
 * A type that represents an OperationBody transformed into a ComposedOperation.
 * The purpose of this type is to prevent accidental resumption as a
 * continuation, which could result in Undefined Behavior.
 */
template<typename Op>
class transformed_operation
{
public:
    using type = Op;

    template<typename... Args>
    explicit transformed_operation(Args&&... args);

    /**
     * Run the stored ComposedOperation.
     *
     * @remark Places this object in a valid, but unspecified state.
     *
     * @param args Arguments passed to function call operator of OperationBody
     */
    template<typename... Args>
    void run(Args&&... args);

    /**
     * Release ownership of an operation. The user is responsible on calling
     * run() on the operation to start it.
     *
     * @remark Places this object in a valid, but unspecified state.
     *
     * @returns The move-constructed operation object
     */
    Op release();

private:
    Op op_;
};

} // namespace compose

#include <compose/impl/transformed_operation.hpp>

#endif // COMPOSE_TRANSFORMED_OPERATION_HPP
