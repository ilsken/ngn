//
//  event.h
//  uv
//
//  Created by Christopher Tarquini on 10/10/13.
//  Copyright (c) 2013 Christopher Tarquini. All rights reserved.
//

#ifndef uv_event_h
#define uv_event_h


#include <functional>
#include <list>
#include <iterator>
#include <map>
#include <typeinfo>
#include <iostream>
#include <deque>
#include <atomic>
#include <memory>
#include <algorithm>

namespace ngn { namespace events {
  template <class Handler, class Allocator>
  class signal;
  
  template <class Signal>
  class connection {
  public:
    using signal_type = Signal;
    using slot_type = typename signal_type::slot_type;
    using function_type = typename signal_type::function_type;
    connection() {}; // empty connection
    connection(const connection& other) : m_slot(other.m_slot) {};
    connection(connection&& other) : m_slot(std::move(other.m_slot)) {};
    connection(const std::weak_ptr<function_type>& slot) : m_slot(slot) {};
    
    connection& operator=(connection&& rhs) {
      this->swap(rhs);
      return *this;
    }
    connection& operator=(const connection& rhs) {
      m_slot = rhs.m_slot;
      return *this;
    }
    
    void swap(connection& other) {
      using std::swap;
      swap(m_slot, other.m_slot);
    }
    
    [[gnu::always_inline]]
    inline bool connected() {
      auto slot = m_slot.lock();
      return slot && bool(*slot);
    }
    [[gnu::always_inline]]
    inline void disconnect() {
      auto slot = m_slot.lock();
      if (slot) {
        *slot = nullptr;
        m_slot.reset();
      }
    }
    
    
  private:
    std::weak_ptr<function_type> m_slot;
  };
  template <class connection>
  class scoped_connection {
  public:
    scoped_connection() : m_connection() {};
    scoped_connection(const connection& target) : m_connection(target) {};
    scoped_connection(const scoped_connection&) = delete;
    scoped_connection(scoped_connection&& other) : m_connection(other.m_connection) {};
    
    scoped_connection& operator=(const connection& rhs) {
      m_connection = rhs;
    };
    scoped_connection& operator=(scoped_connection&& rhs) {
      this->swap(rhs);
      return *this;
    };
    void swap(scoped_connection& other) {
      m_connection.swap(other.m_connection);
    };
    connection release() {
      connection ret{};
      m_connection.swap(ret);
      return ret;
    }
    ~scoped_connection() {
      m_connection.disconnect();
    }
  private:
    connection m_connection;
  };
  
  template <class connection>
  scoped_connection<connection> make_scoped_connection(connection&& target) {
    return ngn::events::scoped_connection<connection>(std::forward<connection>(target));
  };
  
  template <class Handler, class Allocator = std::allocator<std::function<Handler>>>
  class signal {
  public:
    using function_type = std::function<Handler>;
    using allocator_type = Allocator;
    using slot_type = std::shared_ptr<function_type>;
    using slot_list = std::vector<slot_type, typename std::allocator_traits<allocator_type>::template rebind_traits<slot_type>::allocator_type>;
    using list_allocator_type = typename slot_list::allocator_type;
    using const_slot_reference = typename std::add_const<typename std::add_lvalue_reference<slot_type>::type>::type;
    using slot_iterator = typename slot_list::iterator;
    using connection_type = connection<signal>;
    
    // allocator constructor
    signal(const allocator_type& alloc)
      : is_running(false),
      slots { list_allocator_type(alloc) },
      pending_slots { list_allocator_type(alloc) } {};
    
    // default constructor
    signal() : signal(allocator_type()) {};
    template <class... Arguments>
    void emit(Arguments&&... args)
    {
      is_running = true;
      // runs the slot if connected, otherwise return true and queue it for deletion
      auto is_disconnected = [&] (const_slot_reference slot)
      {
        auto& fn = *slot;
        if (fn) { fn(std::forward<Arguments>(args)...); return false;}
        else return true;
      };
      auto begin = slots.begin();
      auto end = slots.end();
      // sane implementations only move elements if the predicate returns true
      // as if calling find_if to find the first matching slot, then shifting the rest of the values
      // so the matched elements end up at the end of the array
      begin = std::remove_if(begin, end, is_disconnected);
      if (begin != end) slots.erase(begin, end);
      if (pending_slots.size() > 0) {
        slots.insert(slots.cend(), std::make_move_iterator(pending_slots.begin()), std::make_move_iterator(pending_slots.end()));
        pending_slots.clear();
      }
      is_running = false;
    }
    connection_type connect(const function_type& slot) {
      auto& container = !is_running ? slots : pending_slots;
      container.emplace_back(std::allocate_shared<function_type>(allocator, std::allocator_arg, allocator, slot));
      return connection_type { std::weak_ptr<function_type>( container.back()) };
    }
    void disconnect_all() {
      slots.clear();
      pending_slots.clear();
    }
    const allocator_type& get_allocator() const {
      return allocator;
    }
    inline bool empty() const {
      return slots.size() + pending_slots.size() == 0;
    }
    inline std::size_t slot_count() const {
      return slots.size() + pending_slots.size();
    }
  private:
    allocator_type allocator;
    bool is_running = false;
    slot_list pending_slots;
    slot_list slots;
  };
}
}

static unsigned long long _last_id = 0;
template <typename... Arguments>
class Event {
    template <class Functor>
    class EventConnection;
public:
    typedef void (*HandlerFn)(Arguments...);
    typedef std::function<void(Arguments...)> Handler;
    typedef Handler handler;
    typedef EventConnection<Handler> Connection;
    typedef std::list<Connection> HandlerList;
    typedef typename HandlerList::iterator Binding;
    typedef Binding (*addFn)(Handler);
    typedef void (*emitFn)(Arguments...);
    typedef typename HandlerList::iterator iterator;
    
    Connection addListener(Handler fn){
        listeners_.emplace_back(fn);
        return listeners_.back();
    };
    
    Connection on(Handler fn){
        return addListener(fn);
    };
    
    Connection once(Handler fn){
        listeners_.emplace_back(fn, true);
        //std::cout << "new size: " << listeners_.size() << "\n";
        return listeners_.back();
    };
    
    void removeListener(Connection binding) {
        
    };
    
    void off(Connection binding){
        removeListener(binding);
    };
    
    void removeAllListeners() {
        listeners_.clear();
    };
    void off(){
        removeAllListeners();
    }
    
    HandlerList& listeners(){
        return listeners_;
    }
    
    size_t size(){
        return listeners_.size();
    }
    
    
    void emit(Arguments&&... args) {
        is_emitting = true;
        size_t size = listeners_.size();
        auto i = listeners_.begin();
        while (size != 0 && i != listeners_.end()) {
            if (i->marked) {
                i = listeners_.erase(i);
            }
            else {
                auto fn = i->fn_;
                if (i->once == true) {
                    i = listeners_.erase(i);
                } else {
                    i++;
                }
                if (fn)
                    fn(std::forward<Arguments>(args)...);
            }
            size--;
        }
        is_emitting = false;
    }
    void operator()(Arguments&&... args){
        emit(std::forward<Arguments>(args)...);
    };
    
    Connection operator()(Handler fn){
        return addListener(fn);
    }
    
private:
    std::atomic<bool> is_emitting;
    static const unsigned int argsize = sizeof...(Arguments);
    unsigned int max_listeners_ = 10;
    HandlerList listeners_;
    template <class Functor>
    class EventConnection {
        friend class Event<Arguments...>;
    public:
        EventConnection (Functor fn) : fn_(fn) , _id(_last_id++) {};
        EventConnection (Functor fn, bool once_) : fn_(fn) ,once(once_), _id(_last_id++) {};
        EventConnection() : _id(_last_id++) {};
        //        EventConnection(const EventConnection&) = delete;
        //      EventConnection(EventConnection&&) = delete;
        void remove() {
            marked = true;
        }
    private:
        
        Functor fn_;
        bool once = false;
        bool marked = false;
        unsigned long long _id = 0;
    };
    
    
};





#endif
