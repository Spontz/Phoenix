#pragma once

#include <memory>

#define SPZ_DECLARE_SMART_POINTERS(T0)\
typedef std::shared_ptr<T0> SP_##T0;\
typedef std::weak_ptr<T0> WP_##T0;\
typedef std::unique_ptr<T0> UP_##T0;\
typedef std::shared_ptr<const T0> CSP_##T0;\
typedef std::weak_ptr<const T0> CWP_##T0;\
typedef std::unique_ptr<const T0> CUP_##T0;

#define SPZ_DECLARE_SMART_POINTERS_EX(TYPE_CATEGORY, T0)\
TYPE_CATEGORY T0;\
SPZ_DECLARE_SMART_POINTERS(T0)

#define SPZ_DECLARE_SMART_POINTERS_SHORT(T0)\
typedef std::shared_ptr<T0> sptr;\
typedef std::weak_ptr<T0> wptr;\
typedef std::unique_ptr<T0> uptr;\
typedef std::shared_ptr<const T0> csptr;\
typedef std::weak_ptr<const T0> cwptr;\
typedef std::unique_ptr<const T0> cuptr;

#define SPZ_DECLARE_SMART_POINTERS_SHORT_EX(TYPE_CATEGORY, T0)\
TYPE_CATEGORY T0;\
SPZ_DECLARE_SMART_POINTERS_SHORT(T0)

#define SPZ_DECLARE_FIELD(T, ID)\
private:\
T ID##_;\
public:\
T& get_##ID() {return ID##_;}\
public:\
T const& get_##ID() const {return ID##_;}
