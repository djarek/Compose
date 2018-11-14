//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_UPCALL_GUARD_HPP
#define COMPOSE_UPCALL_GUARD_HPP

namespace compose
{

#ifndef COMPOSE_NO_NODISCARD
#define COMPOSE_NODISCARD [[nodiscard]]
#else
#define COMPOSE_NODISCARD
#endif // COMPOSE_NO_NODISCARD

/**
 * A tag type to encourage returning after performing operations that may result
 * in dangling references.
 *
 * @remark Will generate unused return value warnings, unless explicitly
 * ignored.
 */
struct COMPOSE_NODISCARD upcall_guard
{
};

} // namespace compose

#endif // COMPOSE_UPCALL_GUARD_HPP
