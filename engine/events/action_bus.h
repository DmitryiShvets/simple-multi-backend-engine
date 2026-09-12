#pragma once

// #include <tuple>
// #include <utility>
// #include <cstddef>

#include <any>
#include <functional>
#include <typeindex>
#include <unordered_map>

namespace ssme {

/*COMPILETIME VERSION  */

// template <typename... THandlers>
// class ActionDispatcher {
// public:
//   explicit ActionDispatcher(THandlers... handlers)
//       : m_handlers(std::move(handlers)...) {}

//   template <typename TAction>
//   typename TAction::ResultType send(const TAction &action) {
//     return dispatch<TAction>(action);
//   }

// private:
//   template <typename TAction, size_t I = 0>
//   auto dispatch(const TAction &action) {
//     if constexpr (I >= sizeof...(THandlers)) {
//       static_assert(I < sizeof...(THandlers),
//                     "ActionDispatcher: no handler accepts this action");
//     } else {
//       auto &handler = std::get<I>(m_handlers);
//       if constexpr (requires { handler.handle(action); }) {
//         return handler.handle(action);
//       } else {
//         return dispatch<TAction, I + 1>(action);
//       }
//     }
//   }

//   std::tuple<THandlers...> m_handlers;
// };

/*RUNTIME VERSION  */

class ActionBus {
public:
  template <typename TAction, typename Fn>
  void on(Fn &&fn) {
    using Result = typename TAction::ResultType;
    std::function<std::any(const std::any &)> wrapped =
        [fn = std::forward<Fn>(fn)](const std::any &payload) -> std::any {
      const TAction &action = std::any_cast<const TAction &>(payload);
      if constexpr (std::is_void_v<Result>) {
        fn(action);
        return {};
      } else {
        return std::any(fn(action));
      }
    };
    m_handlers[typeid(TAction)] = std::move(wrapped);
  }

  template <typename TAction>
  typename TAction::ResultType send(const TAction &action) {
    using Result = typename TAction::ResultType;
    auto it = m_handlers.find(typeid(TAction));
    if (it == m_handlers.end()) {
      if constexpr (std::is_void_v<Result>) return;
      else return Result{};
    }
    auto result = it->second(std::any(action));
    if constexpr (std::is_void_v<Result>) return;
    else return std::any_cast<Result>(result);
  }

private:
  std::unordered_map<std::type_index,
                     std::function<std::any(const std::any &)>> m_handlers;
};

} // namespace ssme
