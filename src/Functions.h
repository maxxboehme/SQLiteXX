#ifndef __SQLITECXX_SQLITE_FUNCTIONS_H__
#define __SQLITECXX_SQLITE_FUNCTIONS_H__

#include "DBConnection.h"
#include "Statement.h"

#include <functional>
#include <string>
#include <vector>

namespace SQLite
{
    enum class FunctionType: int {
        Deterministic = SQLITE_DETERMINISTIC
    };

    enum class TextEncoding: int {
        UTF8 = SQLITE_UTF8,
        UTF16LE = SQLITE_UTF16LE,
        UTF16BE = SQLITE_UTF16BE,
        UTF16 = SQLITE_UTF16,
    };

    inline TextEncoding operator|(TextEncoding lhs, FunctionType rhs) {
        return static_cast<TextEncoding>(
                static_cast<std::underlying_type<OpenMode>::type>(lhs) |
                static_cast<std::underlying_type<OpenMode>::type>(rhs));
    }

    inline TextEncoding operator|(FunctionType lhs, TextEncoding rhs) {
        return static_cast<TextEncoding>(
                static_cast<std::underlying_type<OpenMode>::type>(lhs) |
                static_cast<std::underlying_type<OpenMode>::type>(rhs));
    }

    /**
     * Helpers for setting result of scalar functions.
     */
    inline void return_result(sqlite3_context *context, int value) {
        sqlite3_result_int(context, value);
    }

    inline void return_result(sqlite3_context *context, int64_t value) {
        sqlite3_result_int64(context, value);
    }

    inline void return_result(sqlite3_context *context, double value) {
        sqlite3_result_double(context, value);
    }

    inline void return_result(sqlite3_context *context, const std::string &value) {
        sqlite3_result_text(context, value.c_str(), value.size(), SQLITE_TRANSIENT);
    }

    inline void return_result(sqlite3_context *context, const std::u16string &value) {
        sqlite3_result_text16(context, value.c_str(), value.size() * sizeof(char16_t), SQLITE_TRANSIENT);
    }

    inline void return_result(sqlite3_context *context, const Value &value) {
        sqlite3_result_value(context, value.getHandle());
    }

    inline void return_result(sqlite3_context *context, const Blob &value) {
        sqlite3_result_blob(context, value.data(), value.size(), SQLITE_TRANSIENT);
    }

    template <typename T>
    struct function_traits : public function_traits<decltype(&T::operator())>
    {};

    template <typename C, typename R, typename... Args>
    struct function_traits<R(C::*)(Args...)> {
        typedef std::function<R(Args...)> f_type;
        static const size_t nargs = sizeof...(Args);

        typedef R result_type;

        template<size_t i>
        struct arg
        {
            typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
        };
    };

    template <typename C, typename R, typename... Args>
    struct function_traits<R(C::*)(Args...) const> {
        typedef std::function<R(Args...)> f_type;
        static const size_t nargs = sizeof...(Args);

        typedef R result_type;

        template<size_t i>
        struct arg
        {
            typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
        };
    };

    template <typename R, typename... Args>
    struct function_traits<R(*)(Args...)> {
        typedef std::function<R(Args...)> f_type;
        static const size_t nargs = sizeof...(Args);

        typedef R result_type;

        template<size_t i>
        struct arg
        {
            typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
        };
    };

    template <typename R, typename... Args>
    struct function_traits<R(&)(Args...)> {
        typedef std::function<R(Args...)> f_type;
        static const size_t nargs = sizeof...(Args);

        typedef R result_type;

        template<size_t i>
        struct arg
        {
            typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
        };
    };

    template <typename T>
    struct SQLiteFunctionTraits : public SQLiteFunctionTraits<decltype(&T::operator())>
    {};

    template <typename C, typename R>
    struct SQLiteFunctionTraits<R(C::*)(const std::vector<Value> &)> {
        typedef std::function<R(const std::vector<Value> &)> f_type;
    };

    template <typename C, typename R>
    struct SQLiteFunctionTraits<R(C::*)(const std::vector<Value> &) const> {
        typedef std::function<R(const std::vector<Value> &)> f_type;
    };

    template <typename R>
    struct SQLiteFunctionTraits<R(*)(const std::vector<Value> &)> {
        typedef std::function<R(const std::vector<Value> &)> f_type;
    };

    template <typename R>
    struct SQLiteFunctionTraits<R(&)(const std::vector<Value> &)> {
        typedef std::function<R(const std::vector<Value> &)> f_type;
    };

    template <typename F>
    inline void internal_scalar_function(sqlite3_context* context, int argc, sqlite3_value **values) {
        F *userScalarFunction = static_cast<F *>(sqlite3_user_data(context));
        assert(userScalarFunction != 0);

        try {
            std::vector<Value> argValues;
            for (int i = 0; i < argc; ++i) {
                argValues.push_back(Value(values[i]));
            }

            auto result = (*userScalarFunction)(argValues);
            return_result(context, result);
        } catch (const std::bad_alloc &e) {
            sqlite3_result_error_nomem(context);
        } catch (const SQLite::Exception &e) {
            sqlite3_result_error(context, e.what(), e.errcode);
        } catch (const std::exception &e) {
            sqlite3_result_error(context, e.what(), SQLITE_ABORT);
        } catch (...) {
            sqlite3_result_error_code(context, SQLITE_ABORT);
        }
    }

    template<typename Call>
    inline void internal_delete(void *user_data) {
        Call *callback = static_cast<Call *>(user_data);
        assert(callback != 0);

        delete callback;
    }


    template <typename F>
    inline void CreateScalarFunction(
        DBConnection &connection,
        const std::string &name,
        F &&function,
        const TextEncoding flags = SQLite::TextEncoding::UTF8,
        int nargs = -1)
    {
        using FunctionType = typename SQLiteFunctionTraits<F>::f_type;

        FunctionType *userFunction = new FunctionType(function);

        sqlite3_create_function_v2(
            connection.getHandle(),
            name.c_str(),
            nargs,
            static_cast<int>(flags),
            (void*)userFunction,
            &internal_scalar_function<FunctionType>,
            nullptr,
            nullptr,
            &internal_delete<FunctionType>);
    }
}

#endif
